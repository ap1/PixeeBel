#include <stdio.h>
#include <signal.h>

#include "pb_msg.h"
#include "msg_queue.h"
#include "analyze.h"

int hit_sigint = 0;


static void
sigint_handler( int x )
{
	hit_sigint = 1;
}


int
main(int argc, char *argv[])
{
   int qid;
   msg m;

   qid = mg_open( "/tmp/pb_msgqueue" );

   printf( "Opened qid %d\n", qid );

   // Draining queued data
   printf( "Draining pending data\n" );
   mg_drain( qid, MG_TYPE );
   printf( "   done\n" );

	signal( SIGINT, sigint_handler );

   while ( !hit_sigint && (mg_recv( qid, MG_TYPE, &m ) > 0) ) {
      do_magic( &m );
   }

   mg_release( qid );

   return 0;
}

