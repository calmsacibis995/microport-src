#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#	@(#)	1.8
#
# There is currently no provision for local mods (reorder) or
# user mods ($TERMCAP).  These will be coming eventually.
#
# PARTS	= header adds annarbor beehive cdc concept datamedia dec diablo general
# 	  hardcopy hazeltine heath homebrew hp ibm kds lsi microterm misc pc
#	  perkinelmer print special tektronix teleray teletype televideo ti
#	  visual trailer
#
#	PARTS2 is here so that when the minimal set of terminals which
#	were defined to be Safari 5 core terminal set.  These files
#	are distinguished by their .c.ti suffix.
#
# PARTS2 = *.c.ti
#

ROOT	=
PARTS	= header *.ti trailer
PARTS2	= header att.ti concept.ti dec.ti hp.ti lsi.ti print.ti special.ti tektronix.ti televideo.ti virtual.ti
COMPILE	= ../screen/tic

install:
	if u3b2 || u3b5 ;\
	then \
		$(MAKE) -f makefile install2 ;\
	else \
		$(MAKE) -f makefile install1 ;\
	fi

install1:	$(PARTS) install3
	$(COMPILE) $?

install2:	$(PARTS2) install3
	$(COMPILE) $?
	if u3b2 ;\
	then \
		cp ../termcap $(ROOT)/etc/termcap ;\
	fi

install3:
	make terminfo.src
	touch install

terminfo.src:	$(PARTS)
	cat $(PARTS) > terminfo.src

OLDall:	terminfo.src
	$(COMPILE) -v terminfo.src
	touch install

all:	
	if u3b2 || u3b5 ;\
	then \
		cp ../termcap $(ROOT)/etc/termcap ;\
	fi
	cat $(PARTS) > terminfo.src
	$(COMPILE) -v terminfo.src
	touch install
#	if [ $(ROOT) ] ;\
#	then \
#		( cd $(ROOT)/usr/lib/terminfo; \
#		find . -print | sort | cpio -oc  ) >terminfo.cpio ;\
#		sh mknames *.ti >names; \
#	fi

clean:
	rm -f terminfo.src install
