#!/usr/bin/env python

import socket
import struct
import time
import select

import pb_bin_reader

visibleMs = 200
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


def show( nowMs, channelId, binId ):
   print "+%d:\tcid %d bid %d\t\tqlen %d" % ( nowMs, channelId, binId, len( queue ) )


def hide( nowMs, channelId, binId ):
   print "-%d:\tcid %d bid %d\t\tqlen %d" % ( nowMs, channelId, binId, len( queue ) )


def maybeDrainList( nowMs ):

   if not queue:
      return

   while queue and queue[-1][0] <= nowMs:
      # pop the last item
      ( _, channelId, binId ) = queue.pop()
      hide( nowMs, channelId, binId )


if __name__ == "__main__":

   bins = pb_bin_reader.load( "../bbb_common/bin_defs.txt" )
   print bins

   sock = socket.socket( socket.AF_INET, socket.SOCK_DGRAM )
   sock.bind( bind_port )

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
      show( nowMs, channelId, binId )

      maybeDrainList( nowMs )

   exit( 0 )

