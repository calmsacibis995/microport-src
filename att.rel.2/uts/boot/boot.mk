#	Copyright (c) 1985 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#   @(#)boot.mk	1.7 - 85/08/31

AS=$(PFX)as
CC=$(PFX)cc
LD=$(PFX)ld

CFLAGS= -O
AFLAGS=
LFLAGS=

OBJ=	bboot.o bload.o butil1.o
SRC=	bboot.s bload.c butil1.c
INS=    install
INSDIR= $(ROOT)/etc

all: boot

boot: $(OBJ)
	$(LD) $(LFLAGS) -R -s  -o boot $(OBJ) ifile

bboot.o: bboot.s
	$(CC)  -c  $(CFLAGS) bboot.s

butil1.o: butil1.c
	$(CC)  -c  $(CFLAGS) butil1.c

bload.o: bload.c
	$(CC)  -c  $(CFLAGS) bload.c

clean:
	rm -f $(OBJ)

clobber: clean
	rm -f boot

install: all
	$(INS) -n $(INSDIR) boot
