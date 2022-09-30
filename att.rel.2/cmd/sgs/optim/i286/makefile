#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)ccs-s5:optim/i286/makefile	1.3"

#*************************************************************************
#
#	MAKEFILE FOR OPTIM 
#
#	Files making up this release are:-
#
#	1. Common.
#
#		inter.c
#		optim.c
#		optim.h
#
#	2. Machine dependant.
#
#		defs		-	configuration and basic defs include
#		local.l		-	lex to process input assembler source
#		mdoptim1.c	-	provides routines required by optim.c
#		mdoptim2.c	-	contains m/c dependant optimisers
#		mdoptim3.c	-	support for all of the above
#		mdoptim4.c	-	register tracking routines
#
#*************************************************************************

SGSBASE	= ../..

#
#
#

CC	= cc
LEX	= lex
LINT	= lint

MODEL	=
MSPEC	=
CFLAGS	= -O $(MSPEC)
LDFLAGS = -s
DEFLIST	= -D$(ARCH)
INCLIST	= -I$(OPTIMCOM) -I. -I$(INCB16)
CC_CMD	= $(CC) -c $(CFLAGS) $(DEFLIST) $(INCLIST)
OWN	= bin
GRP	= bin
MODE	= 0775

#
#
#

LIBDIR	= $(ROOT)/lib

# Changed to use i286 include directory
# (inc/b16 directory was not sent by DRI).  
#
#	INCB16	= $(SGSBASE)/inc/b16
#					Michael Winton

INCB16	= $(SGSBASE)/inc/i286
OPTIMCOM= $(SGSBASE)/optim/common

ARCH	= AR32WR
iAPX286	= -DiAPX286

#
#	FILES making up optim
#

HFILES	= defs $(OPTIMCOM)/optim.h # not used $(INCB16)/sgs.h
CFILES	= local.c  $(OPTIMCOM)/optim.c $(OPTIMCOM)/inter.c \
	  mdoptim1.c mdoptim2.c mdoptim3.c mdoptim4.c
OBJECTS	= optim.o inter.o local.o  mdoptim1.o mdoptim2.o mdoptim3.o mdoptim4.o

#
#
#

build:	optim
optim:	$(OBJECTS)
	$(CC) $(LDFLAGS) $(CFLAGS) $(OBJECTS) $(CLIB) -o optim

optim.o: $(OPTIMCOM)/optim.h defs $(OPTIMCOM)/optim.c
	$(CC_CMD) $(OPTIMCOM)/optim.c

inter.o: $(OPTIMCOM)/optim.h defs $(OPTIMCOM)/inter.c
	$(CC_CMD) $(OPTIMCOM)/inter.c

local.o: $(OPTIMCOM)/optim.h defs local.c # not used $(INCB16)/sgs.h 
	$(CC_CMD) $(iAPX286) local.c

local.c: local.l
	$(LEX) local.l
	mv lex.yy.c local.c

mdoptim1.o: $(OPTIMCOM)/optim.h defs mdoptim1.c
	$(CC_CMD) $(iAPX286) mdoptim1.c

mdoptim2.o: $(OPTIMCOM)/optim.h defs mdoptim2.c
	$(CC_CMD) $(iAPX286) mdoptim2.c

mdoptim3.o: $(OPTIMCOM)/optim.h defs mdoptim3.c
	$(CC_CMD) $(iAPX286) mdoptim3.c

mdoptim4.o: $(OPTIMCOM)/optim.h defs mdoptim4.c
	$(CC_CMD) $(iAPX286) mdoptim4.c

#
#	TIDYING - UP
#

shrink:	clean clobber

clean:
	-rm -f $(OBJECTS) install local.c

clobber: clean
	-rm -f optim

#
#	INSTALL
#

install: optim
	sh $(SGSBASE)/sgs.install $(MODE) $(OWN) $(GRP) $(LIBDIR)/$(SGS)optim optim

#
#	SAVE
#

save:	 	$(LIBDIR)/optim
	-rm -f $(LIBDIR)/optim.back
	cp $(LIBDIR)/optim $(LIBDIR)/optim.back

#
#	UNINSTALL
#

uninstall:	$(LIBDIR)/optim.back
	-rm -f $(LIBDIR)/optim
	cp $(LIBDIR)/optim.back $(LIBDIR)/optim

#
#	STRIP
#

strip:	optim
	strip optim

#
#	LINT
#

lint:	$(HFILES) $(CFILES)
	$(LINT) $(DEFLIST) $(INCLIST) $(CFILES)

#
#	LISTINGS
#


listing:
	@pr -n defs 
	@pr -n $(OPTIMCOM)/optim.h  
	@pr -n $(INCB16)/sgs.h 
	@pr -n makefile 
	@pr -n local.l 
	@pr -n $(OPTIMCOM)/optim.c 
	@pr -n $(OPTIMCOM)/inter.c 
	@pr -n mdoptim1.c 
	@pr -n mdoptim2.c 
	@pr -n mdoptim3.c 
	@pr -n mdoptim4.c 
