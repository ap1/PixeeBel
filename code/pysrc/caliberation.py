import numpy
import subprocess
import math
from math import sqrt, exp
import matplotlib.pyplot as plt
import random

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
  return [s if (s > maxval or s < minval) else 0.0 for s in signal]

def RemoveDC(data):
  dc = numpy.average(data)
  #print "dc value is " + str(dc)
  return [v - dc for v in data]

def SumList(arr1, arr2):
  return [a1+a2 for a1, a2 in zip(arr1, arr2)]

def SumListVar(arr1, val):
  return [a1+val for a1 in arr1]

def Kmeans(arr, n):
  count = len(arr)
  labels = []
  Niters = 8

  labels = [random.randint(0,n-1) for v in arr]

  centroids = [random.randint(0,count-1) for i in range(0, n)]

  onebymaxval = 1.0/max(arr)

  for iteration in range(0, Niters):
    print iteration
    locSums = [0.0] * n
    locCounts = [0.0] * n
    for (li,l) in enumerate(labels):
      locSums[l] = locSums[l] + li * abs(arr[li] * onebymaxval)
      locCounts[l] = locCounts[l] + 1 * abs(arr[li] * onebymaxval)

    centroids = sorted([ locSums[si] / locCounts[si] if locCounts[si] > 0 else centroids[si] for si in range(0,n)])



    print centroids
    #print labels

    labels = [numpy.argmin([abs(li - c) for c in centroids]) for (li) in range(0, count)]

  return (centroids, labels)

def SuppressNonMax(signal, neighborhood):
  count = len(signal)

  halfn = int(neighborhood/2)

  outsignal = [s for s in signal]

  for si in range(0, count):
    for fi in range(max(si-neighborhood, 0), min(si, count)):
      if(abs(signal[fi]) > 0.0):
        outsignal[si] = 0.0
        break

  return outsignal

def FindPeaks(samples):
  count = len(samples)
  x = range(0, count)

  #signal_mic0 = RemoveDC(signal_mic0)

  #if removing avgVal
  avgVal = numpy.average(samples)

  #if thresholding
  cleansamples = [0] * count
  for si in x:
    s = samples[si] - avgVal
    if (s < 45.0): s = 0.0
    cleansamples[si] = s

  peaks = SuppressNonMax(cleansamples, 50)

  peak_locations = numpy.nonzero(peaks)

  # print "found %d peaks" % (len(peak_locations[0]))

  # plt.plot(x, samples)
  # plt.plot(x, peaks)
  # plt.show()

  return peak_locations[0]

def AggregatePeaks(peaks0, peaks1, peaks2, peaks3):
  allpeaks = sorted(numpy.concatenate((peaks0, peaks1, peaks2, peaks3)))

  pi = 0
  cleaned_peaks = []
  while (pi < (len(allpeaks)-1)):
    # new peak group
    v = allpeaks[pi]
    nchannels = 1
    while(pi < (len(allpeaks)-1) and abs(v - allpeaks[pi+1]) < 200):
      pi = pi + 1
      nchannels = nchannels + 1
    pi = pi + 1
    if(nchannels >= 3):
      cleaned_peaks.append((v, nchannels))

  return cleaned_peaks

def CaliberateChannels(samples_mic0, samples_mic1, samples_mic2, samples_mic3):
  print "Caliberating channels"
  # find the peaks and tell us channel order
  peaks0 = FindPeaks(samples_mic0)
  peaks1 = FindPeaks(samples_mic1)
  peaks2 = FindPeaks(samples_mic2)
  peaks3 = FindPeaks(samples_mic3)

  # print peaks0
  # print peaks1
  # print peaks2
  # print peaks3

  channelMap = [["N", -1], ["E", -1], ["W", -1], ["S", -1]]

  cleaned_peaks = AggregatePeaks(peaks0, peaks1, peaks2, peaks3)

  print cleaned_peaks

  # we should have 8 peaks
  if(len(cleaned_peaks) == 8):

    closestChannels = []

    for (peak, nch) in cleaned_peaks:
      v0 = peaks0[numpy.argmin([abs(peak - v) for v in peaks0])]
      v1 = peaks1[numpy.argmin([abs(peak - v) for v in peaks1])]
      v2 = peaks2[numpy.argmin([abs(peak - v) for v in peaks2])]
      v3 = peaks3[numpy.argmin([abs(peak - v) for v in peaks3])]

      dirList = []

      if(abs(v0 - peak) < 200.0): dirList.append((v0, 0))
      if(abs(v1 - peak) < 200.0): dirList.append((v1, 1))
      if(abs(v2 - peak) < 200.0): dirList.append((v2, 2))
      if(abs(v3 - peak) < 200.0): dirList.append((v3, 3))

      #print [x[1] for x in sorted(dirList)]
      closestChannels.append(min(dirList)[1])

    for snapi in range(0, 4):
      if(closestChannels[2*snapi] == closestChannels[2*snapi + 1]):
        channelMap[snapi][1] = closestChannels[2*snapi]
      else:
        print "Caliberation failed. Closest channels don't match."

  else:

    print "Caliberation failed. We don't have 8 peaks."

  channelMap = dict([tuple(cm) for cm in channelMap])
  print "Alignment Map:",
  print channelMap
  return channelMap