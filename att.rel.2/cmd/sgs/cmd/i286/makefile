#	Copyright (c) 1985 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#/*   @(#)makefile	1.3 - 85/08/09 */
#
#
CC	= cc
CC.C	= ../common/cc.c
SGS	= i286
SGSBASE = ../..
MODE	= 0755
OWN	= bin
GRP	= bin
LINT	= lint
STRIP	= strip
MSPEC	=
MODEL	=
CFLAGS	= -O $(MSPEC)
DEFLIST	= $(PASS) $(MOD)
INCI286	= ../../inc/i286
# define MODEL to "-DiAPX286"
PASS	= -DTWOPASS
MOD	= -DiAPX286
BINDIR	= $(ROOT)/bin

build:	cc

cc:	$(INCI286)/paths.h $(INCI286)/sgs.h $(CC.C)
	rm -f cc
	$(CC) $(CFLAGS) -I$(INCI286) $(DEFLIST) $(CC.C) -o cc

save:	$(BINDIR)/$(SGS)cc
	cp $(BINDIR)/$(SGS)cc $(BINDIR)/$(SGS)cc.back

install: cc ../common/gencc
	$(STRIP) cc
	sh $(SGSBASE)/sgs.install $(MODE) $(OWN) $(GRP) $(BINDIR)/$(SGS)cc cc
	sh $(SGSBASE)/sgs.install $(MODE) $(OWN) $(GRP) $(BINDIR)/gencc ../common/gencc

uninstall:	$(BINDIR)/$(SGS)cc.back
	cp $(BINDIR)/$(SGS)cc.back $(BINDIR)/$(SGS)cc

clean:	
	-rm -f cc.o
clobber: clean
	-rm -f cc 
shrink: clean clobber

strip:	$(SGS)cc
	strip $(SGS)cc

lint:	$(INCI286)/paths.h $(INCI286)/sgs.h $(CC.C)
	$(LINT) -I$(INCI286) $(PASS) $(CC.C)
