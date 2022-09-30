#	Copyright (c) 1985 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#/*   @(#)makefile	1.3 - 85/08/09 */
SGS	= i286
SGSBASE	= ../..
MODE	= 0755
OWN	= bin
GRP	= bin
BINDIR 	= $(ROOT)/bin
LIBDIR	= $(ROOT)/lib
INCI286	= $(SGSBASE)/inc/i286
INCCOM	= $(SGSBASE)/inc/common
NMCOM	= $(SGSBASE)/nm/common
SGSNAME	= I286
FLEX	= -DFLEXNAMES
DEFLIST	= -D$(SGSNAME) -D$(ARFORMAT) $(FLEX)
INCLIST	= -I$(INCI286) -I$(INCCOM) -I$(INCCOM)
MODEL	=
MSPEC	=
CFLAGS	= -O $(MSPEC)
CC	= cc
CC_CMD	= $(CC) -c $(CFLAGS) $(DEFLIST) $(INCLIST)
STRIP	= strip
LINT	= lint
LFLAGS	=
LIBLD	= -lld
ARFORMAT = PORTAR
OBJECTS	= main.o setflags.o process.o prosym.o decotype.o fcns.o

build: nm

nm:  $(OBJECTS) $(INCCOM)/filehdr.h $(INCCOM)/ldfcn.h $(INCCOM)/syms.h \
	   $(INCCOM)/storclass.h $(INCI286)/paths.h $(INCI286)/sgs.h \
	   $(NMCOM)/prosym.h $(NMCOM)/decotype.h
	$(CC) $(MSPEC) -o nm $(OBJECTS) $(LIBLD)


main.o:  $(NMCOM)/main.c $(INCCOM)/filehdr.h $(INCCOM)/ldfcn.h $(INCI286)/sgs.h \
	 $(NMCOM)/defs.h
	$(CC_CMD) $(NMCOM)/main.c

setflags.o:  $(NMCOM)/setflags.c $(INCI286)/sgs.h $(INCI286)/paths.h $(NMCOM)/defs.h
	$(CC_CMD) $(NMCOM)/setflags.c

process.o:  $(NMCOM)/process.c $(INCCOM)/filehdr.h $(INCCOM)/ldfcn.h \
	    $(INCCOM)/scnhdr.h $(NMCOM)/defs.h
	$(CC_CMD) $(NMCOM)/process.c

prosym.o:  $(NMCOM)/prosym.c $(INCCOM)/filehdr.h $(INCCOM)/ldfcn.h $(INCCOM)/syms.h \
	  $(INCCOM)/storclass.h   $(NMCOM)/defs.h $(NMCOM)/prosym.h
	$(CC_CMD) $(NMCOM)/prosym.c

decotype.o:  $(NMCOM)/decotype.c $(INCCOM)/filehdr.h $(INCCOM)/ldfcn.h \
	    $(INCCOM)/syms.h $(INCCOM)/storclass.h  $(NMCOM)/decotype.h $(NMCOM)/defs.h
	$(CC_CMD) $(NMCOM)/decotype.c

fcns.o:  $(NMCOM)/fcns.c $(INCCOM)/filehdr.h $(INCCOM)/ldfcn.h $(INCI286)/sgs.h \
	 $(NMCOM)/defs.h
	$(CC_CMD) $(NMCOM)/fcns.c

save:	$(BINDIR)/$(SGS)nm
	-rm -f $(BINDIR)/$(SGS)nm.back
	cp $(BINDIR)/$(SGS)nm $(BINDIR)/$(SGS)nm.back

install: nm
	$(STRIP) nm
	sh $(SGSBASE)/sgs.install $(MODE) $(OWN) $(GRP) $(BINDIR)/$(SGS)nm nm

uninstall:	$(BINDIR)/$(SGS)nm.back
	-rm -f $(BINDIR)/$(SGS)nm
	cp $(BINDIR)/$(SGS)nm.back $(BINDIR)/$(SGS)nm

shrink: clean clobber
clean:
	-rm -f $(OBJECTS)
clobber: clean
	-rm -f nm

lint:	$(NMCOM)/main.c $(NMCOM)/setflags.c $(NMCOM)/process.c\
	$(NMCOM)/prosym.c $(NMCOM)/decotype.c $(NMCOM)/fcns.c\
	$(INCCOM)/syms.h $(INCCOM)/storclass.h $(INCI286)/paths.h\
	$(INCI286)/sgs.h $(NMCOM)/prosym.h $(NMCOM)/decotype.h\
	$(INCCOM)/filehdr.h $(INCCOM)/scnhdr.h\
	$(INCCOM)/ldfcn.h $(NMCOM)/defs.h
	$(LINT) $(LFLAGS) $(DEFLIST) $(INCLIST) $(NMCOM)/*.c
