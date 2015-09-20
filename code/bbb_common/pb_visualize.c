#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <string.h>
#include <assert.h>

#include "pb_visualize.h"

int sd = -1;
struct sockaddr_in s_other;
unsigned int slen;

typedef struct udp_msg_ {
   uint16_t bid;
   uint16_t cid;
   uint16_t mag;
} udp_msg;

void
visualize_init()
{
   slen = sizeof( s_other );
   assert( sd == -1 );

   sd = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP );
   assert( sd >= 0 );

   memset( (char *) &s_other, 0, sizeof( s_other ) );

   s_other.sin_family = AF_INET;
   s_other.sin_port = htons( PORT );

   assert( inet_aton( "127.0.0.1", &s_other.sin_addr ) );

}


void
visualize_send( uint16_t id, uint16_t channel, uint16_t mag )
{
   int res;
   udp_msg m;

   assert( sd != -1 );

   m.bid = htons( id );
   m.cid = htons( channel );
   m.mag = htons( mag );

   res = sendto( sd, &m, sizeof( m ), 0, (struct sockaddr *) &s_other, slen );
   assert( res == sizeof( m ) );
}

