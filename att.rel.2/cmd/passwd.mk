#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#	@(#)	1.2
#	passwd make file
ROOT=
INSDIR = $(ROOT)/bin
LDFLAGS = -O -s
INS=

passwd:
	$(CC) $(LDFLAGS) -o passwd passwd.c
	$(INS)

all:	install clobber

install:
	$(MAKE) -f passwd.mk INS="install -f $(INSDIR)  passwd"

clean:
	-rm -f *.o

clobber: clean
	rm -f passwd
