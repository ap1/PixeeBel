#!/usr/bin/python

import numpy
import subprocess
import math
from math import sqrt, exp
import matplotlib.pyplot as plt
import random
import scipy.signal
import pdb

from pysrc import reader
from pysrc import caliberation
from pysrc import filters

import sys

def FindTimeShiftA(a, b):
  na = len(a)
  dt = numpy.arange(1-na, na)
  maxcorr = scipy.signal.correlate(a, b, "full").argmax()
  val = float(dt[maxcorr]) 
  return val

def FindTimeShiftB(a, b):
  af = scipy.fft(a)
  bf = scipy.fft(b)
  c = scipy.ifft(af * scipy.conj(bf))

  time_shift = numpy.argmax(abs(c))

  n = len(a)

  if(time_shift < -n/2): time_shift = time_shift + n
  if(time_shift > n/2): time_shift = time_shift - n
  return time_shift

def FindTimeShiftMatrix(frameSamples):
  corrMatrixA = numpy.matrix([[0.0]*channelCount] * channelCount)
  corrMatrixB = numpy.matrix([[0.0]*channelCount] * channelCount)

  for i, (ci, di) in enumerate(frameSamples.items()):
    for j, (cj, dj) in enumerate(frameSamples.items()):
      if(i <= j):
        valA = FindTimeShiftA(di, dj)
        valB = FindTimeShiftB(di, dj)
        corrMatrixA[(i,j)] = valA
        corrMatrixA[(j,i)] = -valA
        corrMatrixB[(i,j)] = valB
        corrMatrixB[(j,i)] = -valB

  if(numpy.count_nonzero(corrMatrixA) != 0 or numpy.count_nonzero(corrMatrixB) != 0):
    print "Timestamp ", timeStamp
    print corrMatrixA
    print corrMatrixB

# ---- Main ---- #
def Main():
  # samples_file = "data/samples-1439671035.dat"
  # (samples_mic0, samples_mic1, samples_mic2, samples_mic3) = caliberation.LoadSamples(samples_file)
  # channelMap = caliberation.CaliberateChannels(samples_mic0, samples_mic1, samples_mic2, samples_mic3)

  if(len(sys.argv) >= 2):
    reader.load(sys.argv[1])
  else:
    reader.load("data/samples-1439673160.dat")
  # reader.load("data/samples-1439676416.dat")

  framecount = 0

  channelCount = 4

  moment0 = [[] for c in range(0, channelCount)]

  while True:
    parsed = reader.read_one_frame()
    if not parsed:
      break

    (header, frameSamples) = parsed

    timeStamp = header.send_timestamp

    # lowpass everything
    for channel, data in frameSamples.items():
      # data = caliberation.RemoveDC( data )
      # (data, freq) = filters.apply(data, filters.lpf(N, 3000))
      avg = max([abs(d) for d in data])
      #moment0[channel].append(avg)
      moment0[channel].extend(data)

    framecount = framecount+1
    if(framecount % 1000 == 0): print "done %d frames" % framecount

  moment0 = [ moment0[0] ]
  pdb.set_trace()

  sep = 10
#  moment0 = [ moment[:len(moment)/3] for moment in moment0 ]
  N = len(moment0[0])
  x = range(0, N)
  ffts = [ scipy.fft( moment ) for moment in moment0 ]
  ffts = [ [ f if hz else 1 for hz, f in enumerate( fft ) ] for fft in ffts ]
  xdata = range( 1, N )
  xdata = numpy.fft.fftfreq( N, d=0.000110 )
  for i, avgs in enumerate( ffts ):
    #avgs = caliberation.RemoveDC(avgs)
    #(avgs, freq) = filters.apply(avgs, filters.lpf(N, 500))
    #pdb.set_trace()
    plt.plot( xdata, [ scipy.log(numpy.abs(avg)) + i*sep for avg in avgs ] )

  #plt.plot(x, [numpy.argmax([abs(val) for val in v]) for v in zip(*moment0)])

  plt.show()
#------------------------------------------

Main()
