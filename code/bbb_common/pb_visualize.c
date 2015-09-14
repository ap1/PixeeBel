#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <string.h>
#include <assert.h>

#include "pb_visualize.h"

int sd = -1;
struct sockaddr_in s_other;
unsigned int slen;

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
visualize_send( uint16_t id, uint16_t channel )
{
   int res;
   assert( sd != -1 );

   uint32_t data = id;
   data = data << 16;
   data |= channel;

   data = htonl( data );

   res = sendto( sd, &data, sizeof( data ), 0, (struct sockaddr *) &s_other, slen );
   assert( res == sizeof( data ) );
}

