#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#	@(#)	1.2
ROOT =
CFLAGS = $(FFLAG) -O
LDFLAGS = -s
INS = :

all: units unittab

units:	units.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o units units.c
	$(INS) -n $(ROOT)/usr/bin units 
	$(INS) -n $(ROOT)/usr/lib unittab 

install:
	$(MAKE) -f units.mk all INS=/etc/install

clean:
	rm -f units.o

clobber: clean
	rm -f units
