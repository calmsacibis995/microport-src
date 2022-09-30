#	Copyright (c) 1985 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#/*   @(#)makefile	1.3 - 85/08/09 */
SGSBASE	= ../..
HDMODE	= 644
OWN	= bin
GRP	= bin

USRINC	= $(ROOT)/usr/include
INCCOM	= $(SGSBASE)/inc/common

HEADERS = $(USRINC)/aouthdr.h $(USRINC)/ar.h $(USRINC)/filehdr.h \
	$(USRINC)/ldfcn.h $(USRINC)/linenum.h $(USRINC)/patch.h \
	$(USRINC)/sgsmacros.h $(USRINC)/reloc.h $(USRINC)/scnhdr.h\
	$(USRINC)/storclass.h\
	$(USRINC)/syms.h $(USRINC)/paths.h\
	$(USRINC)/sgs.h $(USRINC)/tv.h

build: 

install: reg

reg:	$(HEADERS)

# make things a little easier to read
D	= $(USRINC)
C	= $(INCCOM)

$D/aouthdr.h: $C/aouthdr.h
	sh $(SGSBASE)/sgs.install $(HDMODE) $(OWN) $(GRP) $D/aouthdr.h $C/aouthdr.h

$D/ar.h: $C/ar.h
	sh $(SGSBASE)/sgs.install $(HDMODE) $(OWN) $(GRP) $D/ar.h $C/ar.h

$D/filehdr.h: $C/filehdr.h
	sh $(SGSBASE)/sgs.install $(HDMODE) $(OWN) $(GRP) $D/filehdr.h $C/filehdr.h

$D/ldfcn.h: $C/ldfcn.h
	sh $(SGSBASE)/sgs.install $(HDMODE) $(OWN) $(GRP) $D/ldfcn.h $C/ldfcn.h

$D/patch.h: $C/patch.h
	sh $(SGSBASE)/sgs.install $(HDMODE) $(OWN) $(GRP) $D/patch.h $C/patch.h

$D/scnhdr.h: $C/scnhdr.h
	sh $(SGSBASE)/sgs.install $(HDMODE) $(OWN) $(GRP) $D/scnhdr.h $C/scnhdr.h

$D/syms.h: $C/syms.h
	sh $(SGSBASE)/sgs.install $(HDMODE) $(OWN) $(GRP) $D/syms.h $C/syms.h

$D/reloc.h: $C/reloc.h
	sh $(SGSBASE)/sgs.install $(HDMODE) $(OWN) $(GRP) $D/reloc.h $C/reloc.h

$D/linenum.h: $C/linenum.h
	sh $(SGSBASE)/sgs.install $(HDMODE) $(OWN) $(GRP) $D/linenum.h $C/linenum.h

$D/storclass.h: $C/storclass.h
	sh $(SGSBASE)/sgs.install $(HDMODE) $(OWN) $(GRP) $D/storclass.h $C/storclass.h

$D/sgsmacros.h: $C/sgsmacros.h
	sh $(SGSBASE)/sgs.install $(HDMODE) $(OWN) $(GRP) $D/sgsmacros.h $C/sgsmacros.h

$D/sgs.h: sgs.h
	sh $(SGSBASE)/sgs.install $(HDMODE) $(OWN) $(GRP) $D/sgs.h sgs.h

$D/tv.h: tv.h
	sh $(SGSBASE)/sgs.install $(HDMODE) $(OWN) $(GRP) $D/tv.h tv.h

$D/paths.h: paths.h
	sh $(SGSBASE)/sgs.install $(HDMODE) $(OWN) $(GRP) $D/paths.h paths.h


save:	$(HEADERS)
	cp $D/aouthdr.h $D/aouthdr.h.back
	cp $D/ar.h $D/ar.h.back
	cp $D/patch.h $D/patch.h.back
	cp $D/filehdr.h $D/filehdr.h.back
	cp $D/ldfcn.h $D/ldfcn.h.back
	cp $D/scnhdr.h $D/scnhdr.h.back
	cp $D/syms.h $D/syms.h.back
	cp $D/reloc.h $D/reloc.h.back
	cp $D/linenum.h $D/linenum.h.back
	cp $D/storclass.h $D/storclass.h.back
	cp $D/sgs.h $D/sgs.h.back
	cp $D/tv.h $D/tv.h.back
	cp $D/paths.h $D/paths.h.back
	cp $D/sgsmacros.h $D/sgsmacros.h.back

uninstall:	$D/filehdr.h.back $D/ldfcn.h.back $D/linenum.h.back\
	$D/aouthdr.h.back $D/ar.h.back $D/patch.h.back \
	$D/sgsmacros.h.back  $D/paths.h.back\
	$D/reloc.h.back $D/scnhdr.h.back $D/sgs.h.back\
	$D/storclass.h.back $D/syms.h.back $D/tv.h.back\
	-rm -f $D/aouthdr.h
	cp $D/aouthdr.h.back $D/aouthdr.h
	-rm -f $D/ar.h
	cp $D/ar.h.back $D/ar.h
	-rm -f $D/filehdr.h
	cp $D/filehdr.h.back $D/filehdr.h
	-rm -f $D/ldfcn.h
	cp $D/ldfcn.h.back $D/ldfcn.h
	-rm -f $D/patch.h
	cp $D/patch.h.back $D/patch.h
	-rm -f $D/scnhdr.h
	cp $D/scnhdr.h.back $D/scnhdr.h
	-rm -f $D/syms.h
	cp $D/syms.h.back $D/syms.h
	-rm -f $D/reloc.h
	cp $D/reloc.h.back $D/reloc.h
	-rm -f $D/linenum.h
	cp $D/linenum.h.back $D/linenum.h
	-rm -f $D/storclass.h
	cp $D/storclass.h.back $D/storclass.h
	-rm -f $D/sgs.h
	cp $D/sgs.h.back $D/sgs.h
	-rm -f $D/tv.h
	cp $D/tv.h.back $D/tv.h
	-rm -f $D/paths.h
	cp $D/paths.h.back $D/paths.h
	-rm -f $D/sgsmacros.h
	cp $D/sgsmacros.h.back $D/sgsmacros.h

clobber:
