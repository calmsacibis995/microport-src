#	Copyright (c) 1985 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#/*   @(#)makefile	1.3 - 85/08/08 */
#	i286 assembler makefile
#
#
# Architectures:
#
#	AR16WR = DEC 11/45-70
#	AR32WR = VAX 11/780
#	AR32W  = 3B family, IBM 370
#

BM	= make
ARCH	= AR32WR
SGS	= i286
SGSBASE	= ../..
ROOT	=
NPROC	= ONEPROC
FLEX	= -DFLEXNAMES
FFLAG	=

MODE	= 0755
GRP	= bin
OWN	= bin

STRIP	= strip
MODEL	=
MSPEC	=
NATIVE	=
CC	= cc
CFLAGS	= -O $(MSPEC) 
DEFLIST	= -D$(NPROC) -DiAPX286 -D$(ARCH) $(FLEX) $(NATIVE)
LIBM	= -lm
LINT	= lint
LDFLAGS	= $(MSPEC)
LIBS	=
LIBES	=

BINDIR	= $(ROOT)/bin
LIBDIR	= $(ROOT)/lib
INCIAPX	= $(SGSBASE)/inc/i286
INCCOM	= $(SGSBASE)/inc/common
ASCOM	= $(SGSBASE)/as/common
YACCDIR = $(SGSBASE)/as/yacc

SPECYACC= $(YACCDIR)/yacc
YFLAGS	= -d

OBJECTS	= pass0.o instab.o parse.o gencode.o pass1.o symbols.o \
	code.o expand1.o expand2.o errors.o addr2.o float.o \
	pass2.o addr1.o codeout.o getstab.o obj.o symlist.o \
	unused.o

CFILES	= $(ASCOM)/pass0.c \
	parse.c $(ASCOM)/code.c $(ASCOM)/errors.c gencode.c $(ASCOM)/instab.c \
	$(ASCOM)/pass1.c $(ASCOM)/symbols.c \
	$(ASCOM)/expand1.c expand2.c float.c \
	$(ASCOM)/addr1.c addr2.c $(ASCOM)/codeout.c \
	$(ASCOM)/getstab.c $(ASCOM)/pass2.c $(ASCOM)/obj.c \
	$(ASCOM)/symlist.c $(ASCOM)/unused.c

HFILES	= $(ASCOM)/codeout.h gendefs.h instab.h \
	ops.out $(ASCOM)/symbols.h \
	$(INCIAPX)/paths.h $(ASCOM)/expand.h expand2.h parse.h \
	$(INCCOM)/reloc.h $(INCCOM)/syms.h $(INCCOM)/storclass.h\
	$(INCCOM)/filehdr.h $(INCCOM)/scnhdr.h $(INCCOM)/linenum.h \
	$(INCIAPX)/sgs.h

COMAS	= $(ASCOM)/codeout.h $(ASCOM)/expand.h $(INCCOM)/filehdr.h $(INCCOM)/linenum.h \
	$(INCCOM)/reloc.h $(INCCOM)/scnhdr.h $(INCCOM)/storclass.h $(ASCOM)/symbols.h \
	$(INCCOM)/syms.h $(ASCOM)/addr1.c $(ASCOM)/code.c \
	$(ASCOM)/codeout.c $(ASCOM)/errors.c $(ASCOM)/expand1.c $(ASCOM)/getstab.c \
	$(ASCOM)/obj.c $(ASCOM)/pass0.c $(ASCOM)/pass1.c $(ASCOM)/pass2.c \
	$(ASCOM)/symbols.c $(ASCOM)/symlist.c \
	$(ASCOM)/unused.c

DEPAS	= expand2.h gendefs.h instab.h parse.h \
	addr2.c expand2.c float.c \
	gencode.c parse.y ops.out makefile 


#	ASSEMBLER
build: as

as		: $(OBJECTS)
		$(CC) $(FFLAG) $(LDFLAGS) -o as $(OBJECTS) $(LIBS) $(LIBES) $(LIBM)

pass0.o		: $(ASCOM)/pass0.c pass0.h $(INCIAPX)/paths.h \
		  $(INCIAPX)/sgs.h systems.h
		$(CC) -c $(CFLAGS) $(DEFLIST) -I. -I$(INCIAPX) $(ASCOM)/pass0.c


parse.o		: parse.c $(ASCOM)/symbols.h $(INCCOM)/filehdr.h \
		  instab.h gendefs.h $(INCCOM)/storclass.h \
		  $(INCIAPX)/sgs.h systems.h
		$(CC) -c $(CFLAGS) $(DEFLIST) -I. -I$(INCCOM) -I$(INCIAPX) -I$(ASCOM) -DYYDEBUG=0 parse.c

parse.c parse.h	: parse.y $(SPECYACC)
		$(SPECYACC) $(YFLAGS) parse.y
		mv y.tab.c parse.c
		mv y.tab.h parse.h

$(SPECYACC)	: 
# parse.y has an inordinate number of terminal symbols
		(cd $(YACCDIR); \
		/bin/make -f yacc.mk)

code.o		: $(ASCOM)/code.c $(ASCOM)/symbols.h $(ASCOM)/codeout.h gendefs.h systems.h
		$(CC) -c $(CFLAGS) $(DEFLIST) -I. -I$(ASCOM) $(ASCOM)/code.c

errors.o	: $(ASCOM)/errors.c gendefs.h systems.h
		$(CC) -c $(CFLAGS) $(DEFLIST) -I. $(ASCOM)/errors.c

pass1.o		: $(ASCOM)/pass1.c $(INCIAPX)/paths.h $(ASCOM)/symbols.h gendefs.h systems.h
		$(CC) -c $(CFLAGS) $(DEFLIST) -I. -I$(INCIAPX) $(ASCOM)/pass1.c

instab.o	: $(ASCOM)/instab.c instab.h ops.out \
		  $(ASCOM)/symbols.h parse.h systems.h
		$(CC) -c $(CFLAGS) $(DEFLIST) -I. -I$(ASCOM) $(ASCOM)/instab.c

gencode.o	: gencode.c $(ASCOM)/symbols.h instab.h \
		  parse.h gendefs.h $(ASCOM)/expand.h expand2.h systems.h
		$(CC) -c $(CFLAGS) $(DEFLIST) -DUNIX -I. -I$(ASCOM) gencode.c

expand1.o	: $(ASCOM)/expand1.c $(ASCOM)/expand.h $(ASCOM)/symbols.h gendefs.h systems.h
		$(CC) -c $(CFLAGS) $(DEFLIST) -I. -I$(ASCOM) $(ASCOM)/expand1.c

expand2.o	: expand2.c $(ASCOM)/expand.h expand2.h $(ASCOM)/symbols.h systems.h
		$(CC) -c $(CFLAGS) $(DEFLIST) -I. -I$(ASCOM) expand2.c

float.o		: float.c instab.h $(ASCOM)/symbols.h systems.h
		$(CC) -c $(CFLAGS) $(DEFLIST) -I. -I$(ASCOM) float.c

addr1.o		: $(ASCOM)/addr1.c $(INCCOM)/reloc.h $(INCCOM)/syms.h\
		  $(INCCOM)/storclass.h $(INCCOM)/linenum.h $(INCCOM)/filehdr.h\
		  gendefs.h $(ASCOM)/symbols.h $(ASCOM)/codeout.h systems.h
		$(CC) -c $(CFLAGS) $(DEFLIST) -I. -I$(ASCOM) -I$(INCCOM) $(ASCOM)/addr1.c

addr2.o		: addr2.c $(INCCOM)/reloc.h $(INCCOM)/storclass.h \
		  $(INCCOM)/syms.h gendefs.h $(ASCOM)/symbols.h\
		  $(ASCOM)/codeout.h instab.h systems.h
		$(CC) -c $(CFLAGS) $(DEFLIST) -I. -I$(ASCOM) -I$(INCCOM) addr2.c

codeout.o	: $(ASCOM)/codeout.c $(ASCOM)/symbols.h $(ASCOM)/codeout.h gendefs.h systems.h
		$(CC) -c $(CFLAGS) $(DEFLIST) -I. -I$(ASCOM) $(ASCOM)/codeout.c

getstab.o	: $(ASCOM)/getstab.c gendefs.h $(ASCOM)/symbols.h systems.h
		$(CC) -c $(CFLAGS) $(DEFLIST) -I. -I$(ASCOM) $(ASCOM)/getstab.c

pass2.o		: $(ASCOM)/pass2.c gendefs.h $(ASCOM)/symbols.h systems.h
		$(CC) -c $(CFLAGS) $(DEFLIST) -I. $(ASCOM)/pass2.c

obj.o		: $(ASCOM)/obj.c $(INCCOM)/filehdr.h $(INCCOM)/linenum.h instab.h\
		$(INCCOM)/reloc.h $(INCCOM)/scnhdr.h $(INCCOM)/syms.h $(INCCOM)/storclass.h\
		$(ASCOM)/symbols.h $(ASCOM)/codeout.h gendefs.h $(INCIAPX)/sgs.h systems.h
		$(CC) -c $(CFLAGS) $(DEFLIST) -I. -I$(INCCOM) -I$(INCIAPX) $(ASCOM)/obj.c

symlist.o	: $(ASCOM)/symlist.c $(ASCOM)/symbols.h $(INCCOM)/syms.h \
		  $(INCCOM)/storclass.h gendefs.h systems.h
		$(CC) -c $(CFLAGS) $(DEFLIST) -I. -I$(INCCOM) $(ASCOM)/symlist.c

symbols.o	: $(ASCOM)/symbols.c $(ASCOM)/symbols.h symbols2.h systems.h
		$(CC) -c $(CFLAGS) $(DEFLIST) -I. $(ASCOM)/symbols.c

unused.o	: $(ASCOM)/unused.c symbols2.h $(INCCOM)/filehdr.h \
		  $(INCCOM)/linenum.h $(INCCOM)/reloc.h $(INCCOM)/scnhdr.h \
		  $(INCCOM)/syms.h $(INCCOM)/storclass.h
		$(CC) -c $(CFLAGS) $(DEFLIST) -I. -I$(INCCOM) $(ASCOM)/unused.c

lint.out	: $(CFILES) $(HFILES) 
		$(LINT) $(LINTFLAGS) $(DEFLIST) \
			-I$(INCCOM) -I$(ASCOM) -I$(INCIAPX) $(CFILES) >lint.out
		wc -l lint.out

install 	: as
		$(STRIP) as
		sh $(SGSBASE)/sgs.install $(MODE) $(OWN) $(GRP) $(BINDIR)/$(SGS)as as

save		: $(BINDIR)/$(SGS)as
		-rm -f $(BINDIR)/$(SGS)as.back
		cp $(BINDIR)/$(SGS)as  $(BINDIR)/$(SGS)as.back

uninstall	: $(BINDIR)/$(SGS)as.back
		-rm -f $(BINDIR)/$(SGS)as
		cp $(BINDIR)/$(SGS)as.back  $(BINDIR)/$(SGS)as

shrink:		clean clobber

clean:
		-rm -f $(OBJECTS) parse.c parse.h y.output lint.out

clobber:	clean
		-rm -f as 

listall:	$(CFILES) $(HFILES)
		pr -f $(HFILES) $(CFILES) | lp

listcom:	$(COMAS)
		pr -f $(COMAS) | lp

listdep:	 $(DEPAS)
		pr -f $(DEPAS) | lp
