#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#	@(#)	1.1
#	fuser make file
ROOT=
INSDIR = $(ROOT)/etc
LDFLAGS = -Ml -O -s
INS=:

fuser:
	$(CC) $(LDFLAGS) -o fuser fuser.c
	$(INS) $(INSDIR) fuser

all:	install clobber

install:
	$(MAKE) -f fuser.mk INS="install -f"

clean:
	-rm -f *.o

clobber: clean
	rm -f fuser
