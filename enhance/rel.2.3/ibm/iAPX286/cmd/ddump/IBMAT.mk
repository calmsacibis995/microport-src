#
#	make, make all, make ddump:	This is what you want to type.
#
SRC	= ddump.c
OBJ	= ddump.o 
CC	= /bin/cc
LD	= /bin/ld
CFLAGS	=  -Ml
INCRT	= ../..
INCRT1	= /usr/include
HFILES = 
ROOT	= ../../../dist

all:	ddump

ddump	: ddump.c
	$(CC) $(CFLAGS)  -s -I$(INCRT) -I$(INCRT1) $(SRC) -o ddump

clean:	
	-rm -f ddump *.o

clobber:
	-rm -f $(ROOT)/etc/ddump

install:	all clobber
	cp ddump $(ROOT)/etc/ddump
	chmod 510 $(ROOT)/etc/ddump
	chown bin $(ROOT)/etc/ddump
	chgrp sys $(ROOT)/etc/ddump
