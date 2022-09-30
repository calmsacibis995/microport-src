# makefile for microport's fdisk program.  1/22/86 uport!dwight
#
#	make, make all, make fdisk:	This is what you want to type.
#
BOOTDRVTBL = -DBOOTDRVTBL 
SRC	= fdisk.h fdisk.c insdisk.c
OBJ	= fdisk.o insdisk.o
CC	= /bin/cc
LD	= /bin/ld
# CFLAGS	= -Ml -s
CFLAGS	= -Ml
INCRT	= ../..
INCRT1	= /usr/include
ROOT	= ../../../dist

all:	fdisk showbad

fdisk : $(OBJ)
	$(CC) $(CFLAGS)  -I$(INCRT) -I$(INCRT1) $(OBJ) -o fdisk

showbad : showbad.c
	$(CC) $(CFLAGS) -s  -I$(INCRT) -I$(INCRT1)   showbad.c -o showbad

fdisk.o:
	$(CC) $(CFLAGS) $(BOOTDRVTBL) -I$(INCRT) -I$(INCRT1) -c fdisk.c 

insdisk.o:
	$(CC) -c $(CFLAGS) $(BOOTDRVTBL) -I$(INCRT) -I$(INCRT1) insdisk.c 

clean:
	-rm -f fdisk showbad *.o

clobber:
	-rm -f $(ROOT)/etc/fdisk
	-rm -f $(ROOT)/etc/showbad

install:	all clobber
	cp fdisk showbad $(ROOT)/etc
	chmod 510 $(ROOT)/etc/fdisk 
	chmod 511 $(ROOT)/etc/showbad 
	chown bin $(ROOT)/etc/fdisk
	chown bin $(ROOT)/etc/showbad
	chgrp sys $(ROOT)/etc/fdisk
	chgrp sys $(ROOT)/etc/showbad
