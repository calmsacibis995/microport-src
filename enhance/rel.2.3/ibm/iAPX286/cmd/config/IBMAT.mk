#       Copyright (c) 1984 AT&T
#         All Rights Reserved

#       THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#       The copyright notice above does not evidence any
#       actual or intended publication of such source code.

#       @(#)    1.2     #
ROOT = 
TESTDIR = ../../cf
INS = install
INSDIR = $(ROOT)/etc
FRC =
# CFLAGS = -O
LDFLAGS= -s
all: config
config: config.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o config config.c

runit:
	cd $(TESTDIR) ;../cmd/config/config -m master dfile.wini
#	cd $(TESTDIR) ;diff conf.c conf.c.00 > diff.conf
#	cd $(TESTDIR) ;diff config.h config.h.00 > diff.config
#	cd $(TESTDIR) ;diff handlers.c handlers.c.00 > diff.handlers

install: all

clean:
	rm -f *.o
clobber: clean
	rm -f config

FRC:
