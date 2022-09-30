#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#	@(#)	1.1
ROOT=
TESTDIR = .
FRC =
INS = :
INSDIR = $(ROOT)/usr/bin 
LIBCURSES= -lcurses
LDFLAGS = -O -s

all:	tput

tput:	tput.c $(FRC)
	$(CC) $(LDFLAGS) -o $(TESTDIR)/tput tput.c $(LIBCURSES)
	$(INS) -f $(INSDIR) $(TESTDIR)/tput

install:
	$(MAKE) -f tput.mk INS="install -f $(INSDIR)"

clean:
	-rm -f *.o

clobber:	clean
	-rm -f $(TESTDIR)/tput

FRC:

