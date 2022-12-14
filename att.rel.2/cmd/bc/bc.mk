#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#	bc make file
#	@(#)	1.3	#

ROOT =
OL = $(ROOT)/
SL = $(ROOT)/usr/src/cmd
RDIR = $(SL)/bc
INS = :
REL = current
CSID = -r`gsid bc $(REL)`
MKSID = -r`gsid bc.mk $(REL)`
LIST = lp
INSDIR = $(OL)usr/bin
INSLIB = $(OL)usr/lib
B20 = `if u370; then echo "-b2,0"; fi`
CFLAGS = -O $(B20)
FFLAG =
IFLAG = -n
LDFLAG = -s $(IFLAG) $(FFLAG)
SOURCE = bc.y lib.b
FILES = bc.c
MAKE = make
YACC = yacc

compile all: bc b
	:

bc:	$(FILES)
	$(CC) $(CFLAGS) $(LDFLAG) -o bc $(FILES)
	$(INS) $(INSDIR) bc

$(FILES):
	-$(YACC) bc.y && mv y.tab.c bc.x
	cp bc.x bc.c

b:
	$(INS) $(INSLIB) lib.b

install:
	$(MAKE) -f bc.mk INS="install -f" IFLAG=$(IFLAG) FFLAG=$(FFLAG) OL=$(OL)

build:	bldmk
	get -p $(CSID) s.bc.src $(REWIRE) | ntar -d $(RDIR) -g
	cd $(RDIR); $(YACC) bc.y; mv y.tab.c bc.x

bldmk:  ;  get -p $(MKSID) s.bc.mk > $(RDIR)/bc.mk

listing:
	pr bc.mk $(SOURCE) | $(LIST)
listmk: ;  pr bc.mk | $(LIST)

edit:
	get -e -p s.bc.src | ntar -g

delta:
	ntar -p $(SOURCE) > bc.src
	delta s.bc.src
	rm -f $(SOURCE)

mkedit:  ;  get -e s.bc.mk
mkdelta: ;  delta s.bc.mk

clean:
	:

clobber:	clean
	rm -f bc bc.c bc.x

delete:	clobber
	rm -f $(SOURCE) bc.x
