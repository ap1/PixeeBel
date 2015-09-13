#include <stdio.h>

#include "pb_msg.h"
#include "msg_queue.h"

int
main(int argc, char *argv[])
{
   int i;
   msg foo;
   int qid;


   qid = mg_open( "/tmp/pb_msgqueue" );

   printf("qid %d\n", qid);

   foo.mtype = 1;

   for (i=0; i<10; i++) {
      foo.data[0][i] = i;
   }
   mg_send( qid, &foo );

   for (i=0; i<10; i++) {
      foo.data[0][i] = 10+i;
   }
   mg_send( qid, &foo );

   mg_recv( qid, foo.mtype, &foo );
   mg_recv( qid, foo.mtype, &foo );

   mg_release( qid );

   return 0;
}

