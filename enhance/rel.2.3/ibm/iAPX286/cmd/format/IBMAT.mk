
BOOTDRVTBL = -DBOOTDRVTBL
SRC	= format.h format.c init.c
OBJ	= format.o init.o
CC	= /bin/cc
LD	= /bin/ld
CFLAGS	= -Ml -s $(BOOTDRVTBL)
INCRT	= ../..
INCRT1	= /usr/include
ROOT	= ../../../dist

all:	format

format : $(OBJ)
	$(CC) $(CFLAGS)  -I$(INCRT) -I$(INCRT1) $(OBJ) -o format


format.o:
	$(CC) $(CFLAGS) -I$(INCRT) -I$(INCRT1) -c format.c 

init.o:
	$(CC) -c $(CFLAGS) -I$(INCRT) -I$(INCRT1) init.c 

clean:
	-rm -f format *.o

clobber:
	-rm -f $(ROOT)/etc/format

install:	all clobber
	cp format $(ROOT)/etc/format
	chmod 510 $(ROOT)/etc/format
	chown bin $(ROOT)/etc/format
	chgrp sys $(ROOT)/etc/format
