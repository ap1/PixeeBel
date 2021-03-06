#ifndef __PB_MSG_H
#define __PB_MSG_H

#include <stdint.h>

// This interrupt is generated by PRU0 is done collect all samples
#define PRU0_ARM_DONE_INTERRUPT       (19)

#define NR_CHANNELS              (4)
#define SAMPLES_PER_MSG         (256)

// A single sample is 12 bits wide, use uint16 for storing it
typedef uint16_t     SAMPLE;

// Per channel data in a single message
typedef SAMPLE channel_data[SAMPLES_PER_MSG];


// Shared memory starts with this header
struct header {
   uint32_t magic;
   uint16_t seqno;
   uint16_t count;
};


typedef struct msg_ {
   long mtype;
   channel_data data[NR_CHANNELS];
} msg;


#define MSG_DATA_SIZE            (sizeof(msg) - sizeof(long))

#endif // __PB_MSG_H

