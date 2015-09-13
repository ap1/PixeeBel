#ifndef __MSG_QUEUE_H
#define __MSG_QUEUE_H


#include <sys/ipc.h>
#include "pb_msg.h"


// Returns queueid
int mg_open( char *path );

// Destroy the message queue
void mg_release( int qid );

// Send a message
int mg_send( int qid, msg *m );

// Receive a message
int mg_recv( int qid, long msgtyp, msg *m );

// Get number of messages queued
unsigned long int mg_length( int qid );

// Best effort at draining the queue. Returns
// number of messages read (and discarded).
// Note that this can end up indefinitely reading
// from the queue trying to drain it if a source is
// aggressively writing messages.
unsigned int mg_drain( int qid, long msgtyp );

#endif // __MSG_QUEUE_H

