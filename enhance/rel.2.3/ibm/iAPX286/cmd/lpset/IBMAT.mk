CC	= /bin/cc
CFLAGS	= -s
INC1	= ../..
ROOT	= ../../../dist

all:
	$(CC) $(CFLAGS) -I$(INC1) lpset.c -o lpset

clean:
	-rm -f lp[gs]et *.o

clobber:
	-rm -f $(ROOT)/etc/lpset

install:	all clobber
	cp lpset $(ROOT)/etc/lpset
	chmod 511 $(ROOT)/etc/lpset
	chown bin $(ROOT)/etc/lpset
	chgrp sys $(ROOT)/etc/lpset
