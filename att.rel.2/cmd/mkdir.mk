#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#	@(#)	1.2
#	mkdir make file
ROOT=
INSDIR = $(ROOT)/bin
LDFLAGS = -O -s
INS=:

mkdir:
	$(CC) $(LDFLAGS) -o mkdir mkdir.c
	$(INS) $(INSDIR)  mkdir

all:	install clobber

install:
	$(MAKE) -f mkdir.mk INS="install -f"

clean:
	-rm -f *.o

clobber: clean
	rm -f mkdir
