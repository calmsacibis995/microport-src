#	Copyright (c) 1985 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#/*   @(#)makefile	1.3 - 85/08/09 */
#
#
SGSBASE	= ../..
YACC	= yacc
ARCH	= AR32WR
FLEX	= -DFLEXNAMES
LIBDIR	= $(ROOT)/lib
COMPCOM	= ../mip
INCI286	= $(SGSBASE)/inc/i286
INCCOM	= $(SGSBASE)/inc/common
MSPEC	=
MODEL	=
CFLAGS  = -O $(MSPEC)
DEFLIST	= $(FLEX) -D$(ARCH)
INCLIST	= -I$(INCI286) -I$(COMPCOM) -I$(INCCOM) -I.
CC	= cc
CC_CMD	= $(CC) -c $(CFLAGS) $(INCLIST) $(DEFLIST)
STRIP	= strip
LINT	= lint
LINTF	= -hv $(INCLIST) $(DEFLIST)
OWN	= bin
GRP	= bin
MODE	= 0775

OFRONT=	code.o local.o cgram.o comm1.o messages.o optim.o \
	pftn.o scan.o trees.o xdefs.o

OBACK=	local2.o order.o seg.o table.o allo.o comm2.o match.o reader.o

OBJECTS=$(OFRONT) $(OBACK)

CFRONT=	code.c local.c $(COMPCOM)/cgram.c $(COMPCOM)/comm1.c \
	$(COMPCOM)/messages.c $(COMPCOM)/optim.c $(COMPCOM)/pftn.c \
	$(COMPCOM)/scan.c $(COMPCOM)/trees.c $(COMPCOM)/xdefs.c

CBACK=	local2.c order.c seg.c table.c $(COMPCOM)/allo.c $(COMPCOM)/comm2.c \
	$(COMPCOM)/match.c $(COMPCOM)/reader.c

HFRONT=	macdefs $(INCI286)/paths.h $(INCI286)/sgs.h $(COMPCOM)/common \
	$(COMPCOM)/manifest $(COMPCOM)/messages.h $(COMPCOM)/mfile1 \
	$(INCCOM)/storclass.h

HBACK=	macdefs mac2defs $(INCI286)/paths.h $(COMPCOM)/common \
	$(COMPCOM)/manifest $(COMPCOM)/mfile2 $(INCCOM)/storclass.h

build:  front back ;

# FRONT - PASS ONE

front: $(OFRONT)
	$(CC) $(CFLAGS) -o front $(OFRONT)

code.o:	code.c macdefs $(INCI286)/paths.h $(INCI286)/sgs.h \
	$(COMPCOM)/manifest $(COMPCOM)/mfile1 $(INCCOM)/storclass.h
	$(CC_CMD) code.c

local.o:	local.c $(COMPCOM)/manifest macdefs $(COMPCOM)/mfile1 \
		$(INCCOM)/storclass.h
		$(CC_CMD) local.c

cgram.o:	$(COMPCOM)/cgram.c macdefs $(COMPCOM)/manifest $(COMPCOM)/messages.h \
		$(COMPCOM)/mfile1
		$(CC_CMD) -DYYDEBUG=0 $(COMPCOM)/cgram.c

$(COMPCOM)/cgram.c:	$(COMPCOM)/cgram.y
		@echo "Expect to see message conflicts: 7 shift/reduce"
		$(YACC) $(COMPCOM)/cgram.y
		mv y.tab.c $(COMPCOM)/cgram.c

comm1.o:	$(COMPCOM)/comm1.c macdefs $(COMPCOM)/common \
		$(COMPCOM)/manifest $(COMPCOM)/mfile1
		$(CC_CMD) $(COMPCOM)/comm1.c

messages.o:	$(COMPCOM)/messages.c $(COMPCOM)/messages.h
		$(CC_CMD) $(COMPCOM)/messages.c
		
optim.o:	$(COMPCOM)/optim.c macdefs $(COMPCOM)/manifest \
		$(COMPCOM)/mfile1
		$(CC_CMD) $(COMPCOM)/optim.c

pftn.o:	$(COMPCOM)/pftn.c macdefs $(COMPCOM)/manifest $(COMPCOM)/messages.h \
	$(COMPCOM)/mfile1
	$(CC_CMD) $(COMPCOM)/pftn.c

scan.o:	$(COMPCOM)/scan.c macdefs $(COMPCOM)/manifest $(COMPCOM)/messages.h \
	$(COMPCOM)/mfile1
	$(CC_CMD) $(COMPCOM)/scan.c

trees.o:	$(COMPCOM)/trees.c macdefs $(COMPCOM)/manifest \
		$(COMPCOM)/messages.h $(COMPCOM)/mfile1
		$(CC_CMD) $(COMPCOM)/trees.c

xdefs.o:	$(COMPCOM)/xdefs.c macdefs $(COMPCOM)/manifest \
		$(COMPCOM)/mfile1
		$(CC_CMD) $(COMPCOM)/xdefs.c

# BACK - PASS TWO

back:	$(OBACK)
	$(CC) $(CFLAGS) -o back $(OBACK)

local2.o :	macdefs mac2defs $(COMPCOM)/manifest $(COMPCOM)/mfile2
	$(CC_CMD) local2.c

order.o:	macdefs mac2defs $(COMPCOM)/manifest $(COMPCOM)/mfile2
	$(CC_CMD) order.c

seg.o:	macdefs mac2defs $(COMPCOM)/manifest $(COMPCOM)/mfile2
	$(CC_CMD) seg.c

table.o:	macdefs mac2defs $(COMPCOM)/manifest $(COMPCOM)/mfile2
	$(CC_CMD) table.c

allo.o:	$(COMPCOM)/allo.c macdefs mac2defs $(COMPCOM)/manifest $(COMPCOM)/mfile2
	$(CC_CMD) $(COMPCOM)/allo.c

comm2.o:	$(COMPCOM)/comm2.c macdefs mac2defs $(COMPCOM)/common \
		$(COMPCOM)/manifest $(COMPCOM)/mfile2
	$(CC_CMD) $(COMPCOM)/comm2.c

match.o:	$(COMPCOM)/match.c macdefs mac2defs $(COMPCOM)/manifest \
		$(COMPCOM)/mfile2
	$(CC_CMD) $(COMPCOM)/match.c

reader.o:	$(COMPCOM)/reader.c macdefs mac2defs $(COMPCOM)/manifest \
		$(COMPCOM)/mfile2
	$(CC_CMD) $(COMPCOM)/reader.c


# TIDY UP

strip:
	-strip front back

clean:
	-rm -f $(OBJECTS)
	-rm  -f $(COMPCOM)/cgram.c 
clobber: clean
	-rm -f front back
shrink: clean clobber

# LINT

lint:	lint1 lint2

lint1:	$(HFRONT) $(CFRONT)
	$(LINT) $(LINTF) $(CFRONT)

lint2:	$(HBACK) $(CBACK)
	$(LINT) $(LINTF) $(CBACK)

# INSTALLATION

install:	front back
	$(STRIP) front
	sh $(SGSBASE)/sgs.install $(MODE) $(OWN) $(GRP) $(LIBDIR)/$(SGS)front front
	$(STRIP) back
	sh $(SGSBASE)/sgs.install $(MODE) $(OWN) $(GRP) $(LIBDIR)/$(SGS)back back


save:	$(LIBDIR)/front	$(LIBDIR)/back
	cp $(LIBDIR)/front $(LIBDIR)/front.back
	cp $(LIBDIR)/back $(LIBDIR)/back.back

uninstall:	$(LIBDIR)/front.back $(LIBDIR)/back.back
	cp $(LIBDIR)/front.back $(LIBDIR)/front
	cp $(LIBDIR)/back.back $(LIBDIR)/back
