#	Copyright (c) 1985 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#/*   @(#)makefile	1.3 - 85/08/09 */
SGS	= i286
SGSBASE = ../..
MODE	= 0755
OWN	= bin
GRP	= bin
BINDIR	= $(SGSBASE)/xenv/i286/bin
LIBDIR	= $(SGSBASE)/xenv/i286/lib
INCI286 = $(SGSBASE)/inc/i286
INCCOM	= $(SGSBASE)/inc/common
LISTCOM = $(SGSBASE)/list/common
INCLIST = -I$(INCI286) -I$(INCCOM) -I$(LISTCOM)
CC	= cc
CC_CMD	= $(CC) -c $(CFLAGS) $(DEFLIST) $(INCLIST)
STRIP	= strip
LINT	= lint
LINTFLAGS = -p
LIBLD	= $(SGSBASE)/libld/i286/libld.a
OBJECTS = main.o initfcns.o process.o symfcns.o findname.o parsefcns.o \
	 srciofcns.o fcns.o

build:	list

list:  $(OBJECTS) $(LIBLD)
	$(CC) -o list $(OBJECTS) $(LIBLD)

main.o:  $(LISTCOM)/main.c $(INCCOM)/filehdr.h $(INCCOM)/ldfcn.h \
	$(INCI286)/sgs.h $(LISTCOM)/defs.h
	$(CC_CMD) $(LISTCOM)/main.c

initfcns.o:  $(LISTCOM)/initfcns.c $(INCCOM)/filehdr.h $(INCCOM)/ldfcn.h \
	     $(INCI286)/paths.h $(INCI286)/sgs.h $(LISTCOM)/defs.h
	$(CC_CMD) $(LISTCOM)/initfcns.c

process.o:  $(LISTCOM)/process.c $(LISTCOM)/defs.h
	$(CC_CMD) $(LISTCOM)/process.c

symfcns.o:  $(LISTCOM)/symfcns.c $(INCCOM)/filehdr.h $(INCCOM)/syms.h \
	$(INCCOM)/ldfcn.h $(INCCOM)/storclass.h $(LISTCOM)/defs.h
	$(CC_CMD) $(LISTCOM)/symfcns.c

findname.o:  $(LISTCOM)/findname.c $(INCCOM)/syms.h \
		$(INCCOM)/storclass.h $(LISTCOM)/defs.h \
		$(LISTCOM)/findname.h
	$(CC_CMD) $(LISTCOM)/findname.c

parsefcns.o:  $(LISTCOM)/parsefcns.c $(LISTCOM)/defs.h
	$(CC_CMD) $(LISTCOM)/parsefcns.c

srciofcns.o:	$(LISTCOM)/srciofcns.c $(INCCOM)/filehdr.h \
		$(INCCOM)/linenum.h  $(INCCOM)/ldfcn.h \
		$(LISTCOM)/defs.h $(INCCOM)/sgsmacros.h
	$(CC_CMD) $(LISTCOM)/srciofcns.c

fcns.o:		$(LISTCOM)/fcns.c $(INCCOM)/filehdr.h $(INCCOM)/ldfcn.h\
		$(INCI286)/sgs.h $(LISTCOM)/defs.h
	$(CC_CMD) $(LISTCOM)/fcns.c


clean:
	-rm -f $(OBJECTS)

clobber: clean
	-rm -f list

shrink: clean clobber

save:	$(BINDIR)/$(SGS)list
	-rm -f $(BINDIR)/$(SGS)list.back
	cp $(BINDIR)/$(SGS)list $(BINDIR)/$(SGS)list.back

install: list
	$(STRIP) list
	sh $(SGSBASE)/sgs.install $(MODE) $(OWN) $(GRP) $(BINDIR)/$(SGS)list list

uninstall:	$(BINDIR)/$(SGS)list.back
	-rm -f $(BINDIR)/$(SGS)list
	cp $(BINDIR)/$(SGS)list.back $(BINDIR)/$(SGS)list

lint:	$(LISTCOM)/main.c $(LISTCOM)/initfcns.c $(LISTCOM)/process.c\
	$(LISTCOM)/symfcns.c $(LISTCOM)/findname.c 
	$(LISTCOM)/parsefcns.c $(LISTCOM)/srciofcns.c $(LISTCOM)/fcns.c\
	$(INCCOM)/filehdr.h $(INCCOM)/ldfcn.h $(LISTCOM)/defs.h\
	$(INCI286)/sgs.h $(INCI286)/paths.h $(INCCOM)/syms.h\
	$(INCCOM)/storclass.h $(LISTCOM)/findname.h $(INCCOM)/linenum.h
	$(LINT) $(LINTFLAGS) $(LISTCOM)/*.c
