#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#	@(#)	1.2
#	su make file
ROOT=
INSDIR = $(ROOT)/bin
LDFLAGS = -O -s
INS=:

su:
	$(CC) $(LDFLAGS) -o su su.c
	$(INS) $(INSDIR) su

all:	install clobber

install:
	$(MAKE) -f su.mk INS="install -f"

clean:
	-rm -f *.o

clobber: clean
	rm -f su
