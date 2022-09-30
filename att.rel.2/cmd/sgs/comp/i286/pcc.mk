#	Copyright (c) 1985 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#/*   @(#)pcc.mk	1.3 - 85/08/09 */
#
#
SGSBASE	= .
YACC	= yacc
ARCH	= AR32WR
FLEX	= -DFLEXNAMES
LIBDIR	= /lib
COMPCOM	= ../mip
INCC286	= ../../../sgs/inc/iAPX286
CFLAGS  = -Ml -O
DEFLIST	= $(FLEX) -D$(ARCH)
INCLIST	= -I$(INCC286) -I$(COMPCOM) -I.
CC	= cc
CC_CMD	= $(CC) -c $(CFLAGS) $(INCLIST) $(DEFLIST)
LINT	= lint
LINTF	= -hv $(INCLIST) $(DEFLIST)

OFRONT=	code.o local.o cgram.o comm1.o messages.o optim.o \
	pftn.o scan.o trees.o xdefs.o

OBACK=	local2.o order.o seg.o table.o allo.o comm2.o match.o reader.o

OBJECTS=$(OFRONT) $(OBACK)

CFRONT=	code.c local.c $(COMPCOM)/cgram.c $(COMPCOM)/comm1.c \
	$(COMPCOM)/messages.c $(COMPCOM)/optim.c $(COMPCOM)/pftn.c \
	$(COMPCOM)/scan.c $(COMPCOM)/trees.c $(COMPCOM)/xdefs.c

CBACK=	local2.c order.c seg.c table.c $(COMPCOM)/allo.c $(COMPCOM)/comm2.c \
	$(COMPCOM)/match.c $(COMPCOM)/reader.c

HFRONT=	macdefs $(INCC286)/paths.h $(INCC286)/sgs.h $(COMPCOM)/common \
	$(COMPCOM)/manifest $(COMPCOM)/messages.h $(COMPCOM)/mfile1

HBACK=	macdefs mac2defs $(INCC286)/paths.h $(COMPCOM)/common \
	$(COMPCOM)/manifest $(COMPCOM)/mfile2 

all:	front back

head:  front back ;

# FRONT - PASS ONE

front: $(OFRONT)
	$(CC) -Ml -s -o front $(OFRONT)

code.o:	code.c macdefs $(INCC286)/paths.h $(INCC286)/sgs.h \
	$(COMPCOM)/manifest $(COMPCOM)/mfile1
	$(CC_CMD) -Ml code.c

local.o:	local.c $(COMPCOM)/manifest macdefs $(COMPCOM)/mfile1
		$(CC_CMD) -Ml local.c

cgram.o:	$(COMPCOM)/cgram.c macdefs $(COMPCOM)/manifest $(COMPCOM)/messages.h \
		$(COMPCOM)/mfile1
		$(CC_CMD) -Ml -DYYDEBUG=0 $(COMPCOM)/cgram.c

$(COMPCOM)/cgram.c:	$(COMPCOM)/cgram.y
		@echo "Expect to see message conflicts: 7 shift/reduce"
		$(YACC) $(COMPCOM)/cgram.y
		mv y.tab.c $(COMPCOM)/cgram.c

comm1.o:	$(COMPCOM)/comm1.c macdefs $(COMPCOM)/common \
		$(COMPCOM)/manifest $(COMPCOM)/mfile1
		$(CC_CMD) -Ml $(COMPCOM)/comm1.c

messages.o:	$(COMPCOM)/messages.c $(COMPCOM)/messages.h
		$(CC_CMD) -Ml $(COMPCOM)/messages.c
		
optim.o:	$(COMPCOM)/optim.c macdefs $(COMPCOM)/manifest \
		$(COMPCOM)/mfile1
		$(CC_CMD) -Ml $(COMPCOM)/optim.c

pftn.o:	$(COMPCOM)/pftn.c macdefs $(COMPCOM)/manifest $(COMPCOM)/messages.h \
	$(COMPCOM)/mfile1
	$(CC_CMD) -Ml $(COMPCOM)/pftn.c

scan.o:	$(COMPCOM)/scan.c macdefs $(COMPCOM)/manifest $(COMPCOM)/messages.h \
	$(COMPCOM)/mfile1
	$(CC_CMD) -Ml $(COMPCOM)/scan.c

trees.o:	$(COMPCOM)/trees.c macdefs $(COMPCOM)/manifest \
		$(COMPCOM)/messages.h $(COMPCOM)/mfile1
		$(CC_CMD) -Ml $(COMPCOM)/trees.c

xdefs.o:	$(COMPCOM)/xdefs.c macdefs $(COMPCOM)/manifest \
		$(COMPCOM)/mfile1
		$(CC_CMD) -Ml $(COMPCOM)/xdefs.c

# BACK - PASS TWO

back:	$(OBACK)
	$(CC) -Ml -s -o back $(OBACK)

local2.o :	macdefs mac2defs $(COMPCOM)/manifest $(COMPCOM)/mfile2
	$(CC_CMD) -Ml local2.c

order.o:	macdefs mac2defs $(COMPCOM)/manifest $(COMPCOM)/mfile2
	$(CC_CMD) -Ml order.c

seg.o:	macdefs mac2defs $(COMPCOM)/manifest $(COMPCOM)/mfile2
	$(CC_CMD) -Ml seg.c

table.o:	macdefs mac2defs $(COMPCOM)/manifest $(COMPCOM)/mfile2
	$(CC_CMD) -Ml table.c

allo.o:	$(COMPCOM)/allo.c macdefs mac2defs $(COMPCOM)/manifest $(COMPCOM)/mfile2
	$(CC_CMD) -Ml $(COMPCOM)/allo.c

comm2.o:	$(COMPCOM)/comm2.c macdefs mac2defs $(COMPCOM)/common \
		$(COMPCOM)/manifest $(COMPCOM)/mfile2
	$(CC_CMD) -Ml $(COMPCOM)/comm2.c

match.o:	$(COMPCOM)/match.c macdefs mac2defs $(COMPCOM)/manifest \
		$(COMPCOM)/mfile2
	$(CC_CMD) -Ml $(COMPCOM)/match.c

reader.o:	$(COMPCOM)/reader.c macdefs mac2defs $(COMPCOM)/manifest \
		$(COMPCOM)/mfile2
	$(CC_CMD) -Ml $(COMPCOM)/reader.c


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
	-rm -f $(LIBDIR)/front $(LIBDIR)/back
	cp front $(LIBDIR)/front
	cp back $(LIBDIR)/back
	touch install

save:	$(LIBDIR)/front	$(LIBDIR)/back
	cp $(LIBDIR)/front $(LIBDIR)/front.back
	cp $(LIBDIR)/back $(LIBDIR)/back.back

uninstall:	$(LIBDIR)/front.back $(LIBDIR)/back.back
	cp $(LIBDIR)/front.back $(LIBDIR)/front
	cp $(LIBDIR)/back.back $(LIBDIR)/back
