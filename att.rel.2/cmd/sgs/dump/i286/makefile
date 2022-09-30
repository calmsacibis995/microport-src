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
DUMPCOM	= $(SGSBASE)/dump/common
CC	= cc
MODEL	= 
MSPEC	= 
CFLAGS	= -O $(MSPEC)
FLEX	= -DFLEXNAMES
DEFLIST	= -D$(ARFORMAT) $(FLEX)
INCLIST	= -I$(INCCOM) -I$(DUMPCOM) -I$(INCI286)
CC_CMD	= $(CC) -c $(CFLAGS) $(DEFLIST) $(INCLIST)
STRIP	= strip
LIBLD	= -lld
LINT	= lint
LINTFLAGS =
ARFORMAT = PORTAR
OBJECTS	= main.o setflags.o openobject.o process.o fcns.o

build: dump

dump:  $(OBJECTS)
	$(CC) $(MSPEC) -o dump $(OBJECTS) $(LIBLD)

main.o:  $(DUMPCOM)/main.c $(INCCOM)/filehdr.h $(INCCOM)/ldfcn.h \
	$(DUMPCOM)/filedefs.h $(INCI286)/sgs.h $(DUMPCOM)/dumpmap.h
	$(CC_CMD) $(DUMPCOM)/main.c

setflags.o:  $(DUMPCOM)/setflags.c $(DUMPCOM)/flagdefs.h $(INCI286)/sgs.h
	$(CC_CMD)  $(DUMPCOM)/setflags.c

openobject.o:  $(DUMPCOM)/openobject.c $(INCCOM)/filehdr.h \
	$(INCCOM)/ldfcn.h $(INCI286)/sgs.h $(DUMPCOM)/filedefs.h
	$(CC_CMD) $(DUMPCOM)/openobject.c

process.o:  $(DUMPCOM)/process.c $(INCCOM)/filehdr.h $(INCCOM)/ldfcn.h \
	$(INCI286)/sgs.h $(DUMPCOM)/filedefs.h $(DUMPCOM)/process.h \
	$(DUMPCOM)/flagdefs.h
	$(CC_CMD) $(DUMPCOM)/process.c

fcns.o:  $(DUMPCOM)/fcns.c $(INCCOM)/filehdr.h $(INCCOM)/scnhdr.h \
	$(INCCOM)/reloc.h $(INCCOM)/linenum.h $(INCCOM)/syms.h \
	$(INCCOM)/storclass.h $(INCCOM)/ldfcn.h  $(INCI286)/sgs.h
	$(CC_CMD) $(DUMPCOM)/fcns.c

save:	$(BINDIR)/$(SGS)dump
	-rm -f $(BINDIR)/$(SGS)dump.back
	cp $(BINDIR)/$(SGS)dump $(BINDIR)/$(SGS)dump.back

install: dump
	$(STRIP) dump
	sh $(SGSBASE)/sgs.install $(MODE) $(OWN) $(GRP) $(BINDIR)/$(SGS)dump dump

uninstall:	$(BINDIR)/$(SGS)dump.back
	-rm -f $(BINDIR)/$(SGS)dump
	cp $(BINDIR)/$(SGS)dump.back $(BINDIR)/$(SGS)dump

lint:	$(DUMPCOM)/main.c $(DUMPCOM)/setflags.c $(DUMPCOM)/openobject.c\
	$(DUMPCOM)/process.c $(DUMPCOM)/fcns.c\
	$(INCCOM)/filehdr.h $(INCCOM)/ldfcn.h $(DUMPCOM)/filedefs.h\
	$(INCI286)/sgs.h $(DUMPCOM)/process.h $(DUMPCOM)/flagdefs.h \
	$(INCCOM)/scnhdr.h $(INCCOM)/reloc.h $(INCCOM)/linenum.h\
	$(INCCOM)/syms.h $(INCCOM)/storclass.h $(DUMPCOM)/dumpmap.h
	$(LINT) $(LINTFLAGS) $(INCLIST) $(DUMPCOM)/*.c

shrink: clean clobber

clean:
	-rm -f *.o 
	
clobber: clean
	-rm -f dump
