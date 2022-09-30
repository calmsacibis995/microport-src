#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#	@(#)	1.2
#
ROOT =

AR = ar
LORDER = lorder

CFLAGS = -O

CASSILIB = ../cassi.a

PRODUCTS = $(CASSILIB)

FILES = gf.o		\
		cmrcheck.o	\
		deltack.o	\
		error.o		\
		filehand.o

all: $(PRODUCTS)
	@echo "Library $(PRODUCTS) is up to date\n"

$(CASSILIB): $(FILES)
	$(AR) cr $(CASSILIB) `$(LORDER) *.o | tsort`
	$(CH) chmod 664 $(CASSILIB)

gf.o:	gf.c	\
	 ../../hdr/filehand.h

cmrcheck.o:	cmrcheck.c	\
	 ../../hdr/filehand.h

deltack.o:	deltack.c	\
	 ../../hdr/filehand.h	\
	 ../../hdr/had.h	\
	 ../../hdr/defines.h

filehand.o:	filehand.c ../../hdr/filehand.h

install:	$(PRODUCTS)

clean:
	-rm -f *.o

clobber:	clean
	-rm -f $(PRODUCTS)

