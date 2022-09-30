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

MPWLIB = ../mpwlib.a

PRODUCTS = $(MPWLIB)

FILES = fmalloc.o		\
		xcreat.o	\
		xmsg.o

all: $(PRODUCTS)
	@echo "Library $(PRODUCTS) is up to date\n"

$(MPWLIB): $(FILES)
	$(AR) cr $(MPWLIB) `$(LORDER) *.o | tsort`
	$(CH) chmod 664 $(MPWLIB)

xcreat: ../../hdr/defines.h
xmsg:	../../hdr/defines.h

install: $(MPWLIB)

clean:
	-rm -f *.o

clobber: clean
	-rm -f $(PRODUCTS)

.PRECIOUS:	$(PRODUCTS)
