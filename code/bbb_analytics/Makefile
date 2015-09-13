all: sender

CFLAGS=-ggdb -Wall -I../bbb_common/

COMMON_SOURCES=../bbb_common/msg_queue.c
COMMON_HEADERS=../bbb_common/msg_queue.h ../bbb_common/pb_msg.h

sender: sender.c $(COMMON_SOURCES) $(COMMON_HEADERS)
	gcc $(CFLAGS) -o sender sender.c $(COMMON_SOURCES)

clean:
	rm -f sender

