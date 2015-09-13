#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <assert.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdio.h>

#include "msg_queue.h"


/* Do ftok(path). Creates the world RW file first */
static
key_t mg_get_key( char *path )
{
   int fd;
   key_t key;

   fd = open( path, O_RDWR | O_CREAT | O_TRUNC );
   assert( fd > -1 );
   fchmod( fd, S_IRWXU | S_IRWXG | S_IRWXO );
   close( fd );

   key = ftok( path, 0x1 );
   assert( key > -1 );

   return key;
}


/* This method calls ftok on path and returns the result of msgget */
int
mg_open( char *path )
{
   int qid;
   key_t key = mg_get_key( path );

   qid = msgget( key, IPC_CREAT | 0666 );
   assert( qid > -1 );

   return qid;
}


/* Destroy the message queue */
void
mg_release( int qid )
{
   int res;

   res = msgctl( qid, IPC_RMID, NULL );
   assert( res > -1 );
}



int
mg_send( int qid, msg *m )
{
   int res;
   res = msgsnd( qid, m, MSG_DATA_SIZE, 0 );

   assert( res > -1 );

   return res;
}


int
mg_recv( int qid, long msgtyp, msg *m )
{
   int bytes;
   bytes = msgrcv( qid, m, MSG_DATA_SIZE, msgtyp, 0 );

   printf( "read (%d) msg received %d\n", bytes, m->data[0][1] );

   return bytes;
}


unsigned long int
mg_length( int qid )
{
   struct msqid_ds buf;

   assert( msgctl( qid, IPC_STAT, &buf ) > -1 );

   return (unsigned long int) buf.msg_qnum;
}


unsigned int
mg_drain( int qid, long msgtyp )
{
   unsigned int count=0;
   msg m;

   while( mg_length( qid ) > 0 ) {
      mg_recv( qid, msgtyp, &m );
      count++;
   }

   return count;
}

