#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#	@(#)	1.1

LIB = $(ROOT)/lib

nonprof: all

all:	crypt.c
	$(CC) -O -M$(MSIZE) -c crypt.c

clean:
	rm -f crypt.o

clobber: 	clean

