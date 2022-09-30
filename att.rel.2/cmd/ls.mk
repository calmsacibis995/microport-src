#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#	@(#)	1.2
ROOT =
TESTDIR = .
INS = install
#LDFLAGS = -O -s -i
LDFLAGS = -O -s

all: ls

ls: ls.c
#	$(CC) -DSINGLE $(LDFLAGS) -o $(TESTDIR)/ls ls.c -lcurses
	$(CC) $(LDFLAGS) -DNOTERMINFO -o $(TESTDIR)/ls ls.c

install: all
	$(INS) -n $(ROOT)/bin $(TESTDIR)/ls

clean:
	-rm -f ls.o

clobber: clean
	-rm -f $(TESTDIR)/ls
