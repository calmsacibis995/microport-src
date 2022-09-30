#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#	@(#)	1.2
ROOT=
INC=$(ROOT)/usr/include
TESTDIR = .
UBIN=$(ROOT)/usr/bin
INS = install
CFLAGS = -O -I$(INC) -DASCII -Dunix
#IFLAG = -i
FRC =
INSDIR =
FILES = main.o sub1.o sub2.o header.o

all: itest lex

itest:
#	This test (and flag) only good for pdp11 - not good for s5
#	@if [ x$(IFLAG) != x-i ] ; then echo NO ID SPACE ; exit 1 ; else exit 0 ; fi

lex:	$(FILES) y.tab.o
	$(CC) $(LDFLAGS) $(IFLAG) -s $(FILES) y.tab.o -ly -o $(TESTDIR)/lex

$(FILES): ldefs.c $(FRC)
main.o:	  once.c $(FRC)
y.tab.c:  parser.y $(FRC)
	  $(YACC) parser.y

test:
	  rtest $(TESTDIR)/lex

install: all
	 $(INS) -n $(UBIN) $(TESTDIR)/lex $(INSDIR)

clean:
	 -rm -f *.o y.tab.c

clobber: clean
	 -rm -f $(TESTDIR)/lex

lint:	main.c sub1.c sub2.c header.c y.tab.c once.c ldefs.c
	lint -p main.c sub1.c sub2.c header.c y.tab.c once.c ldefs.c

FRC:
