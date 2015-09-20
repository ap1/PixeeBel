all: sender drain pb_analyzer bin_recognizer

CFLAGS=-ggdb -Wall -I../bbb_common/

OFLAGS = -O2

COMMON_SOURCES=../bbb_common/msg_queue.c
COMMON_HEADERS=../bbb_common/msg_queue.h ../bbb_common/pb_msg.h

sender: sender.c $(COMMON_SOURCES) $(COMMON_HEADERS)
	gcc $(CFLAGS) -o sender sender.c $(COMMON_SOURCES)

drain: drain.c $(COMMON_SOURCES) $(COMMON_HEADERS)
	gcc $(CFLAGS) -o drain drain.c $(COMMON_SOURCES)

obj/analyze.o: analyze.c
	gcc $(CFLAGS) -c analyze.c -o obj/analyze.o

pb_analyzer: pb_analyzer.c $(COMMON_SOURCES) $(COMMON_HEADERS) obj/analyze.o
	gcc $(CFLAGS) -o pb_analyzer pb_analyzer.c obj/analyze.o $(COMMON_SOURCES)

obj/fft4g_h.o: ../fft/fft4g_h.c
	$(CC) $(CFLAGS) $(OFLAGS) -c ../fft/fft4g_h.c -o obj/fft4g_h.o

obj/bin_reader.o: ../bbb_common/bin_reader.c ../bbb_common/bin_reader.h
	gcc $(CFLAGS) -c ../bbb_common/bin_reader.c -o obj/bin_reader.o

obj/msg_queue.o: ../bbb_common/msg_queue.c ../bbb_common/msg_queue.h
	gcc $(CFLAGS) -c ../bbb_common/msg_queue.c -o obj/msg_queue.o

obj/pb_visualize.o: ../bbb_common/pb_visualize.c ../bbb_common/pb_visualize.h
	gcc $(CFLAGS) -c ../bbb_common/pb_visualize.c -o obj/pb_visualize.o

bin_recognizer: obj/fft4g_h.o bin_recognizer.c obj/bin_reader.o obj/msg_queue.o obj/pb_visualize.o
	gcc $(CFLAGS) -lm -o bin_recognizer bin_recognizer.c obj/fft4g_h.o obj/bin_reader.o obj/msg_queue.o obj/pb_visualize.o

clean:
	rm -f *.o obj/*.o sender drain pb_analyzer bin_recognizer

