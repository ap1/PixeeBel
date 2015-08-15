#ifndef __FORMAT_H
#define __FORMAT_H


typedef unsigned char   Sample;
#define SAMPLE_BYTES    (sizeof(Sample))

struct __attribute__((__packed__)) buffer_h {
   unsigned short int sample_seq;
   unsigned char num_channels;
   unsigned char num_samples;
   unsigned short int send_timestamp;
   Sample data[0];
};

#define DATA_MEM(num_channels, num_samples)        \
         ( SAMPLE_BYTES*(num_channels)*(num_samples) )

#define BUFFER_SIZE(num_channels, num_samples)     (sizeof(struct buffer_h) + DATA_MEM(num_channels, num_samples) )

#define DATA_OFFSET(num_channels, i, channel)      \
         ((num_channels)*(i) + (channel))
#endif

