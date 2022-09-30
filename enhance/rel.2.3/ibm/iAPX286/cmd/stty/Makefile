# temporary makefile for stty. 5/2/86
#
SRC	= stty.c
OBJ	= stty.o
CC	= /bin/cc
LD	= /bin/ld
CFLAGS	= -O -s
INCRT	= ../..
INCRT1	= /usr/include

all:	stty

stty : $(SRC)
	$(CC) $(CFLAGS) -o stty stty.c

clean:
	-rm -f stty *.o
	
clobber:

install:

