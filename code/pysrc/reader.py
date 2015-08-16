import ctypes
import struct
import collections
import itertools


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
   res = reader.read_one_frame( fd, bufP )
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

# Usage:
#     load( "data/samples-1439671035.dat" )
#     ( header, samples ) = read_one_frame()
#     cleanup()

