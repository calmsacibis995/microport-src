#	Copyright (c) 1985 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#/*   @(#)makefile	1.3 - 85/08/08 */
#	AR MAKEFILE
#
#
ROOT	=
SGSBASE	= ../..

SGS	= i286
BINDIR	= $(ROOT)/bin
INCCOM	= $(SGSBASE)/inc/common
INCI286	= $(SGSBASE)/inc/i286
ARCOM	= $(SGSBASE)/ar/common
LIBLD	= -lld

MSPEC	=
MODEL	=
CC	= cc
LINT	= lint
CFLAGS	= -O $(MSPEC)
LINTFLAGS =
LDFLAGS	=
LIBES	= $(LIBLD)
ARFORMAT= PORTAR
DEFLIST	= $(FLEX) -D$(ARFORMAT)
INCLIST	= -I$(INCCOM) -I$(INCI286)
ENV	=

FLEX	= -DFLEXNAMES

FRC	=
O	= o

MODE	= 0755
OWN	= bin
GRP	= bin

CP	= cp
RM	= rm
PRINT	= pr
PRFLAGS	=
LP	= lp
STRIP	= strip

OBJECTS	= ar.o
SOURCE	= $(ARCOM)/ar.c
HEADERS	= $(INCCOM)/ar.h $(INCCOM)/filehdr.h $(INCCOM)/syms.h $(INCI286)/paths.h


build:	ar

#-------------------------
ar:	$(OBJECTS) $(FRC)
	$(CC) $(CFLAGS) $(LDFLAGS) -o ar $(OBJECTS) $(LIBES)

ar.$O:	$(ARCOM)/ar.c $(HEADERS) $(FRC)
	$(CC) -c $(CFLAGS) $(DEFLIST) $(INCLIST) $(ENV) $(ARCOM)/ar.c


#-------------------------

install: ar
	$(STRIP) ar
	sh $(SGSBASE)/sgs.install $(MODE) $(OWN) $(GRP) $(BINDIR)/$(SGS)ar ar

#--------------------------

save:
	$(RM) -f $(BINDIR)/$(SGS)ar.back
	$(CP) $(BINDIR)/$(SGS)ar $(BINDIR)/$(SGS)ar.back

#--------------------------

uninstall:	$(BINDIR)/$(SGS)ar.back
	$(RM) -f $(BINDIR)/$(SGS)ar
	$(CP) $(BINDIR)/$(SGS)ar.back $(BINDIR)/$(SGS)ar

#--------------------------

shrink: clean clobber

#--------------------------

clean:
	$(RM) -f $(OBJECTS)

#--------------------------

clobber:	clean
	$(RM) -f ar

#--------------------------

lint:	$(SOURCE) $(HEADERS)
	$(LINT) $(LINTFLAGS) $(DEFLIST) $(INCLIST) $(SOURCE) >lint.out

#---------------------------

listing:	$(SOURCE) $(HEADERS)
	$(PRINT) $(PRFLAGS) $(HEADERS) $(SOURCE) | $(LP)

#---------------------------

FRC:
