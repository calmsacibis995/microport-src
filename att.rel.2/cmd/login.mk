#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#	@(#)	1.1
ROOT =
TESTDIR = .
FRC =
INS = install
INSDIR = $(ROOT)/bin 
LDFLAGS = -O -s
CONS=-DCONSOLE='"/dev/console"'

all: login

login: login.c $(FRC)
	$(CC) $(CONS) $(LDFLAGS) -o $(TESTDIR)/login login.c  

test:
	rtest $(TESTDIR)/login

install: all
	$(INS) -o -n $(INSDIR) $(TESTDIR)/login

clean:
	-rm -f *.o

clobber: clean
	-rm -f $(TESTDIR)/login

FRC:
