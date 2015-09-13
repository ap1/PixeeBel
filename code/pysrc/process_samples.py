#!/usr/bin/python

import numpy
import subprocess
import math
from math import sqrt, exp
import matplotlib.pyplot as plt
import random
import scipy.signal

import reader
import caliberation
import filters

import sys
import pdb


def GetFrameStats(data, inRunningSum, inRunningVoiceSum, inRunningCount):

  Ndata = len(data)
  voicefilter = filters.mult(filters.hpf(Ndata, 80), filters.lpf(Ndata, 1000))
  (voicedata, voicefreq) = filters.apply(data, voicefilter)

  maxVal = max([abs(d) for d in data])
  
  outRunningCount     = inRunningCount    + Ndata
  outRunningSum       = inRunningSum      + sum([abs(d) for d in data])
  outRunningVoiceSum  = inRunningVoiceSum + sum([abs(d) for d in voicedata])

  outRunningAvg       = outRunningSum / outRunningCount
  outRunningVoiceAvg  = outRunningVoiceSum / outRunningCount

  voiceMax = max([abs(d) for d in voicedata])

  return (maxVal, voicedata, voiceMax, outRunningCount, outRunningSum, outRunningAvg, outRunningVoiceSum, outRunningVoiceAvg)


# ---- Main ---- #
def Main():
  # samples_file = "data/samples-1439671035.dat"
  # (samples_mic0, samples_mic1, samples_mic2, samples_mic3) = caliberation.LoadSamples(samples_file)
  # channelMap = caliberation.CaliberateChannels(samples_mic0, samples_mic1, samples_mic2, samples_mic3)

  if(len(sys.argv) >= 2):
    reader.load(sys.argv[1])
  else:
    reader.load("data/samples-1439673160.dat")

  framecount = 0

  channelCount = 4

  globalData = []

  isLoudSound = []
  soundDirection = [] # 0 is none

  runningSum = 0.0
  runningCount = 0.0

  runningVoiceSum = 0.0

  while True:
    parsed = reader.read_one_frame()
    if not parsed:
      break

    (header, frameSamples) = parsed

    maxVals = [ max([abs(d) for d in tmpData]) for (tmpChannel, tmpData) in frameSamples.items() ]

    bestChannel = numpy.argmax(maxVals)
    worstChannel = numpy.argmin(maxVals)

    # # avg data for channels that are not the loudest
    # nonMaxChannelData = [v[1] for v in (frameSamples.items()[0:bestChannel] + frameSamples.items()[bestChannel+1:])]
    # nonMaxChannelAvg = [numpy.average(v) for v in zip(*nonMaxChannelData)]

    #if(maxVals[bestChannel] / maxVals[worstChannel] > 2.0):
    data = [dhi - 0.5 * dlo for (dhi, dlo) in zip(frameSamples.items()[bestChannel][1], frameSamples.items()[worstChannel][1])]

    data = caliberation.RemoveDC(data)
    globalData.extend(data)

    (maxVal, voiceData, voiceMax, 
      runningCount, runningSum, runningAvg,
      runningVoiceSum, 
      runningVoiceAvg) = GetFrameStats(data, runningSum, runningVoiceSum, runningCount)

    Ndata = len(data)

    isLoudSound.extend([1 if ((maxVal / runningAvg) > 7.0) else 0] * Ndata)

    if(maxVals[bestChannel] - maxVals[worstChannel] > 10.0):
      soundDirection.extend([bestChannel+1] * Ndata)
    else:
      soundDirection.extend([0] * Ndata)

    framecount = framecount+1
    if(framecount % 1000 == 0): 
      print "done %d frames" % framecount

  N = len(globalData)
  x = range(0, N)

  plt.plot(x, [(g + 50) for g in globalData])
  plt.plot(x, [(l + 1) for l in isLoudSound])
  plt.plot(x, [(v + 3) for v in soundDirection])
  plt.show()

#------------------------------------------

Main()