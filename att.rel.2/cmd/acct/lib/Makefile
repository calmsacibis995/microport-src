#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#	@(#)	1.1
ROOT = 
TESTDIR = .
LIB = $(ROOT)/lib/a.a
CFLAGS = -O
INCDIR = ..
FRC =

FILES= \
	$(LIB)(pnpsplit.o) \
	$(LIB)(copyn.o) \
	$(LIB)(devtolin.o) \
	$(LIB)(expand.o) \
	$(LIB)(lintodev.o) \
	$(LIB)(namtouid.o) \
	$(LIB)(tmless.o) \
	$(LIB)(tmsecs.o) \
	$(LIB)(uidtonam.o) \
	$(LIB)(substr.o) \
	$(LIB)(regex.o) \
	$(LIB)(regcmp.o)

all:	$(LIB) $(FRC)

$(LIB):	$(FILES)
	@echo lib $(LIB) is now up to date

clean:
	-rm -f *.o

clobber:	clean
	-rm -f $(LIB)

FRC:

.c.a:
	$(CC) $(CFLAGS) -I$(INCDIR) -c $<
	$(AR) rv $@ $*.o
	rm -f $*.o

