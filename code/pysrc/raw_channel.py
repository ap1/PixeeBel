#!/usr/bin/env python

import reader
import numpy

channels = 0

samples = None

#reader.load( "data/samples-1439671035.dat" )
reader.load( "data/samples-1439673160.dat" )

def ubyte( c ):
   if c < -128:
      # this should be 128 because we return 
      # to unsigned byte
      return 128

   if c < 0:
      return 256 + c

   if c > 127:
      return 127

   return c

def removeDC( data ):
  dc = int( numpy.average( data ) )
  return [ ubyte( v - dc ) for v in data ]

while True:
   parsed = reader.read_one_frame()
   if not parsed:
      break

   ( header, frameSamples ) = parsed
   if not samples:
      samples = dict( ( ( channel, [] ) for channel in range( header.num_channels ) ) )

   for channel, data in frameSamples.items():
      data = removeDC( data )
      samples[channel].extend( data )

with open( "data.raw", "wb" ) as f:
   data = bytearray( samples[channel] )
   f.write( data )

reader.cleanup()

