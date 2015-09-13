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

import pdb

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