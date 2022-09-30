#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#	@(#)	1.1
#	du make file
ROOT=
INSDIR = $(ROOT)/bin
LDFLAGS = -Ml -O -s
INS=:

du:
	$(CC) $(LDFLAGS) -o du du.c
	$(INS) $(INSDIR) du

all:	install clobber

install:
	$(MAKE) -f du.mk INS="install -f"

clean:
	-rm -f *.o

clobber: clean
	rm -f du
