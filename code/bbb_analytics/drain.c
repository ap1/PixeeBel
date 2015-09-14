#include <stdio.h>

#include "pb_msg.h"
#include "msg_queue.h"

int
main(int argc, char *argv[])
{
   int qid;

   qid = mg_open( "/tmp/pb_msgqueue" );

   printf("qid %d\n", qid);

   while ( 1 ) {
      mg_drain( qid, MG_TYPE );
   }

   mg_release( qid );

   return 0;
}

