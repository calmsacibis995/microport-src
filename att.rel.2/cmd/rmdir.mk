#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#	@(#)	1.2
ROOT=
INSDIR = $(ROOT)/bin
LDFLAGS = -O -s
INS=:

rmdir:
	$(CC) $(LDFLAGS) -o rmdir rmdir.c
	$(INS) $(INSDIR) rmdir

all:	install clobber

install:
	$(MAKE) -f rmdir.mk INS="install -f"

clean:
	-rm -f *.o

clobber: clean
	rm -f rmdir
