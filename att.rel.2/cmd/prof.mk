#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#	@(#)	1.4
#
CFLAGS	= -O
ROOT	=
VERSION	=
UBINDIR	= $(ROOT)/usr/bin
USRINC	= $(ROOT)/usr/$(VERSION)/include
INS=install

all:	prof.o
	if pdp11; then \
		$(CC) $(CFLAGS) prof.o -o prof; \
	elif iAPX286; then \
		$(CC) $(CFLAGS) prof.o -Ml -lld -s -o prof; \
	else \
		$(CC) $(CFLAGS) prof.o -lld -o prof; \
	fi

prof.o:	prof.c
	if iAPX286; then \
		$(CC) $(CFLAGS) -c -Ml prof.c;  \
	else \
		$(CC) $(CFLAGS) -c prof.c; \
	fi

prof:	all $(USRINC)/stdio.h $(USRINC)/a.out.h $(USRINC)/sys/types.h \
	$(USRINC)/sys/param.h $(USRINC)/mon.h

install:	prof
	rm -f $(UBINDIR)/prof
	cp ./prof $(UBINDIR)/prof
	chmod 775 $(UBINDIR)/prof
	chgrp bin $(UBINDIR)/prof
	chown bin $(UBINDIR)/prof

clean:
	rm -f prof.o

clobber: clean
	rm -f prof
