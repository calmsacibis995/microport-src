#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#	@(#)	1.3
#
ROOT =

AR =
LORDER =

CFLAGS = -O

LIB = $(ROOT)/lib
LIBP = $(ROOT)/lib/libp
LIBPW = libPW.a

PRODUCTS = $(LIBPW)

FILES = abspath.o anystr.o bal.o curdir.o fdfopen.o giveup.o imatch.o \
	index.o lockit.o logname.o move.o patoi.o patol.o regcmp.o \
	regex.o rename.o repeat.o repl.o satoi.o setsig.o sname.o \
	strend.o substr.o trnslat.o userdir.o username.o verify.o \
	any.o xalloc.o xcreat.o xlink.o xopen.o xpipe.o xunlink.o \
	xwrite.o xmsg.o alloca.o cat.o dname.o fatal.o clean.o userexit.o \
	zero.o zeropad.o

all: $(LIBPW)
	@echo "Library $(PRODUCTS) is up to date"

$(LIBPW): move.o alloca.o
	$(CC) -M$(MSIZE) -c $(CFLAGS) *.c
#	$(AR) crs $(LIBPW) `$(LORDER) *.o | tsort`
	for f in $(FILES); \
		do \
		$(AR) cr $(LIBPW) $$f; \
	done;

move.o:	
	if vax; then	\
		$(CC) -c vax/move.s	; \
	elif u3b; then 	\
		$(CC) -c $(CFLAGS) u3b/move.c	; \
	elif u3b5; then	\
		$(CC) -c $(CFLAGS) u3b5/move.c	; \
	elif pdp11; then	\
		$(CC) -c $(CFLAGS) pdp11/move.c	; \
	elif iAPX286; then	\
		$(CC) -M$(MSIZE) -c $(CFLAGS) iAPX286/move.s	; \
	else	\
		$(CC) -c $(CFLAGS) u370/move.c	; \
	fi

alloca.o:
	if vax; then	\
		$(CC) -c vax/alloca.s	; \
	elif u3b; then 	\
		$(CC) -c u3b/alloca.s	; \
	elif u3b5; then	\
		$(CC)  -c u3b5/alloca.s	; \
	elif pdp11; then	\
		$(CC) -c pdp11/alloca.s	; \
	elif iAPX286; then	\
		$(CC) -M$(MSIZE) -c iAPX286/alloca.s	; \
	else	\
		$(CC) -c $(CFLAGS) u370/alloca.c	; \
	fi

install:	$(LIBPW)
	cp $(LIBPW) $(LIB)/$(MODEL)

clean:
	-rm -f *.o

clobber:	clean
	-rm -f $(PRODUCTS)

.PRECIOUS:	$(PRODUCTS)
