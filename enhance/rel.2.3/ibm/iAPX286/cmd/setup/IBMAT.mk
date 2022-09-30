# makefile for the setup program. "setup" is used to emulate the
# dos version of setup. 

CC = /bin/cc
CFLAGS = -O -s
ROOT	= ../../../dist

all:	setup 

setup: setup.o dtlib.o
	$(CC) $(CFLAGS) setup.o dtlib.o -o setup

dumpcmos: dumpcmos.c
	$(CC) $(CFLAGS) dumpcmos.c -o dumpcmos

clean:
	-rm -f setup dumpcmos *.o

clobber:
	-rm -f $(ROOT)/etc/setup

install:	all clobber
	cp setup $(ROOT)/etc/setup
	chmod 510 $(ROOT)/etc/setup
	chown bin $(ROOT)/etc/setup
	chgrp sys $(ROOT)/etc/setup
