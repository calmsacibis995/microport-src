#	Copyright (c) 1985 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#/*   @(#)yacc.mk	1.3 - 85/08/09 */
ROOT=
INC=$(ROOT)/usr/include
UBIN=$(ROOT)/usr/bin
ULIB=$(ROOT)/usr/lib
CFLAGS=-O -I$(INC)
CC=cc
TESTDIR = .
FRC =
INS = install
INSDIR =
IFLAG = -i
OFILES = y1.o y2.o y3.o y4.o

all: itest yacc

itest:
	@if [ x$(IFLAG) != x-i ] ; then echo NO ID SPACE ; exit 1 ; else exit 0 ; fi

yacc:	$(OFILES)
	$(CC) $(LDFLAGS) $(IFLAG) -s -o $(TESTDIR)/yacc $(OFILES)

$(OFILES): dextern files $(FRC)
	   $(CC) -c $(CFLAGS) y1.c y2.c y3.c y4.c

test:
	   rtest $(TESTDIR)/yacc

install:   all
	   $(INS) -n $(UBIN) $(TESTDIR)/yacc $(INSDIR)
	   $(INS) -n $(ULIB) $(TESTDIR)/yaccpar

clean:
	   -rm -f $(OFILES)

clobber:   clean
	   -rm -f $(TESTDIR)/yacc

FRC:
