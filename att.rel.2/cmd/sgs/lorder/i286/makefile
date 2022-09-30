#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)ccs-s5:lorder/i286/makefile	1.3"
#	i286 lorder makefile
#
#
SGS	= i286
SGSBASE	= ../..
MODE	= 0755
OWN	= bin
GRP	= bin
INCI286 = $(SGSBASE)/inc/i286
BINDIR	= $(ROOT)/bin
LORDERCOM = $(SGSBASE)/lorder/common

build:	lorder

lorder:	$(LORDERCOM)/lorder
		-rm -f lorder
		/bin/sed -n -e "/BINDIR/s/.*\"\(.*\)\".*/\1/p" $(INCI286)/paths.h > ed.tmp
		/bin/sed -e "s%SGS%`cat ed.tmp`/$(SGS)%" < $(LORDERCOM)/lorder > lorder
		rm -f ed.tmp

install: lorder
	sh $(SGSBASE)/sgs.install $(MODE) $(OWN) $(GRP) $(BINDIR)/$(SGS)lorder lorder

save:		$(BINDIR)/$(SGS)lorder
		-rm -f $(BINDIR)/$(SGS)lorder.back
		cp $(BINDIR)/$(SGS)lorder $(BINDIR)/$(SGS)lorder.back

uninstall:	$(BINDIR)/$(SGS)lorder.back
		-rm -f $(BINDIR)/$(SGS)lorder
		cp $(BINDIR)/$(SGS)lorder.back $(BINDIR)/$(SGS)lorder

clean:
clobber: clean
	-rm -f lorder
shrink: clean clobber
