#	Copyright (c) 1985 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#/*   @(#)makefile	1.3 - 85/08/09 */
#
#	iAPX286 DISASSEMBLER MAKEFILE 
#
#
SGS	= i286
SGSBASE = ../..
MODE	= 0755
OWN	= bin
GRP	= bin
CC	= cc
MODEL	=
MSPEC	=
CFLAGS	= -O $(MSPEC)
FLEX	= -DFLEXNAMES
FFLAG	=
ARCH	= AR32WR
ARFORMAT= PORTAR
DEFLIST = -D$(ARCH) -DiAPX286 -D$(ARFORMAT) $(FLEX)
INCLIST = -I$(DISCOM) -I$(COMINC) -I$(DISINC) -I.
CC_CMD	= $(CC) -c $(CFLAGS) $(DEFLIST) $(INCLIST)
STRIP	= strip
#
LINT	= lint
LINTFLAGS =
#
#	External Directories
#
BINDIR	= $(ROOT)/bin
LIBDIR	= $(ROOT)/lib
#
#	Internal Directories
#
DISINC	= $(SGSBASE)/inc/i286
COMINC	= $(SGSBASE)/inc/common
DISCOM	= $(SGSBASE)/dis/common
#
LDFLAGS	=
LIBLD	= -lld
#
OBJECTS	= dis_main.o dis_utls.o bits.o tables.o dis_extn.o lists.o
#
#
build:	dis
#
dis:	$(OBJECTS)
	$(CC) $(FFLAG) $(MSPEC) -o dis $(OBJECTS) $(LIBLD) $(LDFLAGS)
#
dis_extn.o:	dis.h $(DISCOM)/dis_extn.c $(COMINC)/scnhdr.h \
	$(COMINC)/filehdr.h $(COMINC)/ldfcn.h $(COMINC)/linenum.h \
	$(COMINC)/syms.h $(COMINC)/storclass.h $(DISCOM)/structs.h
	$(CC_CMD) $(DISCOM)/dis_extn.c

dis_main.o:	dis.h $(DISCOM)/dis_main.c $(COMINC)/filehdr.h \
	$(COMINC)/ldfcn.h $(DISINC)/paths.h $(DISINC)/sgs.h \
	$(COMINC)/scnhdr.h $(COMINC)/syms.h\
        $(COMINC)/storclass.h $(COMINC)/linenum.h $(DISCOM)/structs.h
	$(CC_CMD) $(DISCOM)/dis_main.c

dis_utls.o:	dis.h $(DISCOM)/dis_utls.c $(COMINC)/filehdr.h \
	$(COMINC)/ldfcn.h $(COMINC)/scnhdr.h $(COMINC)/syms.h $(DISINC)/sgs.h \
	$(COMINC)/linenum.h $(COMINC)/storclass.h $(DISCOM)/structs.h
	$(CC_CMD) $(DISCOM)/dis_utls.c

lists.o:	dis.h  $(DISCOM)/lists.c $(DISCOM)/structs.h \
		$(COMINC)/ldfcn.h  $(DISINC)/paths.h  $(DISINC)/sgs.h \
		$(COMINC)/scnhdr.h $(COMINC)/linenum.h $(COMINC)/syms.h \
		$(COMINC)/storclass.h
	$(CC_CMD) $(DISCOM)/lists.c

bits.o:	dis.h bits.c $(COMINC)/filehdr.h \
	$(COMINC)/ldfcn.h $(COMINC)/scnhdr.h $(DISINC)/sgs.h
	$(CC_CMD) bits.c

tables.o:	dis.h tables.c
	$(CC_CMD) tables.c

#
save:		$(BINDIR)/$(SGS)dis
	-rm -f $(BINDIR)/$(SGS)dis.back
	cp $(BINDIR)/$(SGS)dis $(BINDIR)/$(SGS)dis.back

install: dis
	$(STRIP) dis
	sh $(SGSBASE)/sgs.install $(MODE) $(OWN) $(GRP) $(BINDIR)/$(SGS)dis dis

uninstall:	$(BINDIR)/$(SGS)dis.back
	-rm -f $(BINDIR)/$(SGS)dis
	cp $(BINDIR)/$(SGS)dis.back $(BINDIR)/$(SGS)dis

clean:
	-rm -f $(OBJECTS)

clobber: clean
	-rm -f dis

shrink: clean clobber

lint:
	$(LINT) $(LINTFLAGS) $(INCLIST) $(DISCOM)/*.c
