#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#	@(#)	1.1
#	df make file
ROOT=
INCRT = $(ROOT)/usr/include
INSDIR = $(ROOT)/bin
CFLAGS = -O -s -Ml
INS=:

df:
	$(CC) -I$(INCRT) $(CFLAGS) -o df df.c
	$(INS) $(INSDIR) df

all:	install clobber

install:
	$(MAKE) -f df.mk INS="install -f"

clean:
	-rm -f *.o

clobber: clean
	rm -f df
