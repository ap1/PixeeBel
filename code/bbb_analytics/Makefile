all: sender drain pb_analyzer bin_recognizer

CFLAGS=-ggdb -Wall -I../bbb_common/

OFLAGS = -O2

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

fft4g_h.o: ../fft/fft4g_h.c
	$(CC) $(CFLAGS) $(OFLAGS) -c ../fft/fft4g_h.c -o fft4g_h.o

bin_reader.o: ../bbb_common/bin_reader.c ../bbb_common/bin_reader.h
	gcc $(CFLAGS) -c ../bbb_common/bin_reader.c

msg_queue.o: ../bbb_common/msg_queue.c ../bbb_common/msg_queue.h
	gcc $(CFLAGS) -c ../bbb_common/msg_queue.c

bin_recognizer: fft4g_h.o bin_recognizer.c bin_reader.o msg_queue.o
	gcc $(CFLAGS) -lm -o bin_recognizer bin_recognizer.c fft4g_h.o bin_reader.o msg_queue.o

clean:
	rm -f sender drain pb_analyzer analyze.o fft4g_h.o bin_recognizer

