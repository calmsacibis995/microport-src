#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#	@(#)	1.2
TESTDIR = .
FRC =
INS = install
INSDIR = $(ROOT)/etc 
CFLAGS= -O -DSYS_NAME
LDFLAGS = -s -n
OBJECTS= getty.o

all: getty

getty: getty.c $(FRC)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(TESTDIR)/getty getty.c  

test:
	rtest $(TESTDIR)/getty

install: all
	$(INS) -f $(INSDIR) -o $(TESTDIR)/getty $(INSDIR)

clean:
	-rm -f $(OBJECTS)

clobber: clean
	-rm -f $(TESTDIR)/getty

FRC:
