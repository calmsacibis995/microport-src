#	Copyright (c) 1985 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#/*   @(#)makefile	1.3 - 85/08/09 */
#
#
SGSBASE = ../..
AR	= ar
BM	= make
SGS	= i286
OWN	= bin
GRP	= bin
LINT	= lint
INCCOM	= $(SGSBASE)/inc/common
BINDIR	= $(ROOT)/bin
LIBDIR	= $(ROOT)/lib
LIBLDCOM = $(SGSBASE)/libld/common
ARFORMAT = PORTAR
ARCH	= AR16WR
CC	= $(BINDIR)/$(SGS)cc
LIB	= $(LIBDIR)
MODEL	=
MSPEC	=
CFLAGS  = -O

build:
	cd $(LIBLDCOM); $(BM) CC=$(CC) CFLAGS="$(CFLAGS) $(MSPEC)"  INCCOM=$(INCCOM) \
	ARFORMAT=$(ARFORMAT) ARCH=$(ARCH)

install:      build
	sh $(SGSBASE)/sgs.install 644 $(OWN) $(GRP) $(LIB)/$(MODEL)/libld.a $(LIBLDCOM)/libld.a
	$(AR) ts $(LIB)/$(MODEL)/libld.a

save:	$(LIB)/$(MODEL)/libld.a
	-rm -f $(LIB)/$(MODEL)/libld.a.back
	cp $(LIB)/$(MODEL)/libld.a $(LIB)/$(MODEL)/libld.a.back

uninstall:	$(LIB)/$(MODEL)/libld.a.back
	-rm -f $(LIB)/$(MODEL)/libld.a
	cp $(LIB)/$(MODEL)/libld.a.back $(LIB)/$(MODEL)/libld.a

shrink:
	@echo "shrink not done because other SGSs may use same object"

lint:
	cd $(LIBLDCOM); $(BM) LINT=$(LINT) lint
