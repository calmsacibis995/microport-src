#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#	@(#)	1.2	#
ROOT = 
TESTDIR = .
INS = install
INSDIR = $(ROOT)/etc
FRC =
CFLAGS = -O
LDFLAGS= -s
all: config
config: 
	if iAPX286; then $(CC) $(CFLAGS) $(LDFLAGS) -o $(TESTDIR)/config ./iAPX286/config.c; fi
	if vax; then $(CC) $(CFLAGS) $(LDFLAGS) -o $(TESTDIR)/config ./vax/config.c; fi
	if pdp11; then $(CC) $(CFLAGS) $(LDFLAGS) -o $(TESTDIR)/config ./pdp11/config.c; fi
	if u3b; then $(CC) $(CFLAGS) $(LDFLAGS) -o $(TESTDIR)/config ./u3b20/config.c; fi

install: all
	$(INS) -f $(INSDIR) $(TESTDIR)/config
clean:
	rm -f *.o
clobber: clean
	rm -f $(TESTDIR)/config

FRC:
