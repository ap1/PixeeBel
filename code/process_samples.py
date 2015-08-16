#!/usr/bin/python

import numpy
import subprocess
import math
from math import sqrt, exp
import matplotlib.pyplot as plt

pi = 3.1415926

class CaliberationData:
  # values are ordered in the N-E-W-S sequence
  avgAmplitude = None # array containing average amplitude (gain)

# def CaliberateFromDataFile(filename):


def LoadChannel(samples_file, channel):
  reader_exe = "bin/reader"
  print "reading channel " + str(channel)
  #raw_input("Press Enter to continue...")
  data = subprocess.check_output([reader_exe, samples_file, str(channel)]) 
  return [float(d) for d in data.splitlines()]

def LoadSamples(samples_file):
  return (
    LoadChannel(samples_file, 0),
    LoadChannel(samples_file, 1),
    LoadChannel(samples_file, 2),
    LoadChannel(samples_file, 3))

def printLine():
  print "-----------"

def gaussian(n, sigma, scale):
  r = range(-int(n/2),int(n/2)+1)
  return [scale / (sigma * sqrt(2*pi)) * exp(-float(x)**2/(2*sigma**2)) for x in r]

def invGaussian(n, sigma, scale):
  r = range(-int(n/2),int(n/2)+1)
  return [scale - (scale / (sigma * sqrt(2*pi))) * exp(-float(x)**2/(2*sigma**2)) for x in r]

def threshold(signal, minval, maxval):
  return [math.max(math.min(s, maxval), minval) for s in signal]

def RemoveDCComponent(data):
  dc = numpy.average(data)
  print "dc value is " + str(dc)
  return [v - dc for v in data]

def SumList(arr1, arr2):
  return [a1+a2 for a1, a2 in zip(arr1, arr2)]

# ---- Main ---- #
def Main():
  samples_file = "data/samples-1439671035.dat"
  
  (samples_mic0, samples_mic1, samples_mic2, samples_mic3) = LoadSamples(samples_file)

  printLine()
  print "Loaded all data"
  printLine()

  print "Sample sizes: %d %d %d %d" % (
    len(samples_mic0), 
    len(samples_mic1), 
    len(samples_mic2), 
    len(samples_mic3))

  # print samples_mic0[0:25]
  # print samples_mic1[1:25]
  # print samples_mic2[1:25]
  # print samples_mic3[1:25]

  # find the peaks and tell us channel order

  signal_mic0 = RemoveDCComponent(samples_mic0)
  #print signal_mic0[0:25]

  lowPassKernel = gaussian(64, 1, 1.0) # SumList(gaussian(65, 8, 1.0), invGaussian(65, 2, 0.01))

  plt.plot(range(0, len(lowPassKernel)), lowPassKernel, label='filter kernel')

  plt.show()

  #signal_mic0 = threshold(signal_mic0, )

  lowpass_mic0 = numpy.convolve(signal_mic0, lowPassKernel, "same")

  #print lowpass_mic0[0:25]

  x = range(0, len(samples_mic0))

  print "%d %d %d %d" % (len(x), len(samples_mic0), len(signal_mic0), len(lowpass_mic0))

  #plt.plot(x, samples_mic0)
  plt.plot(x, signal_mic0, label='zero mean signal')
  plt.plot(x, lowpass_mic0, label='low pass output')

  plt.show()

#------------------------------------------

Main()