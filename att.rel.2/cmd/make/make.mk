#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#	@(#)	1.3

# The rules.c file can be modified locally for people who still like
#	things like fortran.

O	= -O
TESTDIR = .
YACCRM = rm -f
LDFLAGS = -n -s
INS = install -o
INSDIR = $(ROOT)/bin
LIBS = -lld

OBJECTS =  \
	main.o \
	doname.o \
	misc.o \
	files.o \
	rules.o \
	dosys.o \
	gram.o \
	dyndep.o \
	prtmem.o

CFLAGS = $O -Ml -Dunix=1

all:  make
	@echo MAKE is up to date.

make:  $(OBJECTS)
	if pdp11 ; then \
		$(CC) -o $(TESTDIR)/make $(LDFLAGS) $(OBJECTS) ; \
	elif iAPX286 ; then  \
		$(CC) -Ml -o $(TESTDIR)/make $(LDFLAGS) $(OBJECTS) $(LIBS) ; \
	else \
		$(CC) -o $(TESTDIR)/make $(LDFLAGS) $(OBJECTS) $(LIBS) ; \
	fi

gram.c:	gram.y

gram.o: gram.c

$(OBJECTS):  defs

install: all
	$(INS) -n $(INSDIR) $(TESTDIR)/make

clean:
	-rm -f *.o
	$(YACCRM) gram.c

clobber: clean
	-rm -f $(TESTDIR)/make

#.DEFAULT:
#	$(GET) $(GFLAGS) -p s.$< > $<
