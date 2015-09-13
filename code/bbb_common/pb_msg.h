#ifndef __PB_MSG_H
#define __PB_MSG_H

#include <stdint.h>


#define NR_CHANNELS              (4)
#define SAMPLES_PER_MSG         (256)

// A single sample is 12 bits wide, use uint16 for storing it
typedef uint16_t     SAMPLE;

// Per channel data in a single message
typedef SAMPLE channel_data[SAMPLES_PER_MSG];



typedef struct msg_ {
   long mtype;
   channel_data data[NR_CHANNELS];
} msg;


#define MSG_DATA_SIZE            (sizeof(msg) - sizeof(long))

#endif // __PB_MSG_H

