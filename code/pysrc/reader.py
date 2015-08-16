import ctypes
import struct
import collections
import itertools
import filters
from Queue import deque


buf = ctypes.create_string_buffer( 2048 )
bufP = (ctypes.c_char_p)(ctypes.addressof( buf ))

fd = -1
reader = None

def load( filename ):
   global reader
   global fd
   reader = ctypes.CDLL( "capture/libreader.so" )
   fd = reader.open_file( filename )

def read_one_frame():
   res = reader.read_one_frame( fd, bufP, 0 )
   if res == 0:
      return None

   bufH = collections.namedtuple( "BufH", "sample_seq num_channels " \
                                          "num_samples send_timestamp" )

   bufH.sample_seq = struct.unpack( "H", buf[0:2] )[0]
   bufH.num_channels = struct.unpack( "B", buf[2:3] )[0]
   bufH.num_samples =  struct.unpack( "B", buf[3:4] )[0]
   bufH.send_timestamp = struct.unpack( "H", buf[4:6] )[0]

   bufSize = bufH.num_samples * bufH.num_channels
   data = struct.unpack( "B" * bufSize, buf[6:6+bufSize] )

   channels = range( bufH.num_channels )
   samples = dict( ( ( channel, [] ) for channel in channels ) )
   for sample, channel in zip( data, itertools.cycle( channels ) ):
      samples[channel].append( sample )

   return ( bufH, samples )

def cleanup():
   reader.close_file( fd )

def perframe( channels, lambdaOp=None, filt=None, numFrames=1 ):
   
   assert( numFrames > 0 )

   if numFrames > 1:
      queue = deque()

   while True:
      samples = dict( ( ( channel, [] ) for channel in channels ) )
      parsed = read_one_frame()
      if not parsed:
         break

      ( header, frameSamples ) = parsed

      for channel, data in frameSamples.items():
         if channel not in channels:
            continue
         samples[channel].extend( data )

      if lambdaOp:
         samples = dict( ( ( c, lambdaOp( samples[c] ) )
                           for c in channels ) )

      if filt:
         samples = dict(( ( c, filters.apply( samples[c], filt ) )
                           for c in channels ))

      if numFrames == 1:
         yield ( header, samples )
      else:
         queue.append( samples )
         if len( queue ) > numFrames:
            queue.popleft()

         header.num_samples *= len( queue )

         # generate merged dictionary
         samples = dict( ( ( channel, [] ) for channel in channels ) )
         for queueItem in queue:
            for channel, data in queueItem.items():
               samples[channel].extend( data )

         yield ( header, samples )
