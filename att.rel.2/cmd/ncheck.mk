#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#	@(#)	1.1
ROOT =
TESTDIR = .
FRC =
INS = install
IFLAG = -i
LDFLAGS = -s -n
INSDIR = -n $(ROOT)/usr/bin
CFLAGS = -O
FFLAG =
SMFLAG=

all: ncheck

ncheck: ncheck.c 
	if iAPX286 ; then \
	$(CC) -DFsTYPE=3 $(LDFLAGS) $(CFLAGS) -Ml -o $(TESTDIR)/ncheck ncheck.c ; \
	elif [ x$(IFLAG) != x-i ]  ; then \
	$(CC) $(LDFLAGS) -Dsmall=-1 $(CFLAGS)  $(IFLAG) -o $(TESTDIR)/ncheck ncheck.c ; \
	else $(CC) $(LDFLAGS) $(CFLAGS) $(IFLAG) -o $(TESTDIR)/ncheck ncheck.c ; \
	fi


test:
	rtest $(TESTDIR)/ncheck

install: all
	$(INS) $(INSDIR) $(TESTDIR)/ncheck $(ROOT)/etc

clean:

clobber: clean
	-rm -f $(TESTDIR)/ncheck

FRC:
