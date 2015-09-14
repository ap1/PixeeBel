#include <unistd.h>

#include "pb_msg.h"
#include "bin_reader.h"
#include "pb_visualize.h"

static int nr_bins;
bin bins[MAX_BINS];

int main( int argc, char *argv[] )
{
   unsigned int i;
   unsigned int channel = 0;

   nr_bins = bins_load( "bin_defs.txt", bins );

   visualize_init();

   while ( 1 ) {

      for ( i = 0; i < nr_bins; i++ ) {
         visualize_send( i, channel );

         sleep( 1 );
      }

      channel = ( channel + 1 ) % NR_CHANNELS;
   }

   return 0;
}

