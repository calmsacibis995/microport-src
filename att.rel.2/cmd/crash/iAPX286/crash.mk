#	Copyright (c) 1985 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#/*   @(#)crash.mk	1.4 - 85/08/12 */
TESTDIR = .
FRC =
INS = /etc/install
INSDIR = $(ROOT)/etc
CFLAGS = -O -Ml
LDFLAGS =  -s
CC = cc
SYS =

OFILES = buf.o callout.o file.o inode.o lck.o main.o $(SYS)misc.o mount.o \
	 proc.o $(SYS)symtab.o stat.o text.o tty.o u.o sysvad.o map286.o

all:	stest
stest:
	make -f crash.mk comp SYS=iAPX CC="$(CC)" CFLAGS="-Ml -O"

comp:	$(OFILES) cmd.h crash.h 
	$(CC) $(CFLAGS) $(FFLAG) $(LDFLAGS) -o $(TESTDIR)/crash $(OFILES) -lld

install: stest
	$(INS) -n $(INSDIR) $(TESTDIR)/crash 

clean:
	-rm -f *.o

clobber: clean
	-rm -f $(TESTDIR)/crash

FRC:
