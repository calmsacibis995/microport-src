#     @(#) iAPX286 iAPX286.mk 1.3 - 85/08/09
#     Copyright (c) 1985 AT&T
#       All Rights Reserved
#     THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#     The copyright notice above does not evidence any
#     actual or intended publication of such source code.
# @(#)iAPX286.mk	6.2
#	Modified by Microport
#
# M001	uport!mike Mon May 11 13:21:53 PDT 1987
#	modified to allow AT MERGE compiles in parallel directory.
#	to make atmerge directory:
#		cd rel.xxx/ibm
#		mkdir merge
#		cd merge
#		make -f ../iAPX286/Makefile ATMERGE=-DATERMGE
#

MAKEFILE = Makefile
SYSDIR  = ../iAPX286
SRC	= ../$(SYSDIR)
LIBS	= ml os io pwb em db
ALLDIRS = $(LIBS) cmd cf

# default is to make the unix libraries only,
# use "make all" to also make the rest of the stuff
unix:	libs link
libs:	$(LIBS)
all:	libs cmd unix
link:	cf

# ATT specified names
machine:	ml
system:		os
drivers:	io
pwbstuff:	pwb
emulstuff:	em
debugger:	db
commands:	cmd

dirs:;	for i in $(ALLDIRS) cmd \
	do test -d $@ || mkdir $@ ; \
	done 
	
$(ALLDIRS): $(SYSDIR)/$$@ FRC
	@echo "=== $@ === \c"
	cd $@; $(MAKE) -f $(SRC)/$@/$(MAKEFILE) "ATMERGE=$(ATMERGE)"

clean:;	for i in $(ALLDIRS) \
	do cd $$i; $(MAKE) -f $(SRC)/$$@/$(MAKEFILE) clean; \
	done

clobber:; for i in $(ALLDIRS) \
	do cd $$i; $(MAKE) -f $(SRC)/$$@/$(MAKEFILE) clobber; \
	done

FRC:
	@echo FRC
