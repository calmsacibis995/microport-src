#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#	@(#)	1.4
ROOT =
TESTDIR = .
INS = /etc/install
LIBCURSES=-lcurses
LDFLAGS = -O -s

all: pg

pg: pg.c
	$(CC) -DSINGLE $(LDFLAGS) -o $(TESTDIR)/pg pg.c $(LIBCURSES)

install: all
	$(INS) -n $(ROOT)/usr/bin $(TESTDIR)/pg

clean:
	-rm -f pg.o

clobber: clean
	-rm -f $(TESTDIR)/pg
