#     @(#) iAPX286 iAPX286.mk 1.3 - 85/08/09
#     Copyright (c) 1985 AT&T
#       All Rights Reserved
#     THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#     The copyright notice above does not evidence any
#     actual or intended publication of such source code.
# @(#)iAPX286.mk	6.1
all:	machine system drivers pwbstuff emulstuff

machine:
	cd ml; make -f ml.mk "FRC=$(FRC)" "INCRT=$(INCRT)"

system:
	cd os; make -f os.mk "FRC=$(FRC)" "INCRT=$(INCRT)"

drivers:
	cd io; make -f io.mk "FRC=$(FRC)" "INCRT=$(INCRT)"

pwbstuff:
	cd pwb; make -f pwb.mk "FRC=$(FRC)" "INCRT=$(INCRT)"

emulstuff:
	cd em; make -f em.mk "FRC=$(FRC)" "INCRT=$(INCRT)"

clean:
	cd ml; make -f ml.mk clean
	cd os; make -f os.mk clean
	cd io; make -f io.mk clean
	cd pwb; make -f pwb.mk clean
	cd em; make -f em.mk clean

clobber:
	cd ml; make -f ml.mk clobber
	cd os; make -f os.mk clobber
	cd io; make -f io.mk clobber
	cd pwb; make -f pwb.mk clobber
	cd em; make -f em.mk clobber

FRC:
