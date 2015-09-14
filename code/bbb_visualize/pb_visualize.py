#!/usr/bin/env python

import socket
import struct
import time
import select
import screen


NR_CHANNELS = 4
visibleMs = 500
selectMs = 60


# Each queue element is tuple with ( time-to-vanish, channel-id, bin-id )
queue = []
bind_port = ( '127.0.0.1', 3000 )


def millis():
   return int( time.time() * 1000.0 )


def evalSelectDelay( nowMs ):

   if not queue:
      return selectMs / 1000.0

   nextEventTimeMs = queue[ -1 ][ 0 ]

   deltaMs = max( nextEventTimeMs - nowMs, 0 )

   return deltaMs / 1000.0


def maybeDrainList( nowMs ):

   if not queue:
      return

   while queue and queue[-1][0] <= nowMs:
      # pop the last item
      ( _, channelId, binId ) = queue.pop()
      screen.hide( nowMs, channelId, binId )


if __name__ == "__main__":

   # edges = dict( ( channel, ScreenEdge( channel ) ) for channel in xrange( NR_CHANNELS ) )

   sock = socket.socket( socket.AF_INET, socket.SOCK_DGRAM )
   sock.bind( bind_port )

   screen.init()

   while True:
      nowMs = millis()
      timeout = evalSelectDelay( nowMs )

      readReady, _, _ = select.select( [ sock ], [], [], timeout )

      if not readReady:
         maybeDrainList( nowMs )
         continue

      data, _ = sock.recvfrom( 4 )

      binId = struct.unpack( "!H", data[0:2] )[0]
      channelId = struct.unpack( "!H", data[2:4] )[0]

      queue.insert( 0, ( nowMs + visibleMs, channelId, binId ) )
      screen.show( nowMs, channelId, binId )

      maybeDrainList( nowMs )

