all: subdirs

DIRS = capture

subdirs:
	for dir in $(DIRS); do \
		$(MAKE) -C $$dir; \
	done

install:
	for dir in $(DIRS); do \
		$(MAKE) -C $$dir install; \
	done

clean:
	rm -f {bin,capture}/{cap,reader} capture/libreader.so

