#!/usr/bin/env python

import reader
import numpy
import sys
from filters import removeDc as removeDc
from utils import writeListAsRaw, toFloat
import pdb

if len( sys.argv ) != 2:
   print "Usage: %s dat-file" % sys.argv[0]
   sys.exit( 0 )

# ---------------------------------
# main
# ---------------------------------

channels = [ 0, 1 ]
#samples = None

count = 0

if __name__ == "__main__":

   #samples = dict( ( ( channel, [] ) for channel in channels ) )
   deltaSampling = 0.00011 * 240.0
   deltaTime = 0

   reader.load( sys.argv[1] )
   for header, frSamples in reader.perframe( channels,
                           lambdaOp=lambda d: toFloat(removeDc(d)),
                           numFrames=3 ):
      deltaTime += deltaSampling
      frSamples = dict(( ( channel, numpy.array(data) )
                         for channel, data in frSamples.items() ))
      max0 = max( [ abs(r) for r in frSamples[channels[0]] ] )
      max1 = max( [ abs(r) for r in frSamples[channels[1]] ] )
      diff = frSamples[0] - frSamples[1] * max0/max1
      print "%f\t%f" % ( deltaTime, max( [ abs(r) for r in diff ] ) )

      count += 1
#      pdb.set_trace()

   reader.cleanup()

