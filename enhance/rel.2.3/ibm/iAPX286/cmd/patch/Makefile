# makefile for patch.c

CC	= /bin/cc
CFLAGS	= -s
ROOT	= ../../../dist


all:	patch

patch:
	$(CC) $(CFLAGS) -O patch.c -o patch -lld

clean:
	-rm -f patch *.o

clobber:
	-rm -f $(ROOT)/etc/patch

install:	all clobber
	cp patch $(ROOT)/etc/patch
	chmod 510 $(ROOT)/etc/patch
	chown bin $(ROOT)/etc/patch
	chgrp sys $(ROOT)/etc/patch
