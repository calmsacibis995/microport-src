h27431
s 00004/00003/00059
d D 1.2 87/08/12 04:02:45 root 2 1
c change -O to $O
e
s 00062/00000/00000
d D 1.1 87/08/12 00:54:33 root 1 0
c date and time created 87/08/12 00:54:33 by root
e
u
U
t
T
I 1
#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#	@(#)	1.3

# The rules.c file can be modified locally for people who still like
#	things like fortran.

I 2
O	= -O
E 2
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

D 2
CFLAGS = -O -Ml -Dunix=1
E 2
I 2
CFLAGS = $O -Ml -Dunix=1
E 2

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

D 2
.DEFAULT:
	$(GET) $(GFLAGS) -p s.$< > $<
E 2
I 2
#.DEFAULT:
#	$(GET) $(GFLAGS) -p s.$< > $<
E 2
E 1
