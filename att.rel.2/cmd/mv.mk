#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#	mv/cp/ln make file
#	@(#)	1.2	#

ROOT =
OL = $(ROOT)/
SL = $(ROOT)/usr/src/cmd
RDIR = $(SL)
INS = :
REL = current
CSID = -r`gsid mv $(REL)`
MKSID = -r`gsid mv.mk $(REL)`
LIST = lp
INSDIR = $(OL)bin
INSLIB = $(ROOT)/usr/lib
TESTDIR = .
IFLAG = -n
CFLAGS = -O
LDFLAGS = -s $(IFLAG)
SOURCE = mv.c mv_dir.c
MAKE = make
INC = -I $(ROOT)/usr/include -I $(ROOT)/usr/include/sys

compile all: mv_dir mv
	:
mv_dir:
	$(CC) $(CFLAGS) $(INC) $(LDFLAGS) -o $(TESTDIR)/mv_dir mv_dir.c
#	cp $(INSDIR)/mv $(TESTDIR)/cp;
	$(INS) $(INSLIB)  $(TESTDIR)/mv_dir;
	rm -f $(TESTDIR)/cp;


mv:
	$(CC) $(CFLAGS) $(INC)  $(LDFLAGS) -o $(TESTDIR)/mv mv.c
	# PATH must not look in currdir for this to work!
		$(INS) $(INSDIR)  $(TESTDIR)/mv; \
		$(CH) chown root $(INSDIR)/mv; \
		rm -f $(INSDIR)/ln; \
		ln $(INSDIR)/mv $(INSDIR)/ln; \
		rm -f $(INSDIR)/cp; \
		ln $(INSDIR)/mv $(INSDIR)/cp; \
	$(CH) -chmod 775 $(INSDIR)/mv $(INSDIR)/ln $(INSDIR)/cp

install:
	$(MAKE) -f mv.mk INS="install -f" OL=$(OL)

build:	bldmk
	get -p $(CSID) s.mv.c $(REWIRE) > $(RDIR)/mv.c
	get -p $(CSID) s.mv_dir.c $(REWIRE) > $(RDIR)/mv_dir.c
bldmk:  ;  get -p $(MKSID) s.mv.mk > $(RDIR)/mv.mk
	cd $(RDIR); rm -f ln.mk cp.mk
	cd $(RDIR); ln mv.mk cp.mk; ln mv.mk ln.mk

listing:
	pr mv.mk $(SOURCE) | $(LIST)
listmk: ;  pr mv.mk | $(LIST)

edit:
	get -e s.mv.c
	get -e s.mv_dir.c

delta:
	delta s.mv.c
	delta s.mv_dir.c
	rm -f $(SOURCE)

mkedit:  ;  get -e s.mv.mk
mkdelta: ;  delta s.mv.mk

clean:
	:

clobber:
	rm -f $(TESTDIR)/mv $(TESTDIR)/ln $(TESTDIR)/cp $(TESTDIR)/mv_dir

delete:	clobber
	rm -f $(SOURCE)
