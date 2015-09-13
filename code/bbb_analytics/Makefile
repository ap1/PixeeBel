all: sender

CFLAGS=-ggdb -Wall

SENDERFILES=sender.o msg_queue.o

%.o: $.c pb_msg.h msg_queue.h
	gcc $(CFLAGS) -c -o $@ $<

sender: $(SENDERFILES)
	gcc $(CFLAGS) -o sender $(SENDERFILES)

clean:
	rm -f sender $(SENDERFILES)

