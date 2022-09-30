#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#	@(#)	1.2	#
ROOT =
TESTDIR = .
FRC =
INS = install
INSDIR =
CFLAGS = -O
FFLAG =
OFILES = atof.o bs.o string.o
#IFLAG = -i

all: bs

bs:	$(OFILES)
	$(CC) $(LDFLAGS) $(FFLAG) -s $(IFLAG) -o $(TESTDIR)/bs $(OFILES) -lm 

atof.o:	atof.c $(FRC)
bs.o:	bs.c $(FRC)
string.o: string.c $(FRC)

test:
	bs testall

install: all
	$(INS) -n $(ROOT)/bin $(TESTDIR)/bs $(INSDIR)

clean:
	-rm -f *.o

clobber: clean
	-rm -f $(TESTDIR)/bs

FRC:
