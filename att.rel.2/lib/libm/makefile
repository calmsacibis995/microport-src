#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#	@(#)	1.5
FILES = \
	asin.o\
	erf.o\
	fabs.o\
	floor.o\
	fmod.o\
	gamma.o\
	hypot.o\
	j0.o\
	j1.o\
	jn.o\
	matherr.o\
	pow.o\
	sinh.o\
	tanh.o

all:	prep 	$(FILES)

prep:
	rm -rf proflib
	mkdir proflib

$(FILES):
	$(CC) -c -O -p -M$(MSIZE) $*.c && mv $*.o proflib
	$(CC) -c -O -M$(MSIZE) $*.c

clean:
	rm -f *.o
	rm -rf proflib

clobber:	clean
	rm -f libm.a
