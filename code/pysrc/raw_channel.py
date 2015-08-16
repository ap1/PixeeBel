#!/usr/bin/env python

import reader
import numpy
import sys
from filters import removeDc as removeDc
from utils import writeListAsRaw, toUbyte
import pdb

if len( sys.argv ) != 2:
   print "Usage: %s dat-file" % sys.argv[0]
   sys.exit( 0 )

# ---------------------------------
# main
# ---------------------------------

channels = [ 0, 1 ]
samples = None

if __name__ == "__main__":

   samples = dict( ( ( channel, [] ) for channel in channels ) )

   reader.load( sys.argv[1] )
   for header, frSamples in reader.perframe( channels, lambdaOp=removeDc ):
      for channel, data in frSamples.items():
         samples[channel].extend( data )

   for channel in channels:
      writeListAsRaw( ( toUbyte( v ) for v in samples[channel] ),
                      filename="data%d.raw" % channel )
   reader.cleanup()

