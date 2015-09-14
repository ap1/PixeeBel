#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <assert.h>

#include "bin_reader.h"


static int
next_param( char *buf, unsigned int start )
{
#ifdef DEBUG
   printf( "> %d %d %c\n", start, buf[start], buf[start] );
#endif

   while ( buf[start] ) {
      if ( buf[start] != ' ' && buf[start] != '\t' ) {
         return start;
      }
      start++;
   }

   printf( "Can't parse %s", buf );
   assert( 0 );

   return -1;
}


static int
next_blank( char *buf, unsigned int start )
{
   while ( buf[start] ) {
      if ( buf[start] == ' ' || buf[start] == '\t' ) {
         return start;
      }
      start++;
   }

   printf( "Can't parse %s", buf );
   assert( 0 );

   return -1;
}



unsigned int
bins_load( char *filename, bin *bins )
{
   FILE *f;
   unsigned int nr_bins = 0;
   char buf[1024];
   int offset;

   f = fopen( filename, "r" );
   assert( f );

   while ( fgets( buf, sizeof( buf ), f ) && !feof( f ) ) {
      if ( buf[0] == '#' || strlen( buf ) < 7 ) {
         continue;
      }

      sscanf( buf, "%s ", bins[nr_bins].name );

      offset = next_param( buf, strlen( bins[nr_bins].name ) );
      bins[nr_bins].freq_low = atoi( &buf[offset] ); 

      offset = next_param( buf, next_blank( buf, offset ) );
      bins[nr_bins].freq_high = atoi( &buf[offset] );

      offset = next_param( buf, next_blank( buf, offset ) );
      bins[nr_bins].threshold = atoi( &buf[offset] );

#ifdef DEBUG
      printf( "-%s-%d-%d-%d-\n", bins[nr_bins].name,
              bins[nr_bins].freq_low, bins[nr_bins].freq_high,
              bins[nr_bins].threshold );
#endif

      bins[nr_bins].id = nr_bins;

      nr_bins++;
   }

   fclose( f );

   return nr_bins;
}

