#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#	@(#)	1.3
ROOT=
ULIBDIR=$(ROOT)/usr/lib
LORDER=lorder

CC=
CFLAGS=

LIBOBJS=libmai.o libzer.o

all:     $(ULIBDIR)/$(MODEL)/liby.a

$(ULIBDIR)/$(MODEL)/liby.a: $(LIBOBJS)
	$(AR) rv tmplib.a `$(LORDER) *.o | tsort`;

libmai.o:	libmai.c
		$(CC) -M$(MSIZE) -c $(CFLAGS) libmai.c

libzer.o:	libzer.c
		$(CC) -M$(MSIZE) -c $(CFLAGS) libzer.c

install:  all
	cp tmplib.a $(ULIBDIR)/$(MODEL)/liby.a;

clean:
	-rm -f *.o

clobber:	clean
	-rm -f tmplib.a 
