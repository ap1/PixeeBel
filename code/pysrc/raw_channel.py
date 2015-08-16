#!/usr/bin/env python

import reader
import numpy
import sys
from filters import removeDC as removeDC
from utils import writeListAsRaw
import pdb

if len( sys.argv ) != 2:
   print "Usage: %s dat-file" % sys.argv[0]
   sys.exit( 0 )

# ---------------------------------
# main
# ---------------------------------

channels = [ 0 ]
samples = None

if __name__ == "__main__":

   samples = dict( ( ( channel, [] ) for channel in channels ) )

   reader.load( sys.argv[1] )
   for header, frSamples in reader.perframe( channels, lambdaOp=removeDC ):
      for channel, data in frSamples.items():
         samples[channel].extend( data )

   for channel in channels:
      writeListAsRaw( samples[channel],
                      filename="data%d.raw" % channel )
   reader.cleanup()

