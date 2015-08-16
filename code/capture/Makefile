all: cap reader libreader.so

CFLAGS=-Wall -ggdb
   
libreader.so: reader.c format.h
	gcc ${CFLAGS} -fPIC -shared -o libreader.so reader.c

cap: cap.c format.h
	gcc ${CFLAGS} -o cap cap.c

reader: reader.c format.h
	gcc ${CFLAGS} -o reader reader.c

install: cap reader
	cp cap reader ../bin/

clean:
	rm -f cap reader libreader.so

