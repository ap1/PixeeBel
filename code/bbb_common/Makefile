all: test_visualize_sender

CFLAGS=-Wall -ggdb

COMMONFILES=pb_visualize.c bin_reader.c
TVSFILES=test_visualize_sender.c $(COMMONFILES)

test_visualize_sender: $(TVSFILES)
	gcc $(CFLAGS) -o test_visualize_sender $(TVSFILES)

clean:
	rm -f test_visualize_sender

