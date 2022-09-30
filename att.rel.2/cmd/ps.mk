#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#	@(#)	1.3
#	ps make file
ROOT=
INSDIR = $(ROOT)/bin
LDFLAGS = -Ml -O -s
INS=:

ps:
	$(CC) $(LDFLAGS) -o ps ps.c
	$(INS) $(INSDIR) ps

all:	install clobber

install:
	$(MAKE) -f ps.mk INS="install -f"

clean:
	-rm -f *.o

clobber: clean
	rm -f ps
