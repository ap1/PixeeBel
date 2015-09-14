#ifndef __PB_BINS_H
#define __PB_BINS_H

#define MAX_BINS      (16)


typedef struct bin_ {
  uint16_t  id;
  uint16_t  freq_low;
  uint16_t  freq_high;

  uint16_t  threshold;
  char    name[32];
} bin;

bin bins[MAX_BINS] = { {  } };

static int nr_bins = 4;

#endif