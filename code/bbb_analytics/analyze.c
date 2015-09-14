#include <stdio.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


#include "analyze.h"


float channels[NR_CHANNELS][SAMPLES_PER_MSG];


#define SUM_DATA( len, data, sum )    do { \
            register unsigned int i; sum = 0; \
            for ( i = 0; i < len; ++i ) { sum += data[i]; } \
         } while ( 0 ) 

#define ADD_SCALAR( len, data, scalar )    do { \
            register unsigned int i; \
            for ( i = 0; i < len; ++i ) { data[i] += scalar; } \
         } while ( 0 ) 

#define TO_FLOAT( len, in, out ) do { \
            register unsigned int i; \
            for ( i = 0; i < len; ++i ) { out[i] = in[i]; } \
         } while ( 0 )

int fds[NR_CHANNELS] = { -1, -1, -1, -1 };

void
stage1( SAMPLE *samples, float *floats )
{
   uint32_t sum = 0;
   float avg = 0;

   SUM_DATA( SAMPLES_PER_MSG, samples, sum );

   avg = (float) sum / SAMPLES_PER_MSG;

   TO_FLOAT( SAMPLES_PER_MSG, samples, floats );

   ADD_SCALAR( SAMPLES_PER_MSG, floats, -avg );

   printf( "%f ", avg );
}


void
do_magic( msg *m )
{
   unsigned int channel;
   char buf[32];

   if ( fds[0] == -1 ) {
      for ( channel=0; channel < NR_CHANNELS; ++channel ) {
         sprintf( buf, "mic%d.txt", channel );
         fds[channel] = open( buf, O_RDWR | O_CREAT | O_TRUNC );
      }
   }

   for ( channel = 0; channel < NR_CHANNELS; ++channel ) {
      // Operate on input m->data[channel] and write the result
      // as floats in channels[channel]
      stage1( m->data[channel], channels[channel] );
      sprintf( buf, "%f\n", channels[channel][SAMPLES_PER_MSG - 1] );
      write( fds[channel], buf, strlen( buf ) );
   }
}

