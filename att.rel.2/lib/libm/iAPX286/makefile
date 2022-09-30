#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#	@(#)	1.2
CFILES = \
	atan.o\
	exp.o\
	log.o\
	sin.o\
	sqrt.o\
	tan.o

SFILES = \
	xatan.o\
	xexp.o\
	xlog.o\
	xsqrt.o\
	xtrans.o

all: prep $(SFILES) $(CFILES)

prep:
	rm -rf proflib
	mkdir proflib

$(SFILES):
	$(CC) -M$(MSIZE) -c -O $?  && cp $*.o proflib

$(CFILES):
	$(CC) -M$(MSIZE) -c -O -p $?  && mv $*.o proflib
	$(CC) -M$(MSIZE) -c -O  $?

clean:
	rm -f *.o

clobber:	clean
