#!/usr/bin/env python

import socket
import struct
import time
import select
import screen
import signal
import sys


NR_CHANNELS = 4
visibleMs = 200
selectMs = 60


# Each queue element is tuple with ( time-to-vanish, channel-id, bin-id )
queue = []
bind_port = ( '127.0.0.1', 3000 )

interrupted = False


# Control+C handler
def sigint_handler( signal, frame ):
   global interrupted
   interrupted = True
   print "SIGINT"


def millis():
   return int( time.time() * 1000.0 )


def evalSelectDelay( nowMs ):

   if not queue:
      return selectMs / 1000.0

   nextEventTimeMs = queue[ -1 ][ 0 ]

   deltaMs = max( min( nextEventTimeMs - nowMs, selectMs ), 0 )

   return deltaMs / 1000.0


def maybeDrainList( nowMs ):

   if not queue:
      return

   while queue and queue[-1][0] <= nowMs:
      # pop the last item
      ( _, channelId, binId, mag ) = queue.pop()
      screen.hide( nowMs, channelId, binId, mag, queue=queue )


if __name__ == "__main__":

   sock = socket.socket( socket.AF_INET, socket.SOCK_DGRAM )
   sock.bind( bind_port )

   screen.init()

   signal.signal( signal.SIGINT, sigint_handler )

   iter = 0

   while not interrupted:
      iter += 1

      if iter % 100 == 0:
         print "Queue length %d" % len( queue )

      nowMs = millis()
      timeout = evalSelectDelay( nowMs )

      try:
         readReady, _, _ = select.select( [ sock ], [], [], timeout )
      except select.error:
         continue

      if not readReady:
         maybeDrainList( nowMs )
         continue

      data, _ = sock.recvfrom( 4, 6 )

      binId = struct.unpack( "!H", data[0:2] )[0]
      channelId = struct.unpack( "!H", data[2:4] )[0]
      mag = struct.unpack( "!H", data[4:6] )[0]

      queue.insert( 0, ( nowMs + visibleMs, channelId, binId, mag ) )
      screen.show( nowMs, channelId, binId, mag, queue=queue )

      maybeDrainList( nowMs )

   screen.stop()
   sys.exit( 0 )
