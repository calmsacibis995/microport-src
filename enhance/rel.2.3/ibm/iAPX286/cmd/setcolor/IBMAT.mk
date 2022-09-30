# makefile for microport's setcolor program.  5/14/86 uport!dwight
#
#	make, make all, make setcolor:	This is what you want to type.
#
SRC	= setcolor.c
OBJ	= setcolor.o
CC	= /bin/cc
LD	= /bin/ld
CFLAGS	= -s
INCRT	= ../..
INCRT1	= /usr/include
ROOT	= ../../../dist
DEST	= $(ROOT)/bin

all:	setcolor

clean:
	-rm -f setcolor *.o

clobber:
	-rm -f $(DEST)/setcolor

install:	all clobber
	cp setcolor $(DEST)/setcolor
	chmod 510 $(DEST)/setcolor
	chown bin $(DEST)/setcolor
	chgrp sys $(DEST)/setcolor
