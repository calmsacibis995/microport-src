#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#	@(#)	1.1
# how to use this makefile
# to make sure all files  are up to date: make -f sa.mk all
#
# to force recompilation of all files: make -f sa.mk all FRC=FRC 
#
# to test new executables before installing in 
# /usr/lib/sa:	make -f sa.mk testbi
#
# to install just one file:	make -f sa.mk safile "INS=/etc/install"
#
# The sadc and sadp modules must be able to read /dev/kmem,
# which standardly has restricted read permission.
# They must have set-group-ID mode
# and have the same group as /dev/kmem.
# The chmod and chgrp commmands below ensure this.
#
ROOT =
TESTDIR = .
FRC =
INS = @:
INSDIR = $(ROOT)/usr/lib/sa
CFLAGS = -I. -Ml -O
LDFLAGS = -s
FFLAG =
ARGS = all
CC = cc
 

all:	sadc sar sa1 sa2 timex sag sadp

sadc:: sadc.c sa.h 
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(TESTDIR)/sadc sadc.c 
sadc::
	-mkdir $(INSDIR)
	$(INS) -o -n $(INSDIR) $(TESTDIR)/sadc $(INSDIR)
sar:: sar.c sa.h
	$(CC) $(FFLAG) $(CFLAGS) $(LDFLAGS) -o $(TESTDIR)/sar sar.c
sar::
	$(INS) -n $(ROOT)/usr/bin $(TESTDIR)/sar 
sa2:: sa2.sh
	cp sa2.sh sa2
sa2::
	-mkdir $(INSDIR)
	$(INS) -n $(INSDIR) $(TESTDIR)/sa2 $(INSDIR)
sa1:: sa1.sh
	cp sa1.sh sa1
sa1::
	-mkdir $(INSDIR)
	$(INS) -n $(INSDIR) $(TESTDIR)/sa1 $(INSDIR)
 
timex::	timex.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(TESTDIR)/timex timex.c 
timex::
	$(INS) -n $(ROOT)/usr/bin $(TESTDIR)/timex
sag::	saga.o sagb.o
	$(CC) $(FFLAG) $(CFLAGS) $(LDFLAGS) -o $(TESTDIR)/sag saga.o sagb.o 
sag::
	$(INS) -n $(ROOT)/usr/bin $(TESTDIR)/sag
saga.o:	saga.c saghdr.h
	$(CC) -c $(CFLAGS) saga.c
sagb.o:	sagb.c saghdr.h
	$(CC) -c $(CFLAGS) sagb.c
sadp:: sadp.c 
	$(CC) $(FFLAG) $(CFLAGS) $(LDFLAGS) -o $(TESTDIR)/sadp sadp.c
sadp::
	$(INS) -n $(ROOT)/usr/bin $(TESTDIR)/sadp 
test:		testai

testbi:		#test for before installing
	sh  $(TESTDIR)/runtest new $(ROOT)/usr/src/cmd/sa

testai:		#test for after install
	sh $(TESTDIR)/runtest new

install:
	-make -f sa.mk $(ARGS) FFLAG=$(FFLAG) "INS=install" INSDIR=$(INSDIR)

clean:
	-rm -f *.o
 
clobber:	clean
		-rm -f sadc sar sa1 sa2 sag timex sadp

FRC:
