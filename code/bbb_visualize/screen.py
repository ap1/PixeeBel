
import time
import math

import Adafruit_Nokia_LCD as LCD
import Adafruit_GPIO.SPI as SPI

from PIL import Image
from PIL import ImageDraw
from PIL import ImageFont

import pb_bin_reader


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


class Display( object ):

   def __init__( self ):
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

   def clear( self ):
      for row in range( 0, LCD.LCDHEIGHT ):
         self.draw.line( ( 0, LCD.LCDWIDTH, row, row ), fill=FGCOLOR )

      self.disp.clear()
      self.disp.display()

   def refresh( self ):
      self.disp.image( self.image )
      self.disp.display()

   def coords( self, edge, binId ):
      x1 = x2 = y1 = y2 = 0

      freq_low = bins[binId]['freq_low']
      freq_high = bins[binId]['freq_high']

      def COORD_FLAT( freq ):
         return math.log( freq ) * self.k_flat

      def COORD_LINEAR_STRETCH( freq ):
         return ( freq - min_bin_freq ) * self.k_linear_stretch

      def COORD_FLAT2( freq ):
         return math.log( freq - min_bin_freq + 1 ) * self.k_flat2

      def COORD_STRETCH( freq ):
         return math.log( freq / min_bin_freq ) * self.k_stretch

      def COORD( freq ):
         return COORD_LINEAR_STRETCH( freq )

      if edge in "fb":
         offset_x = ( LCD.LCDWIDTH - self.barLength ) / 2

         x1 = offset_x + COORD( freq_low )
         x2 = offset_x + COORD( freq_high )

         y1 = y2 = binId if edge == "f" else ( LCD.LCDHEIGHT - binId - 1 )
      else:
         offset_y = ( LCD.LCDHEIGHT - self.barLength ) / 2

         y1 = offset_y + COORD( freq_low )
         y2 = offset_y + COORD( freq_high )

         x1 = x2 = binId if edge == "l" else ( LCD.LCDWIDTH - binId - 1 )

      return ( int( x1 ), int( y1 ), math.ceil( x2 ), math.ceil( y2 ) )

   def show( self, edge, binId ):
      coords = self.coords( edge, binId )
      self.draw.line( coords, fill=FGCOLOR )
      self.refresh()
     
   def hide( self, edge, binId ):
      coords = self.coords( edge, binId )
      self.draw.line( coords, fill=BGCOLOR )
      self.refresh()


def init():
   global bins, display, min_bin_freq, max_bin_freq

   bins = pb_bin_reader.load( "../bbb_common/bin_defs.txt" )
   for binValue in bins.values(): print binValue

   for binValue in bins.values():
      min_bin_freq = min( min_bin_freq, binValue["freq_low"] )
      max_bin_freq = max( max_bin_freq, binValue["freq_high"] )

   display = Display()


def show( nowMs, channelId, binId ):
   edge = edgeByChannelId[ channelId ]
   print "+%d:\tedge %s bid %d" % ( nowMs, edge, binId )
   display.show( edge, binId )

def hide( nowMs, channelId, binId ):
   edge = edgeByChannelId[ channelId ]
   print "-%d:\tedge %s bid %d" % ( nowMs, edge, binId )
   display.hide( edge, binId )


if __name__ == "__main__":
   lcd_init()


## Draw a white filled box to clear the image.
#draw.rectangle((0,0,LCD.LCDWIDTH,LCD.LCDHEIGHT), outline=255, fill=255)
#
## Draw some shapes.
#draw.ellipse((2,2,22,22), outline=0, fill=255)
#draw.rectangle((24,2,44,22), outline=0, fill=255)
#draw.polygon([(46,22), (56,2), (66,22)], outline=0, fill=255)
#draw.line((68,22,81,2), fill=0)
#draw.line((68,2,81,22), fill=0)
#
## Load default font.
#font = ImageFont.load_default()
#
## Alternatively load a TTF font.
## Some nice fonts to try: http://www.dafont.com/bitmap.php
## font = ImageFont.truetype('Minecraftia.ttf', 8)
#
## Write some text.
#draw.text((8,30), 'Hello World!', font=font)
#
## Display image.
#disp.image(image)
#disp.display()
