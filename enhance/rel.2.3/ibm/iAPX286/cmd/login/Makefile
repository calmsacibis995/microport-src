# makefile for the setup program. "setup" is used to emulate the
# dos version of setup. 

ROOT	= ../../../dist
CFLAGS = -O -s -I../.. -DSYS_NAME $(OPTIONS)
LDFLAGS = -O -s
CONS=-DCONSOLE='"/dev/console"'

all:	getty init

init: init.o
	$(CC) $(CFLAGS) init.o -o init

getty: getty.c
	$(CC) $(CFLAGS) getty.c -o getty

login: login.c $(FRC)
	$(CC) $(CONS) $(CFLAGS) login.c -o login

clean:
	-rm -f init getty login *.o

clobber:
	-rm -f $(ROOT)/etc/init
	-rm -f $(ROOT)/etc/telinit
	-rm -f $(ROOT)/etc/getty
	-rm -f $(ROOT)/bin/login

install:	all clobber
	cp init $(ROOT)/etc/init
	ln $(ROOT)/etc/init $(ROOT)/etc/telinit
	cp getty $(ROOT)/etc/getty
	cp login $(ROOT)/bin/login
	chmod 510 $(ROOT)/etc/init $(ROOT)/etc/getty $(ROOT)/bin/login
	chown bin $(ROOT)/etc/init $(ROOT)/etc/getty $(ROOT)/bin/login
	chgrp sys $(ROOT)/etc/init $(ROOT)/etc/getty $(ROOT)/bin/login

