#	Copyright (c) 1985 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#/*   @(#)makefile	1.3 - 85/08/09 */
#
#	I286 STRIP MAKEFILE
#
SGS	= i286
SGSBASE	= ../..
MODE	= 0755
OWN	= bin
GRP	= bin
BINDIR	= $(ROOT)/bin
LIBDIR	= $(ROOT)/lib
STRIPCOM = $(SGSBASE)/strip/common
INCCOM	= $(SGSBASE)/inc/common
INCI286	= $(SGSBASE)/inc/i286
CC	= cc
MODEL	=
MSPEC	=
CFLAGS	= -O $(MSPEC)
FLEX	= -DFLEXNAMES
DEFLIST	= -D$(ARFORMAT) $(FLEX)
INCLIST	= -I$(INCI286) -I$(INCCOM) -I$(STRIPCOM)
CC_CMD	= $(CC) -c $(CFLAGS) $(DEFLIST) $(INCLIST)
STRIP	= /bin/strip
LINT	= lint
LINTFLAGS =
LDFLAGS	= 
LIBLD	= -lld
ARFORMAT = PORTAR
OBJECTS = main.o setflags.o process.o checkflags.o buildtmp.o buildfcns.o \
	 copytmp.o fcns.o

build:	strip

strip: $(OBJECTS)
	$(CC) $(MSPEC) -o strip $(OBJECTS) $(LDFLAGS) $(LIBLD)

main.o:  $(STRIPCOM)/main.c $(INCCOM)/filehdr.h $(INCCOM)/ldfcn.h $(INCI286)/paths.h \
	 $(INCI286)/sgs.h
	$(CC_CMD) $(STRIPCOM)/main.c

setflags.o:  $(STRIPCOM)/setflags.c $(INCI286)/sgs.h $(STRIPCOM)/defs.h
	$(CC_CMD) $(STRIPCOM)/setflags.c

process.o:  $(STRIPCOM)/process.c $(INCCOM)/filehdr.h $(INCCOM)/ldfcn.h \
	    $(STRIPCOM)/defs.h $(INCI286)/paths.h $(INCI286)/sgs.h
	$(CC_CMD) $(STRIPCOM)/process.c

checkflags.o: $(STRIPCOM)/checkflags.c $(INCCOM)/filehdr.h $(INCCOM)/ldfcn.h
	$(CC_CMD) $(STRIPCOM)/checkflags.c

buildtmp.o:  $(STRIPCOM)/buildtmp.c $(INCCOM)/filehdr.h $(INCCOM)/ldfcn.h \
	     $(STRIPCOM)/defs.h
	$(CC_CMD) $(STRIPCOM)/buildtmp.c

buildfcns.o:  $(STRIPCOM)/buildfcns.c $(INCCOM)/filehdr.h $(INCCOM)/scnhdr.h \
	      $(INCCOM)/reloc.h $(INCCOM)/syms.h $(INCCOM)/ldfcn.h \
	      $(INCCOM)/storclass.h $(STRIPCOM)/defs.h $(INCI286)/sgs.h $(INCCOM)/linenum.h
	$(CC_CMD) $(STRIPCOM)/buildfcns.c

copytmp.o:  $(STRIPCOM)/copytmp.c $(INCCOM)/filehdr.h $(INCCOM)/ldfcn.h \
	    $(STRIPCOM)/defs.h
	$(CC_CMD) $(STRIPCOM)/copytmp.c

fcns.o:  $(STRIPCOM)/fcns.c $(INCCOM)/filehdr.h $(INCCOM)/ldfcn.h $(INCI286)/sgs.h \
	 $(STRIPCOM)/defs.h
	$(CC_CMD) $(STRIPCOM)/fcns.c


save:	$(BINDIR)/$(SGS)strip
	-rm -f $(BINDIR)/$(SGS)strip.back
	cp $(BINDIR)/$(SGS)strip $(BINDIR)/$(SGS)strip.back

install: strip
	$(STRIP) strip
	sh $(SGSBASE)/sgs.install $(MODE) $(OWN) $(GRP) $(BINDIR)/$(SGS)strip strip

uninstall:	$(BINDIR)/$(SGS)strip.back
	-rm -f $(BINDIR)/$(SGS)strip
	cp $(BINDIR)/$(SGS)strip.back $(BINDIR)/$(SGS)strip

lint:	$(STRIPCOM)/main.c $(STRIPCOM)/setflags.c $(STRIPCOM)/process.c\
	$(STRIPCOM)/checkflags.c $(STRIPCOM)/buildtmp.c $(STRIPCOM)/buildfcns.c\
	$(STRIPCOM)/copytmp.c $(STRIPCOM)/fcns.c\
	$(INCCOM)/reloc.h $(INCCOM)/syms.h $(INCCOM)/storclass.h\
	$(INCI286)/paths.h $(INCI286)/sgs.h $(STRIPCOM)/defs.h\
	$(INCCOM)/filehdr.h $(INCCOM)/ldfcn.h $(INCCOM)/scnhdr.h
	$(LINT) $(LINTFLAGS) $(DEFLIST) $(INCLIST) $(STRIPCOM)/*.c

shrink: clean clobber

clean:
	-rm -f $(OBJECTS)

clobber: clean
	-rm -f strip
