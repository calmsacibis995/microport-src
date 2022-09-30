#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#	@(#)	1.2
ROOT=
TESTDIR = .
FRC =
INS = install
INSDIR = $(ROOT)/usr/bin 
LDFLAGS = -O -s
LIBCURSES=-lcurses

all: tabs

tabs: tabs.c $(FRC)
	$(CC) $(LDFLAGS) -o $(TESTDIR)/tabs tabs.c $(LIBCURSES)

install: all
	$(INS) -f $(INSDIR) $(TESTDIR)/tabs

clean:
	-rm -f *.o

clobber: clean
	-rm -f $(TESTDIR)/tabs

FRC:
