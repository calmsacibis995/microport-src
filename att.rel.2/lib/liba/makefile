#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#	@(#)	1.3
#
# makefile for liba
#
#

VARIANT=
CFLAGS=
PCFLAGS=
PFX=
CC=$(PFX)cc
AR=$(PFX)ar
ROOT=
MODEL=
LIB = $(ROOT)/lib
LIBP = $(ROOT)/lib/libp
PROF=
DONE=

all:
	make -e -f makefile specific MACHINE=iAPX286 \
		MODEL=$(MODEL) CFLAGS="$(CFLAGS) -M$(MSIZE)" ;

specific:
	$(CC) $(CFLAGS) -c lmul.s
	$(CC) $(CFLAGS) -c ldivmod.s
	$(AR) rv liba.a lmul.o ldivmod.o

move:
	cp liba.a $(LIB)/$(MODEL)/liba.a
	cp liba.a $(LIBP)/$(MODEL)/liba.a

install: all move

clean:
	rm -f ldivmod.o lmul.o

clobber: clean
	rm -f liba.a
