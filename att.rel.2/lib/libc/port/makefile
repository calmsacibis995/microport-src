#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#	@(#)	1.3
#
# makefile for libc/port
#
#
# The macro PROF is null by default, causing profiled object to be maintained.
# If profiled object is not desired, the reassignment PROF=@# should appear in
# the make command line.
#

.SUFFIXES: .p
CC=cc
CFLAGS=-O
PROF=

all:
	cd gen; make -e $(IGN) nonprof
	cd print; make -e $(IGN) nonprof
	cd stdio; make -e $(IGN) nonprof
	cd sys; make -e $(IGN) nonprof
	cd Crypt; make -e $(IGN) nonprof

clean:
	cd gen; make clean
	cd print; make clean
	cd stdio; make clean
	cd sys; make clean
	cd Crypt; make clean

clobber: clean
	-rm -f */*.o
	-rm -f */*.p
