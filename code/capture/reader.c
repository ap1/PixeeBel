#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

#include "format.h"

#define BUF_SIZE		(2048)

void usage()
{
   printf("Usage: reader <samplesfile> <channel>\n\n");
}


void process_chunk(struct buffer_h *buf, int channel)
{
   unsigned int i;
   Sample sample;

   for (i=0; i<buf->num_samples; i++) {
      sample = buf->data[DATA_OFFSET(buf->num_channels, i, channel)];
      printf("%d\n", sample);
   }
}


/* return 0 on failure, otherwise number of samples */
int read_one_frame(int fd, struct buffer_h *buf)
{
   int n;
   unsigned bytes;

   n = read(fd, buf, sizeof(struct buffer_h));
   if (n != sizeof(struct buffer_h)) {
      fprintf(stderr, "can't read the header\n");
      return 0;
   }

   fprintf(stderr, "chunk: seq %d channels %d samples %d ts %d\n",
           buf->sample_seq, buf->num_channels,
           buf->num_samples, buf->send_timestamp);

   bytes = DATA_MEM(buf->num_channels, buf->num_samples);
   n = read(fd, buf->data, bytes);
   if (n!=bytes) {
      fprintf(stderr, "Couldn't read the whole chunk. Read %d/%d\n", n, bytes);
      n -= n % DATA_MEM(buf->num_channels, 1);
      buf->num_samples = n / DATA_MEM(buf->num_channels, 1);
      return buf->num_samples;
   }

   return buf->num_samples;
}


void read_all_frames(int fd, int channel)
{
   unsigned int num_samples;
   char buf[BUF_SIZE];

   while ((num_samples=read_one_frame(fd, (struct buffer_h *) buf))) {
      fprintf(stderr, "read %d samples\n", num_samples);

      process_chunk((struct buffer_h *) buf, channel);
   }
}


int main(int argc, char *argv[])
{
   int fd;

   if (argc != 3) {
      usage();
      return 0;
   }

   if ((fd = open(argv[1], O_RDONLY))==-1) {
      printf("Can't open %s for reading\n", argv[1]);
      return -1;
   }

   read_all_frames(fd, atoi(argv[2]));

   close(fd);

   return 0;
}

