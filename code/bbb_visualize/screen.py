
import threading
import time
import math

import Adafruit_Nokia_LCD as LCD
import Adafruit_GPIO.SPI as SPI

from PIL import Image
from PIL import ImageDraw
from PIL import ImageFont

import pb_bin_reader

MAX_UINT = 65535

DC = 'P9_15'
RST = 'P9_12'
SPI_PORT = 1
SPI_DEVICE = 0

MAX_FREQ = 10000 / 2

edgeByChannelId = { 0: "f",
                    1: "r",
                    2: "l",
                    3: "b" }

bins = None
display = None

FGCOLOR = 1
BGCOLOR = 0

min_bin_freq = MAX_FREQ
max_bin_freq = 0

interrupted = False


class DisplayThread( threading.Thread ):
   def __init__( self, disp, image ):
      threading.Thread.__init__( self )
      self.disp = disp
      self.image = image

   def run( self ):
      while not interrupted:
         self.disp.image( self.image )
         self.disp.display()
         time.sleep( 0.050 )


class Display( object ):

   def __init__( self, min_bin_freq, max_bin_freq ):
      spi = SPI.SpiDev( SPI_PORT, SPI_DEVICE, max_speed_hz=4000000 )
      self.disp = LCD.PCD8544( DC, RST, spi=spi )
      self.disp.begin( contrast=50 )

      # '1' is for 1-bit color
      self.image = Image.new( '1', ( LCD.LCDWIDTH, LCD.LCDHEIGHT ) )
      self.draw = ImageDraw.Draw( self.image )

      self.disp.clear()

      self.barLength = min( LCD.LCDHEIGHT, LCD.LCDHEIGHT )
      self.k_flat = self.barLength / math.log( MAX_FREQ )
      self.k_linear_stretch = float( self.barLength ) / ( max_bin_freq - min_bin_freq )
      self.k_flat2 = self.barLength / math.log( max_bin_freq - min_bin_freq + 1 )
      self.k_stretch = self.barLength / math.log( max_bin_freq / min_bin_freq )

      self.min_bin_freq = min_bin_freq
      self.max_bin_freq = max_bin_freq

      font = ImageFont.load_default()
      # font = ImageFont.truetype('Minecraftia.ttf', 8)
      self.draw.text( ( 16, 20 ), 'Pixee el', font=font, fill=FGCOLOR )
      self.draw.text( ( 46, 18 ), 'B', font=font, fill=FGCOLOR )
      self.disp.image( self.image )
      self.disp.display()

      self.displayThread = DisplayThread( self.disp, self.image )
      self.displayThread.start()

   def stop( self ):
      self.displayThread.join( 2 )

   def clear( self ):
      self.disp.clear()
      self.disp.display()

   def refresh( self ):
#      self.disp.image( self.image )
#      self.disp.display()
      pass

   def COORD_FLAT( self, freq ):
      return math.log( freq ) * self.k_flat

   def COORD_LINEAR_STRETCH( self, freq ):
      return ( freq - self.min_bin_freq ) * self.k_linear_stretch

   def COORD_FLAT2( self, freq ):
      return math.log( freq - self.min_bin_freq + 1 ) * self.k_flat2

   def COORD_STRETCH( self, freq ):
      return math.log( freq / self.min_bin_freq ) * self.k_stretch

   def coords( self, edge, binId, coord ):
      x1 = x2 = y1 = y2 = 0

      freq_low = bins[binId]['freq_low']
      freq_high = bins[binId]['freq_high']

      if edge in "fb":
         offset_x = ( LCD.LCDWIDTH - self.barLength ) / 2

         x1 = offset_x + coord( freq_low )
         x2 = offset_x + coord( freq_high )

         y1 = y2 = binId if edge == "f" else ( LCD.LCDHEIGHT - binId - 1 )
      else:
         offset_y = ( LCD.LCDHEIGHT - self.barLength ) / 2

         y1 = offset_y + coord( freq_low )
         y2 = offset_y + coord( freq_high )

         x1 = x2 = binId if edge == "l" else ( LCD.LCDWIDTH - binId - 1 )

      return [ int( x1 ), int( y1 ), math.ceil( x2 ), math.ceil( y2 ) ]

   def show( self, edge, binId, mag=None, queue=None ):
      coords = self.coords( edge, binId, coord=self.COORD_STRETCH )
      self.draw.line( coords, fill=FGCOLOR )
      self.refresh()
     
   def hide( self, edge, binId, mag=None, queue=None ):
      coords = self.coords( edge, binId, coord=self.COORD_STRETCH )
      self.draw.line( coords, fill=BGCOLOR )
      self.refresh()


class RectDisplay( Display ):

   def __init__( self, min_bin_freq, max_bin_freq ):
      Display.__init__( self, min_bin_freq, max_bin_freq )

   def drawRectangle( self, edge, coords, fill ):
      depth = len( bins )

      if edge == "f":
         coords[ 1 ] = 0
         coords[ 3 ] = depth
      elif edge == "b":
         coords[ 1 ] = LCD.LCDHEIGHT - 1 - depth
         coords[ 3 ] = LCD.LCDHEIGHT - 1
      elif edge == "r":
         coords[ 0 ] = LCD.LCDWIDTH - 1 - depth
         coords[ 2 ] = LCD.LCDWIDTH - 1
      elif edge == "l":
         coords[0] = 0
         coords[2] = depth

      self.draw.rectangle( coords, fill=fill )

   def show( self, edge, binId, mag=None, queue=None ):
      coords = self.coords( edge, binId, coord=self.COORD_LINEAR_STRETCH )
      self.drawRectangle( edge, coords, fill=FGCOLOR )
      self.refresh()
     
   def hide( self, edge, binId, mag=None, queue=None ):
      coords = self.coords( edge, binId, coord=self.COORD_LINEAR_STRETCH )
      self.drawRectangle( edge, coords, fill=BGCOLOR )

      for i in [ q for q in queue if edgeByChannelId[ q[2] ] == edge ]:
         coords = self.coords( edge, q[1], coord=self.COORD_LINEAR_STRETCH )
         self.drawRectangle( edge, coords, fill=FGCOLOR )

      self.refresh()


class MagDisplay( Display ):

   def __init__( self, min_bin_freq, max_bin_freq ):
      Display.__init__( self, min_bin_freq, max_bin_freq )

   def drawMag( self, edge, coords, mag, fill ):
      depth = mag * len( bins ) / MAX_UINT

      if edge == "f":
#         coords[ 1 ] = 0
         coords[ 1 ] = depth
         coords[ 3 ] = depth
      elif edge == "b":
         coords[ 1 ] = LCD.LCDHEIGHT - 1 - depth
#         coords[ 3 ] = LCD.LCDHEIGHT - 1
         coords[ 3 ] = LCD.LCDHEIGHT - 1 - depth
      elif edge == "r":
         coords[ 0 ] = LCD.LCDWIDTH - 1 - depth
         coords[ 2 ] = LCD.LCDWIDTH - 1 - depth
#         coords[ 2 ] = LCD.LCDWIDTH - 1
      elif edge == "l":
#         coords[0] = 0
         coords[0] = depth
         coords[2] = depth

#      self.draw.rectangle( coords, fill=fill )
      self.draw.line( coords, fill=fill )

   def show( self, edge, binId, mag, queue=None ):
      coords = self.coords( edge, binId, coord=self.COORD_LINEAR_STRETCH )
      self.drawMag( edge, coords, mag, fill=FGCOLOR )
      self.refresh()
     
   def hide( self, edge, binId, mag, queue=None ):
      coords = self.coords( edge, binId, coord=self.COORD_LINEAR_STRETCH )
      self.drawMag( edge, coords, mag, fill=BGCOLOR )

#      for i in [ q for q in queue if edgeByChannelId[ q[2] ] == edge ]:
#         coords = self.coords( edge, q[1], coord=self.COORD_LINEAR_STRETCH )
#         self.drawRectangle( edge, coords, fill=FGCOLOR )

      self.refresh()


def init():
   global bins, display, min_bin_freq, max_bin_freq

   bins = pb_bin_reader.load( "../bbb_common/bin_defs.txt" )
   for binValue in bins.values(): print binValue

   for binValue in bins.values():
      min_bin_freq = min( min_bin_freq, binValue["freq_low"] )
      max_bin_freq = max( max_bin_freq, binValue["freq_high"] )

   display = MagDisplay( min_bin_freq, max_bin_freq )


def show( nowMs, channelId, binId, mag, queue=None ):
   edge = edgeByChannelId[ channelId ]
#   print "+%d:\tedge %s bid %d" % ( nowMs, edge, binId )
   display.show( edge, binId, mag=mag, queue=queue )

def hide( nowMs, channelId, binId, mag, queue=None ):
   edge = edgeByChannelId[ channelId ]
#   print "-%d:\tedge %s bid %d" % ( nowMs, edge, binId )
   display.hide( edge, binId, mag=mag, queue=queue )

def stop():
   global interrupted
   interrupted = True
   display.stop()

