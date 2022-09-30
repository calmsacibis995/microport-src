# makefile for televideo tape utilities
CC	= /bin/cc
LD	= /bin/ld
CFLAGSL	= -Ml
#CFLAGSL	= -Ml -g
CFLAGS	= -Ml
#CFLAGS	= 
INCRT	= ../..
INCRT1	= /usr/include
#ROOT	= /src/dist
ROOT	= ../../../dist

all:	strm

strm:	strm.c 
	$(CC) $(CFLAGSL) -I$(INCRT) -I$(INCRT1) strm.c -o strm

clean:
	rm -f strm *.o
