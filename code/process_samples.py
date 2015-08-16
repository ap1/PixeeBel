#!/usr/bin/python

import numpy
import subprocess
import math
from math import sqrt, exp
import matplotlib.pyplot as plt
import random
import scipy.signal

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

  N = len(moment0[0])
  x = range(0, N)
  for avgs in moment0:
    #avgs = caliberation.RemoveDC(avgs)
    #(avgs, freq) = filters.apply(avgs, filters.lpf(N, 500))
    plt.plot(range(0, N), avgs)

  #plt.plot(x, [numpy.argmax([abs(val) for val in v]) for v in zip(*moment0)])

  plt.show()
#------------------------------------------

Main()