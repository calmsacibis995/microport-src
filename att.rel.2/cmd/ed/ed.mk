#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#	@(#)	1.2
#	ed make file

ROOT =
OL = $(ROOT)/
INSDIR = $(OL)bin
IFLAG = -n
B10 = 
CRYPT=
CFLAGS = -O $(CRYPT) $(B10)
LDFLAGS = -s $(IFLAG)

compile all: ed
	:

ed:	ed.c
	$(CC) -S $(CFLAGS) ed.c
	-if pdp11; then sh ./edfun ed.s; else true; fi
	$(CC) $(LDFLAGS) -o ed ed.s

ed_x:	ed.c
	if [ -s ./ed ] ; then mv ed ed_i; fi
	-rm -f ed.s ed.o
	$(MAKE) -f ed.mk ed CRYPT=-DCRYPT
	mv ed ed_x; if [ -f ed_i ] ; then mv ed_i ed; touch ed; fi

install: ed ed_x
	cpset ed $(INSDIR)/ed 
	cpset ed_x $(INSDIR)/ed_x 
	rm -f $(INSDIR)/red
	ln $(INSDIR)/ed $(INSDIR)/red
	ln $(INSDIR)/ed_x $(INSDIR)/red_x

clean:
	  rm -f ed.s ed.o

clobber:  clean
	  rm -f ed ed_x ed_i
