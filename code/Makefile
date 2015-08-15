all: cap reader

CFLAGS=-Wall -ggdb

cap: cap.c format.h
	gcc ${CFLAGS} -o cap cap.c

reader: reader.c format.h
	gcc ${CFLAGS} -o reader reader.c

clean:
	rm -f cap reader

