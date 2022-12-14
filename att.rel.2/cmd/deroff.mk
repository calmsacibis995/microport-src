#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#	deroff make file
#	@(#)	1.1	#
#	NOTE: This makefile is only necessary because of the need for the
#		base register option for cc on u370.

ROOT =
OL = $(ROOT)/
SL = $(ROOT)/usr/src/cmd
RDIR = $(SL)
INS = :
REL = current
CSID = -r`gsid deroff $(REL)`
MKSID = -r`gsid deroff.mk $(REL)`
LIST = lp
INSDIR = $(OL)usr/bin
IFLAG = -n
B20 =
CFLAGS = -O $(B20)
LDFLAGS = -s $(IFLAG)
SOURCE = deroff.c
MAKE = make

compile all: deroff
	:

deroff:
	$(CC) $(CFLAGS) $(LDFLAGS) -o deroff deroff.c
	$(INS) deroff $(INSDIR)
	$(CH) chmod 775 $(INSDIR)/deroff
	@if [ "$(OL)" = "/" ]; \
		then cd $(INSDIR); chown bin deroff; chgrp bin deroff; \
	 fi

install:
	$(MAKE) -f deroff.mk INS=cp OL=$(OL)

build:	bldmk
	get -p $(CSID) s.deroff.c $(REWIRE) > $(RDIR)/deroff.c
bldmk:  ;  get -p $(MKSID) s.deroff.mk > $(RDIR)/deroff.mk

listing:
	pr deroff.mk $(SOURCE) | $(LIST)
listmk: ;  pr deroff.mk | $(LIST)

edit:
	get -e s.deroff.c

delta:
	delta s.deroff.c

mkedit:  ;  get -e s.deroff.mk
mkdelta: ;  delta s.deroff.mk

clean:
	:

clobber:  clean
	  rm -f deroff

delete:	clobber
	rm -f $(SOURCE)
