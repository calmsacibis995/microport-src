#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#	@(#)	1.4
ROOT=
INS=$(ROOT)/etc/install
MODEL=
MSIZE=
LIB = $(ROOT)/usr/lib
LIBP = $(ROOT)/usr/lib/libp
CC=$(PFX)cc
CFLAGS=

all:	libg.a

libg.a: dbxxx.s
	$(CC) $(CFLAGS) -M$(MSIZE) -c dbxxx.s
	mv dbxxx.o libg.a

install:
	#
	# move the library to the correct directory
	cp libg.a $(LIB)/$(MODEL)/libg.a;rm -f libg.a

clean:
	rm -f *.o

clobber: clean
	rm -f libg.a
