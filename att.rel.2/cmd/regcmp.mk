#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#	@(#)	1.2

ROOT =
OL = $(ROOT)/
SL = $(ROOT)/usr/src/cmd
RDIR = $(SL)
INS = install -f
REL = current
CSID = -r`gsid regcmp $(REL)`
MKSID = -r`gsid regcmp.mk $(REL)`
LIST = lp
INSDIR = $(OL)usr/bin
IFLAG = -n
CFLAGS = -O
LDFLAGS = -s $(IFLAG)
SOURCE = regcmp.c
MAKE = make

compile all: regcmp

regcmp:
	$(CC) $(CFLAGS) $(LDFLAGS) -o regcmp regcmp.c -lPW

install: all
	$(INS) $(INSDIR) regcmp

build:	bldmk
	get -p $(CSID) s.regcmp.c $(REWIRE) > $(RDIR)/regcmp.c
bldmk:  ;  get -p $(MKSID) s.regcmp.mk > $(RDIR)/regcmp.mk

listing:
	pr regcmp.mk $(SOURCE) | $(LIST)
listmk: ;  pr regcmp.mk | $(LIST)

edit:
	get -e s.regcmp.c

delta:
	delta s.regcmp.c

mkedit:  ;  get -e s.regcmp.mk
mkdelta: ;  delta s.regcmp.mk

clean:
	:

clobber:
	  rm -f regcmp

delete:	clobber
	rm -f $(SOURCE)
