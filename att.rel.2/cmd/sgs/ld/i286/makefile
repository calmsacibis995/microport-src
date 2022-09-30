#	Copyright (c) 1985 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#/*   @(#)makefile	1.3 - 85/08/09 */
#	iAPX LD MAKEFILE
#
#
# Specify the machine architecture on which ld runs:
#
#	AR16WR = DEC 11/45-70
#	AR32WR = VAX 11/780
#	AR32W  = 3B20, IBM370, ...
#
# Specify the version of UN*X beng used:
#
#	TS  :	BTL UN*X TS
#
ARCH	= AR32W
SYS	=
SGS	= i286
VRS	=
ARFORMAT= PORTAR
NATIVE	=
FLEX	= -DFLEXNAMES
MODE	= 0755
OWN	= bin
GRP	= bin
#
#
# Specify directories:
#
SGSBASE	= ../..
BINDIR	= $(ROOT)/bin
LIBDIR	= $(ROOT)/lib
LIBLD	= -lld
LDCOM	= $(SGSBASE)/ld/common
INCCOM	= $(SGSBASE)/inc/common
INCIAPX	= $(SGSBASE)/inc/i286
#
#
# Specify language and/or processor flags:
#
STRIP	= strip
CC  	= cc
LEX 	= lex
LINT	= lint
YACC	= yacc
YFLAGS	= -d
MODEL	=
MSPEC	=
CFLAGS	= -O $(MSPEC)
FRC	=
DEFLIST	= -D$(ARCH) -D$(ARFORMAT) $(FLEX) -DiAPX286 $(NATIVE)
INCLIST = -I. -I$(LDCOM) -I$(INCIAPX) -I$(INCCOM)
CC_CMD	= $(CC) -c $(CFLAGS) $(DEFLIST) $(INCLIST)
LIBES	= -ll -ly 
LIBS	=
LINTFLAGS = -abhx

#
#
# Specify the *.c files making up ld:
#
CFILES	= $(LDCOM)/alloc.c $(LDCOM)/expr0.c $(LDCOM)/expr1.c\
	$(LDCOM)/globs.c $(LDCOM)/instr.c\
	$(LDCOM)/ld00.c $(LDCOM)/ld01.c $(LDCOM)/ld1.c $(LDCOM)/ld2.c\
	$(LDCOM)/lists.c $(LDCOM)/output.c $(LDCOM)/syms.c\
	$(LDCOM)/slotvec.c $(LDCOM)/tv.c $(LDCOM)/util.c\
	$(LDCOM)/version.c\
	lex.yy.c maps.c y.tab.c sglobs.c special1.c\
	initvars.c special2.c specsyms.c $(LDCOM)/regions.c
#
#
# Specify the *.h files making up ld:
#
HFILES1	= y.tab.h params.h system.h\
	$(LDCOM)/attributes.h $(LDCOM)/list.h\
	$(LDCOM)/structs.h $(LDCOM)/extrns.h $(LDCOM)/ldtv.h\
	$(INCCOM)/aouthdr.h $(LDCOM)/slotvec.h \
	system.h $(LDCOM)/instr.h
HFILES2 = $(INCCOM)/filehdr.h $(INCCOM)/scnhdr.h $(INCCOM)/syms.h\
	$(INCCOM)/storclass.h $(INCCOM)/bool.h\
	$(INCCOM)/sgsmacros.h $(INCCOM)/ldfcn.h $(INCCOM)/reloc.h\
	$(INCCOM)/linenum.h $(INCCOM)/patch.h
HFILES3 = $(INCIAPX)/sgs.h $(INCIAPX)/paths.h $(INCIAPX)/tv.h
#
#
# Specify the *.o files making up ld:
#
O	= o
#
OBJECTS= expr0.$O globs.$O sglobs.$O ld00.$O lex.yy.$O lists.$O\
	initvars.$O util.$O version.$O y.tab.$O special1.$O \
	alloc.$O specsyms.$O expr1.$O globs.$O sglobs.$O \
	ld01.$O ld1.$O ld2.$O lists.$O maps.$O output.$O \
	slotvec.$O syms.$O tv.$O  \
	instr.$O special2.$O regions.$O

#
#
# Make ld:
#

all:		ld

build:		ld

ld:		$(OBJECTS)
	$(CC) -i -s -o ld $(CFLAGS) $(OBJECTS) $(LIBES) $(LIBLD) $(LIBS)


#
#
# Make the *.c:
#
y.tab.c y.tab.h:	$(LDCOM)/ld.yac $(FRC)
	@echo "Expect to see message 'conflicts: 6 shift/reduce'";
	$(YACC) $(YFLAGS) $(LDCOM)/ld.yac

y.tab.$O:	$(LDCOM)/attributes.h $(LDCOM)/list.h $(LDCOM)/structs.h $(FRC)\
		$(LDCOM)/extrns.h $(LDCOM)/ldtv.h system.h\
		$(INCCOM)/filehdr.h $(INCCOM)/scnhdr.h $(INCCOM)/syms.h\
		$(INCCOM)/storclass.h $(INCCOM)/bool.h $(INCCOM)/sgsmacros.h\
		$(INCIAPX)/sgs.h $(INCIAPX)/tv.h $(LDCOM)/ldmacros.h\
		params.h\
		y.tab.c
	$(CC_CMD) y.tab.c

lex.yy.c:	$(LDCOM)/ld.lex $(FRC)
	$(LEX) $(LDCOM)/ld.lex

lex.yy.$O:	$(LDCOM)/structs.h $(LDCOM)/extrns.h system.h $(FRC)\
		$(LDCOM)/ldtv.h\
		$(LDCOM)/instr.h $(INCCOM)/aouthdr.h\
		$(INCCOM)/filehdr.h $(INCCOM)/scnhdr.h $(INCCOM)/syms.h\
		$(INCCOM)/storclass.h $(INCCOM)/bool.h\
		$(INCIAPX)/paths.h $(INCIAPX)/sgs.h $(INCIAPX)/tv.h\
		y.tab.h\
		lex.yy.c
	$(CC_CMD) lex.yy.c

expr0.$O:	$(LDCOM)/extrns.h $(LDCOM)/structs.h system.h $(FRC)\
		$(INCCOM)/filehdr.h $(INCCOM)/scnhdr.h $(INCCOM)/syms.h\
		$(INCCOM)/storclass.h $(INCCOM)/sgsmacros.h\
		y.tab.h $(LDCOM)/ldmacros.h\
		$(LDCOM)/expr0.c
	$(CC_CMD) $(LDCOM)/expr0.c

expr1.$O:	$(LDCOM)/extrns.h $(LDCOM)/structs.h system.h $(FRC)\
		$(INCCOM)/filehdr.h $(INCCOM)/scnhdr.h $(INCCOM)/syms.h\
		$(INCCOM)/storclass.h $(INCCOM)/sgsmacros.h\
		y.tab.h $(LDCOM)/ldmacros.h\
		$(LDCOM)/expr1.c
	$(CC_CMD) $(LDCOM)/expr1.c

ld00.$O:	$(LDCOM)/extrns.h $(LDCOM)/list.h $(LDCOM)/structs.h $(FRC)\
		system.h $(INCCOM)/ldfcn.h\
		$(INCCOM)/filehdr.h $(INCCOM)/scnhdr.h $(INCCOM)/syms.h\
		$(INCCOM)/storclass.h $(INCCOM)/sgsmacros.h\
		$(INCIAPX)/paths.h $(INCIAPX)/sgs.h\
		params.h y.tab.h\
		$(LDCOM)/ld00.c
	$(CC_CMD) $(LDCOM)/ld00.c

ld01.$O:	$(LDCOM)/extrns.h $(LDCOM)/structs.h $(LDCOM)/list.h $(FRC)\
		system.h $(LDCOM)/ldtv.h $(LDCOM)/instr.h\
		$(INCCOM)/filehdr.h $(INCCOM)/scnhdr.h $(INCCOM)/syms.h\
		$(INCCOM)/storclass.h $(INCCOM)/sgsmacros.h\
		$(INCIAPX)/tv.h $(LDCOM)/ldmacros.h\
		$(LDCOM)/ld01.c
	$(CC_CMD) $(LDCOM)/ld01.c

ld1.$O:		$(LDCOM)/extrns.h $(LDCOM)/list.h $(LDCOM)/ldtv.h $(FRC)\
		$(LDCOM)/structs.h system.h\
		$(INCCOM)/filehdr.h $(INCCOM)/scnhdr.h $(INCCOM)/syms.h\
		$(INCCOM)/storclass.h $(INCCOM)/ldfcn.h $(INCCOM)/reloc.h\
		$(INCCOM)/sgsmacros.h \
		$(INCIAPX)/sgs.h $(INCIAPX)/tv.h                \
		params.h $(LDCOM)/ldmacros.h\
		$(LDCOM)/instr.h \
		$(LDCOM)/ld1.c
	$(CC_CMD) $(LDCOM)/ld1.c

ld2.$O:		$(LDCOM)/ldtv.h $(LDCOM)/extrns.h $(LDCOM)/list.h $(FRC)\
		$(LDCOM)/structs.h system.h\
		$(INCCOM)/filehdr.h $(INCCOM)/scnhdr.h $(INCCOM)/syms.h\
		$(INCCOM)/sgsmacros.h $(INCCOM)/storclass.h\
		$(INCCOM)/reloc.h $(INCCOM)/linenum.h $(INCCOM)/patch.h\
		$(INCIAPX)/tv.h\
		$(LDCOM)/instr.h \
		y.tab.h params.h\
		$(LDCOM)/ld2.c
	$(CC_CMD) $(LDCOM)/ld2.c

alloc.$O:	$(LDCOM)/extrns.h $(LDCOM)/list.h $(LDCOM)/structs.h $(FRC)\
		system.h $(LDCOM)/ldtv.h\
		$(INCCOM)/filehdr.h $(INCCOM)/scnhdr.h $(INCCOM)/syms.h\
		$(INCCOM)/storclass.h $(INCCOM)/sgsmacros.h\
		$(LDCOM)/ldmacros.h\
		$(INCIAPX)/tv.h $(INCIAPX)/sgs.h\
		params.h\
		$(LDCOM)/alloc.c
	$(CC_CMD) $(LDCOM)/alloc.c

output.$O:	$(LDCOM)/structs.h $(LDCOM)/extrns.h system.h $(FRC)\
		$(LDCOM)/ldtv.h $(LDCOM)/slotvec.h $(INCCOM)/aouthdr.h\
		$(LDCOM)/instr.h\
		$(INCCOM)/filehdr.h $(INCCOM)/scnhdr.h $(INCCOM)/syms.h\
		$(INCCOM)/sgsmacros.h $(INCCOM)/storclass.h\
		$(INCCOM)/reloc.h $(INCCOM)/linenum.h $(INCCOM)/ldfcn.h\
		$(INCCOM)/patch.h\
		$(INCIAPX)/sgs.h $(INCIAPX)/tv.h params.h\
		$(LDCOM)/ldmacros.h $(LDCOM)/output.c
	$(CC_CMD) $(LDCOM)/output.c

maps.$O:	$(LDCOM)/extrns.h $(LDCOM)/structs.h $(LDCOM)/attributes.h $(FRC)\
		$(INCIAPX)/sgs.h system.h\
		$(INCCOM)/filehdr.h $(INCCOM)/scnhdr.h $(INCCOM)/syms.h\
		$(INCCOM)/storclass.h $(INCCOM)/sgsmacros.h\
		maps.c
	$(CC_CMD) maps.c

util.$O:	$(LDCOM)/structs.h $(LDCOM)/extrns.h system.h $(FRC)\
		$(INCCOM)/filehdr.h $(INCCOM)/scnhdr.h $(INCCOM)/syms.h\
		$(INCCOM)/storclass.h $(INCCOM)/sgsmacros.h\
		$(INCCOM)/reloc.h $(LDCOM)/slotvec.h $(INCIAPX)/sgs.h\
		y.tab.h \
		$(LDCOM)/util.c
	$(CC_CMD) $(LDCOM)/util.c

lists.$O: 	$(LDCOM)/list.h $(LDCOM)/structs.h system.h $(FRC)\
		$(INCCOM)/filehdr.h $(INCCOM)/scnhdr.h $(INCCOM)/syms.h\
		$(INCCOM)/sgsmacros.h $(INCCOM)/storclass.h\
		$(LDCOM)/lists.c
	$(CC_CMD) $(LDCOM)/lists.c

syms.$O:	$(LDCOM)/structs.h $(LDCOM)/extrns.h $(LDCOM)/ldtv.h $(FRC)\
		system.h $(LDCOM)/instr.h\
		$(INCCOM)/filehdr.h $(INCCOM)/scnhdr.h $(INCCOM)/syms.h\
		$(INCCOM)/storclass.h $(INCCOM)/sgsmacros.h\
		$(INCIAPX)/paths.h $(INCIAPX)/tv.h\
		$(LDCOM)/ldmacros.h $(LDCOM)/syms.c
	$(CC_CMD) $(LDCOM)/syms.c

tv.$O:		$(LDCOM)/extrns.h $(LDCOM)/structs.h $(LDCOM)/ldtv.h $(FRC)\
		system.h $(LDCOM)/list.h\
		$(INCCOM)/filehdr.h $(INCCOM)/scnhdr.h $(INCCOM)/syms.h\
		$(INCCOM)/bool.h \
		$(INCCOM)/storclass.h $(INCCOM)/reloc.h \
		$(INCCOM)/ldfcn.h $(INCCOM)/sgsmacros.h\
		$(INCIAPX)/tv.h $(LDCOM)/ldmacros.h\
		$(LDCOM)/tv.c
	$(CC_CMD) $(LDCOM)/tv.c

globs.$O:	$(LDCOM)/structs.h $(LDCOM)/ldtv.h system.h $(FRC)\
		$(INCCOM)/filehdr.h $(INCCOM)/scnhdr.h $(INCCOM)/syms.h\
		$(INCCOM)/storclass.h\
		$(INCIAPX)/paths.h $(INCIAPX)/tv.h params.h\
		$(LDCOM)/globs.c
	$(CC_CMD) $(LDCOM)/globs.c

instr.$O:	$(LDCOM)/instr.c $(FRC)
	$(CC_CMD) $(LDCOM)/instr.c

slotvec.$O:	$(LDCOM)/slotvec.h $(LDCOM)/structs.h $(LDCOM)/extrns.h $(FRC)\
		system.h\
		$(INCCOM)/filehdr.h $(INCCOM)/scnhdr.h $(INCCOM)/syms.h\
		$(INCCOM)/storclass.h \
       		$(INCIAPX)/paths.h\
		$(LDCOM)/slotvec.c
	$(CC_CMD) $(LDCOM)/slotvec.c

version.$O:	system.h $(FRC)\
		$(LDCOM)/version.c
	$(CC_CMD) $(LDCOM)/version.c

sglobs.$O:	$(LDCOM)/structs.h $(INCCOM)/filehdr.h  $(FRC)\
		$(INCCOM)/scnhdr.h params.h \
		$(INCCOM)/syms.h $(INCCOM)/storclass.h \
		$(INCIAPX)/tv.h $(LDCOM)/ldtv.h system.h sglobs.c
	$(CC_CMD) sglobs.c

special1.$O:	system.h special1.c params.h $(FRC)
	$(CC_CMD) special1.c

special2.$O:	system.h $(INCCOM)/aouthdr.h $(LDCOM)/structs.h  $(FRC)\
		$(INCCOM)/filehdr.h $(INCIAPX)/sgs.h\
		$(INCCOM)/scnhdr.h $(INCCOM)/syms.h \
		$(INCCOM)/storclass.h $(INCCOM)/ldfcn.h\
		$(INCIAPX)/tv.h $(LDCOM)/ldtv.h $(LDCOM)/slotvec.h\
		$(INCCOM)/reloc.h $(LDCOM)/extrns.h\
		$(INCCOM)/sgsmacros.h params.h $(LDCOM)/list.h\
		special2.c
	$(CC_CMD) special2.c

initvars.$O:	system.h params.h $(LDCOM)/structs.h $(LDCOM)/extrns.h $(FRC)
	$(CC_CMD) initvars.c

specsyms.$O:	system.h $(LDCOM)/extrns.h $(INCCOM)/sgsmacros.h $(FRC)\
		$(LDCOM)/structs.h $(INCCOM)/filehdr.h \
		$(INCCOM)/scnhdr.h $(INCIAPX)/sgs.h \
		$(INCCOM)/syms.h $(INCCOM)/storclass.h\
		$(INCIAPX)/tv.h $(LDCOM)/ldtv.h\
		specsyms.c
	$(CC_CMD) specsyms.c

regions.$O:	system.h $(LDCOM)/structs.h $(INCCOM)/filehdr.h $(FRC)\
		$(INCCOM)/scnhdr.h\
		$(INCCOM)/syms.h $(INCCOM)/storclass.h \
		$(LDCOM)/extrns.h $(LDCOM)/list.h \
		$(INCCOM)/sgsmacros.h params.h $(LDCOM)/regions.c
	$(CC_CMD) $(LDCOM)/regions.c

#
#
# File and administrative commands:
#
save:	$(BINDIR)/$(SGS)ld
	-rm -f $(BINDIR)/$(SGS)ld.back
	cp $(BINDIR)/$(SGS)ld   $(BINDIR)/$(SGS)ld.back

install: ld
	sh $(SGSBASE)/sgs.install $(MODE) $(OWN) $(GRP) $(BINDIR)/$(SGS)ld ld
	$(STRIP) ld

uninstall:	$(BINDIR)/$(SGS)ld.back
	-rm -f $(BINDIR)/$(SGS)ld
	cp  $(BINDIR)/$(SGS)ld.back    $(BINDIR)/$(SGS)ld

clean:
	-rm -f $(OBJECTS) y.tab.c lex.yy.c y.tab.h

clobber: clean
	-rm -f ld

shrink: clean clobber
#
# Run lint on the *.c files:
#
lint:	$(CFILES) $(HFILES1) $(HFILES2) $(HFILES3) 
	$(LINT) $(LINTFLAGS) $(INCLIST) $(CFILES)
# Force recompilation
FRC:
