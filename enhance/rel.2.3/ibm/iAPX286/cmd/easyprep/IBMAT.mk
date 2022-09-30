# makefile for microport's fdisk program.  1/22/86 uport!dwight
#
#	make, make all, make fdisk:	This is what you want to type.
#
SRC	= easyprep.c 
OBJ	=
CC	= /bin/cc
LD	= /bin/ld
CFLAGS	= -s -O $(DEBUG)
INCRT	= ../..
INCRT1	= /usr/include
ROOT	= ../../../dist


all:	easyprep

easyprep : $(INCRT)/sys/misc.h $(INCRT)/sys/cmos.h $(INCRT)/sys/wn.h \
	$(INCRT)/sys/wndefaults.h $(INCRT)/sys/fdisk.h ../getype.c easyprep.c
	$(CC) $(CFLAGS)  -I$(INCRT) -I$(INCRT1) $(SRC) -o easyprep

clean:
	-rm -f easyprep *.o

clobber:
	-rm -f $(ROOT)/etc/easyprep

install:	all clobber
	cp easyprep $(ROOT)/etc
	chmod 500 $(ROOT)/etc/easyprep 
	chown root $(ROOT)/etc/easyprep
	chgrp sys $(ROOT)/etc/easyprep
