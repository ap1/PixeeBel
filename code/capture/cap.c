#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <time.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "format.h"

// 0x100 is (max_number+1) for unsigned short int
#define USINT_DIFF(a,b)        ( (unsigned short int) ((a)-(b)) )
#define USINT_NEG1             ( (unsigned short int) -1 )

#define BUF_SIZE		(2048)
unsigned int our_port = 8888;

// socket descriptor and outfile descriptor
int sd, fd;
unsigned long int filesize=0;

// read buffer
char buffer[BUF_SIZE];

struct buffer_h last_buffer_h;
unsigned int recv_count=0;

#define CALIBRATION_SAMPLES      (16)
unsigned int calibration = CALIBRATION_SAMPLES * 2;

unsigned int avg_delay = 0;
unsigned int min_avg_delay, max_avg_delay;



void sighandler(int signum)
{
   fprintf(stderr, "interrupted\n");
   close(fd);
   close(sd);
   exit(0);
}


int parse_args(int argc, char *argv[])
{
   return 0;
}

#define TRACE()      fprintf(stderr, "received %d\tseqno %u\tdelay %u\tfilesize %lu\n", \
                             recv_count, buffer->sample_seq, delay, filesize )

inline void parse_input(char *_buffer, size_t bytes)
{
   int w_bytes;
   unsigned short int delay;
   struct buffer_h *buffer = (struct buffer_h *) _buffer;

   if (bytes < sizeof(struct buffer_h)) {
      fprintf(stderr, "insufficient bytes in the received message. discarding.\n");
      return;
   }

   if (bytes != BUFFER_SIZE(buffer->num_channels, buffer->num_samples)) {
      fprintf(stderr, "got %u bytes, expected %d bytes. discarding.\n",
              (unsigned int) bytes, (int) BUFFER_SIZE(buffer->num_channels, buffer->num_samples));
      return;
   }

   delay = USINT_DIFF(buffer->send_timestamp, last_buffer_h.send_timestamp);
   recv_count++;
   filesize += bytes;

   if (USINT_DIFF(buffer->sample_seq, last_buffer_h.sample_seq) != 1 && !calibration) {
      fprintf(stderr, "%d packet(s) missed!\n",
              USINT_DIFF(buffer->sample_seq, last_buffer_h.sample_seq)-1);
      TRACE();

   } else if (calibration>=CALIBRATION_SAMPLES) {
      --calibration;
   } else if (calibration) {
      // currently calibrating expected delay

      //fprintf(stderr, "calibrating: sample %d delay %u\n", calibration, delay);

      avg_delay += delay;
      if (--calibration == 0) {
         avg_delay /= CALIBRATION_SAMPLES;
         min_avg_delay = avg_delay * 9 / 10;
         max_avg_delay = avg_delay * 11 / 10;
         fprintf(stderr, "calibrated: expected delay range %d..%d us\n", min_avg_delay, max_avg_delay);
      }

   } else if (delay < min_avg_delay || delay > max_avg_delay) {
      // calibration complete.. watch out for unexpected jitter

      fprintf(stderr, "unexpected delay seen %d\n", delay);
      TRACE();
   } 

   if (recv_count % 500 == 0) {
      TRACE();
   }

   w_bytes = write(fd, buffer, bytes);
   assert( w_bytes == bytes );

   last_buffer_h = *buffer;   // struct copy
}


int main(int argc, char *argv[])
{
   char out_file[64];
   int res, flags;
   unsigned int remote_addr_len;
   int bytes;
   struct sockaddr_in our_addr, remote_addr;

   if ((res = parse_args(argc, argv)) < 0) {
      return res;
   }

   if ((sd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
      fprintf(stderr, "socket() failed\n");
      return -1;
   }

   our_addr.sin_family = AF_INET;
   our_addr.sin_addr.s_addr = htonl(INADDR_ANY);
   our_addr.sin_port = htons(our_port);

   if ((res = bind(sd, (struct sockaddr *) &our_addr, sizeof(our_addr))) < 0) {
      fprintf(stderr, "bind to port %d failed\n", our_port);
      return -1;
   }

   signal(SIGINT, sighandler);

   // out file
   sprintf(out_file, "samples-%lu.dat", time(NULL));
   if ((fd = open(out_file, O_RDWR | O_TRUNC | O_CREAT, S_IRUSR | S_IWUSR))<0) {
      fprintf(stderr, "can't open %s for writing\n", out_file);
      close(sd);
      return -1;
   }

   flags = 0;
   fprintf(stderr, "writing to %s, listening..\n", out_file);

   while (1) {
      remote_addr_len = sizeof(remote_addr);
      bytes = recvfrom(sd, buffer, BUF_SIZE, flags, (struct sockaddr *) &remote_addr,
                       &remote_addr_len);
      if (bytes < 0) {
         fprintf(stderr, "recvfrom failed\n");
         continue;
      }

      parse_input(buffer, bytes);
   }

   close(sd);
   return 0;
}

