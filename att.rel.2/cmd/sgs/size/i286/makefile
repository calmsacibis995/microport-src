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
BINDIR	= $(ROOT)/bin
LIBDIR	= $(ROOT)/lib
INCI286	= $(SGSBASE)/inc/i286
INCCOM	= $(SGSBASE)/inc/common
SIZECOM	= $(SGSBASE)/size/common
SGSNAME	= I286
CC	= cc
MODEL	=
MSPEC	=
CFLAGS	= -O $(MSPEC)
DEFLIST	= -D$(ARFORMAT) -D$(SGSNAME)
INCLIST	= -I$(INCI286) -I$(INCCOM) -I$(SIZECOM)
CC_CMD	= $(CC) -c $(CFLAGS) $(DEFLIST) $(INCLIST)
STRIP	= strip
LINT	= lint
LINTFLAGS =
LIBLD	= -lld
OBJECTS	= main.o process.o fcns.o
ARFORMAT = PORTAR

build: size

size:  $(OBJECTS)
	$(CC) $(MSPEC) -o size $(OBJECTS) $(LIBLD)

main.o:   $(INCCOM)/filehdr.h $(INCCOM)/ldfcn.h $(INCI286)/sgs.h \
		$(SIZECOM)/defs.h  $(SIZECOM)/main.c
	$(CC_CMD) $(SIZECOM)/main.c

process.o:  $(INCCOM)/filehdr.h $(INCCOM)/scnhdr.h $(INCCOM)/ldfcn.h \
	    $(SIZECOM)/process.h $(SIZECOM)/defs.h \
	    $(SIZECOM)/process.c $(INCI286)/sgs.h
	$(CC_CMD) $(SIZECOM)/process.c

fcns.o:	$(INCCOM)/filehdr.h $(INCCOM)/scnhdr.h $(INCCOM)/ldfcn.h \
	$(INCI286)/sgs.h $(SIZECOM)/defs.h $(SIZECOM)/fcns.c
	$(CC_CMD) $(SIZECOM)/fcns.c

save:	$(BINDIR)/$(SGS)size
	-rm -f $(BINDIR)/$(SGS)size.back
	cp $(BINDIR)/$(SGS)size $(BINDIR)/$(SGS)size.back

install: size
	$(STRIP) size
	sh $(SGSBASE)/sgs.install $(MODE) $(OWN) $(GRP) $(BINDIR)/$(SGS)size size

uninstall:	$(BINDIR)/$(SGS)size.back
	-rm -f $(BINDIR)/$(SGS)size
	cp $(BINDIR)/$(SGS)size.back $(BINDIR)/$(SGS)size

shrink: clean clobber
clean:
	-rm -f $(OBJECTS) 
clobber: clean
	-rm -f size

lint:	$(SIZECOM)/main.c $(SIZECOM)/process.c $(SIZECOM)/fcns.c\
	$(INCCOM)/filehdr.h $(INCCOM)/ldfcn.h $(INCCOM)/scnhdr.h\
	$(INCI286)/sgs.h $(SIZECOM)/process.h $(SIZECOM)/defs.h
	$(LINT) $(LINTFLAGS) $(DEFLIST) $(INCLIST) $(SIZECOM)/*.c
