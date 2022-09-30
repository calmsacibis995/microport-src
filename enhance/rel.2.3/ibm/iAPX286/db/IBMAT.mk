#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.
#
# @(#)makefile	1.11

#ident	"@(#)db.mk	1.15"

# uport!mike Mon May 11 13:30:45 PDT 1987
#	Modified to allow AT MERGE kernel in parallel directory.
#	See ../Makefile for details.

# ALLOPTIONS is needed for make depend. Should include all possible
#   switches that affect the include file dependency.
# AT386 is used to define AT type i/o
# we need to have all the defines used in ../io/makefile and ../os/makefile
ALLOPTIONS =	-DLCCFIX -DIBMAT -DSIASM -DAT386 -DGDEBUGGER $(INCS) $(ATMERGE)
CFLAGS =	-Ml $O $(DEBUG) $(INCS) $(OPTIONS)
CON =		console
CPP =		/lib/cpp -DiAPX286 $(DEBUG) $(INCS) $(ALLOPTIONS)
DIR = 		db
FRC =	
INCS =		-I$(SRC) -I$(SRC)/sys -I$(ROOT)/usr/include 
MAKEFILE =	Makefile
MAKENEW = 	.n.$(MAKEFILE)
MAKEOLD = 	.o.$(MAKEFILE)
O =		-O
OFILE =		../db.o
OPT =		$(ROOT)/$(PFX)/lib/$(PFX)optim
OPTIONS =	-DLCCFIX -DIBMAT -DSIASM -DAT386 -DGDEBUGGER $(INCS) $(ATMERGE)
ROOTNAME =	$(SRC)/cmd/rootname/rootname
SRC =		../../iAPX286
SRCDIR =	$(SRC)/$(DIR)
UNIXSYMS =	../cmd/unixsyms/unixsyms

FRC =

CFILES =\
	db.c\
	dbcon.c\
	dbg.c\
	dbintrp.c\
	dblex.c\
	showthings.c\
	sysnames.c\
	../io/kd_asydbg.c\
	../os/prf.c\
	../os/trap.c

FILES =\
	db.o\
	dbcon.o\
	dbg.o\
	dbintrp.o\
	dblex.o\
	showthings.o\
	sysnames.o\
	kd_asydbg.o\
	prf.o\
	trap.o

all:    $(OFILE)

$(OFILE):	$(FILES)
	$(LD) -r -Ml -o $(OFILE) $(FILES)

kernel:	all
	cd ../cf; make DEBUGLIB=$(OFILE)
	$(UNIXSYMS) ../system5

initflop:
	mkfs /dev/dsk/0s25 2120:200 2 15
	dd if=../../dist/etc/flboot.hd of=/dev/dsk/0s24 count=1
	dd if=../../dist/etc/Boot.flhd of=/dev/dsk/0s24 seek=15 count=8
stop:
	patch ../system5 kernstop 1
putty:
	patch ../system5 k_putdev 0x3f8
	patch ../system5 k_getdev 0x3f8
floppy:
	su root -c "mount /dev/dsk/0s25 /mnt"
	cd ..; cp system5 /mnt
	-ln -f /mnt/system5 /mnt/unix
	su root -c "umount /dev/dsk/0s25"
	sync; sync
	fsck /dev/dsk/0s25
	@echo "Finished with creating the debug floppy A "

clean:
	-rm -f *.o

clobber: clean
	-rm -f $(OFILE)

FRC:

depend:
	echo '# DO NOT DELETE THIS LINE - make depend uses it\n' > .deplist
	echo "# Dependencies generated on `date`" >> .deplist
	echo 'FILES =\c' > .filelist
	for i in $(CFILES); \
	do \
	    echo "$$i" >> /dev/tty; \
	    file=`$(ROOTNAME) $$i`; \
 	    echo "\\\\\n\t$$file.o\c" >> .filelist; \
 	    echo "$$file.o: \c"; \
 	    $(CPP) $(SRCDIR)/$$i | \
	     grep -v "/usr/include" | \
	     awk '/^# 1 /{printf "\\\n\t%s",$$3};END{print;print "\t$$(CC) -c $$(CFLAGS) $$(SRCDIR)/'$$i'";print}'|\
 	     sed 's/"//g'; \
 	done >> .deplist; \
 	echo '\n' >> .filelist; \
 	echo '/^FILES /\n.,/^$$/d\n-1r .filelist\n/^# DO NOT DELETE/,$$d\nr .deplist\nw $(MAKENEW)\nq' | ed - $(MAKEFILE)
	mv $(MAKEFILE) $(MAKEOLD)
	mv $(MAKENEW) $(MAKEFILE)
	ln -f $(MAKEFILE) IBMAT.mk
	rm -f .deplist .filelist

# anything after the next line will disappear!
# DO NOT DELETE THIS LINE - make depend uses it

# Dependencies generated on Mon Jun  1 12:06:07 PDT 1987
db.o: \
	../../iAPX286/db/db.c
	$(CC) -c $(CFLAGS) $(SRCDIR)/db.c

dbcon.o: \
	../../iAPX286/db/dbcon.c\
	../../iAPX286/sys/types.h\
	../../iAPX286/sys/param.h
	$(CC) -c $(CFLAGS) $(SRCDIR)/dbcon.c

dbg.o: \
	../../iAPX286/db/dbg.c\
	../../iAPX286/sys/types.h\
	../../iAPX286/sys/param.h\
	../../iAPX286/sys/mmu.h\
	../../iAPX286/sys/reg.h\
	../../iAPX286/sys/seg.h
	$(CC) -c $(CFLAGS) $(SRCDIR)/dbg.c

dbintrp.o: \
	../../iAPX286/db/dbintrp.c
	$(CC) -c $(CFLAGS) $(SRCDIR)/dbintrp.c

dblex.o: \
	../../iAPX286/db/dblex.c
	$(CC) -c $(CFLAGS) $(SRCDIR)/dblex.c

showthings.o: \
	../../iAPX286/db/showthings.c
	$(CC) -c $(CFLAGS) $(SRCDIR)/showthings.c

sysnames.o: \
	../../iAPX286/db/sysnames.c
	$(CC) -c $(CFLAGS) $(SRCDIR)/sysnames.c

kd_asydbg.o: \
	../../iAPX286/db/../io/kd_asydbg.c\
	../../iAPX286/sys/types.h\
	../../iAPX286/sys/kd_video.h\
	../../iAPX286/sys/kd.h\
	../../iAPX286/sys/kd_color.h
	$(CC) -c $(CFLAGS) $(SRCDIR)/../io/kd_asydbg.c

prf.o: \
	../../iAPX286/db/../os/prf.c\
	../../iAPX286/sys/param.h\
	../../iAPX286/sys/types.h\
	../../iAPX286/sys/systm.h\
	../../iAPX286/sys/buf.h\
	../../iAPX286/sys/conf.h
	$(CC) -c $(CFLAGS) $(SRCDIR)/../os/prf.c

trap.o: \
	../../iAPX286/db/../os/trap.c\
	../../iAPX286/sys/param.h\
	../../iAPX286/sys/types.h\
	../../iAPX286/sys/systm.h\
	../../iAPX286/sys/user.h\
	../../iAPX286/sys/tss.h\
	../../iAPX286/sys/proc.h\
	../../iAPX286/sys/reg.h\
	../../iAPX286/sys/psl.h\
	../../iAPX286/sys/trap.h\
	../../iAPX286/sys/seg.h\
	../../iAPX286/sys/sysinfo.h\
	../../iAPX286/sys/mmu.h\
	../../iAPX286/sys/fp.h\
	../../iAPX286/sys/8259.h
	$(CC) -c $(CFLAGS) $(SRCDIR)/../os/trap.c

