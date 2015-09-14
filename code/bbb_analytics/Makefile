all: sender drain pb_analyzer

CFLAGS=-ggdb -Wall -I../bbb_common/

COMMON_SOURCES=../bbb_common/msg_queue.c
COMMON_HEADERS=../bbb_common/msg_queue.h ../bbb_common/pb_msg.h

sender: sender.c $(COMMON_SOURCES) $(COMMON_HEADERS)
	gcc $(CFLAGS) -o sender sender.c $(COMMON_SOURCES)

drain: drain.c $(COMMON_SOURCES) $(COMMON_HEADERS)
	gcc $(CFLAGS) -o drain drain.c $(COMMON_SOURCES)

analyze.o: analyze.c
	gcc $(CFLAGS) -c analyze.c

pb_analyzer: pb_analyzer.c $(COMMON_SOURCES) $(COMMON_HEADERS) analyze.o
	gcc $(CFLAGS) -o pb_analyzer pb_analyzer.c analyze.o $(COMMON_SOURCES)

clean:
	rm -f sender drain pb_analyzer analyze.o

