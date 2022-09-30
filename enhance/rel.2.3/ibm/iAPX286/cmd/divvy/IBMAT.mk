# makefile for microport's divvy program.  1/22/86 uport!dwight
#
#	make, make all, make divvy:	This is what you want to type.
#
SRC	= divvy.c
OBJ	= divvy.o initPER.o
CC	= /bin/cc
LD	= /bin/ld
CFLAGS	=  -Ml -s
INCRT	= ../..
INCRT1	= /usr/include
HFILES = ../../sys/divvy.h
ROOT	= ../../../dist


all:	divvy

divvy	: $(OBJ)
	$(CC) $(CFLAGS) -I$(INCRT) -I$(INCRT1) $(OBJ) -o divvy

divvy.o : divvy.c $(HFILES)
	$(CC) $(CFLAGS)  -I$(INCRT) -I$(INCRT1) -c divvy.c

initPER.o : initPER.c $(HFILES)
	$(CC) $(CFLAGS)  -I$(INCRT) -I$(INCRT1) -c initPER.c

clean:
	-rm -f divvy *.o

clobber:
	-rm -f $(ROOT)/etc/divvy

install:	all clobber
	cp divvy $(ROOT)/etc/divvy
	chmod 510 $(ROOT)/etc/divvy
	chown bin $(ROOT)/etc/divvy
	chgrp sys $(ROOT)/etc/divvy
