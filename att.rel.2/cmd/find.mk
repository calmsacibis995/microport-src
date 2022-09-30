#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#	@(#)	1.2
ROOT =
TESTDIR = .
INS = /etc/install
LDFLAGS = -O -s -Wl,-k 32766

all: find

find: find.c
	$(CC) $(LDFLAGS) -o $(TESTDIR)/find find.c

install: all
	$(INS) -n $(ROOT)/bin $(TESTDIR)/find

clean:
	-rm -f find.o

clobber: clean
	-rm -f $(TESTDIR)/find
