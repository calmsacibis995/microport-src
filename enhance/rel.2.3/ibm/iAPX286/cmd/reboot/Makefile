# makefile for patch.c

CC	= /bin/cc
CFLAGS	= -s
ROOT	= ../../../dist
INC	= ../..


all:	reboot

reboot:
	$(CC) $(CFLAGS) -I$(INC) -O reboot.c -o reboot -lld

clean:
	-rm -f reboot *.o

clobber:
	-rm -f $(ROOT)/etc/reboot

install:	all clobber
	cp reboot $(ROOT)/etc/reboot
	chmod 510 $(ROOT)/etc/reboot
	chown bin $(ROOT)/etc/reboot
	chgrp sys $(ROOT)/etc/reboot
