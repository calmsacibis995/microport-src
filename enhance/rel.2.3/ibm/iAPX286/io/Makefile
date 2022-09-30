#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#       @(#)io.mk	2.3 - 4/15/87
#	@(#)Makefile	1.7
# Modification History
# uport!dwight Mon Feb 17 11:43:42 PST 1986
#	Upgraded for the IBM AT.
# uport!doug Oct 4 
# 	Added cartridge tape driver
# uport!mark Dec 7
# 	Don't do anything special for the DigiBoard now
# uport!doug Tue Jan  6 12:19:30 PDT 1987
#	fix object dependency lines to refer to library objects
#	remove setup date and touch lines as clock is now ok
# uport!rex 4/20/87
#	modified for new driver names and mike's make depend
# uport!mike 4/27/87
#	modified for new keyboard driver module names
# uport!mike Mon May 11 13:30:45 PDT 1987
#	Modified to allow AT MERGE kernel in parallel directory.
#	See ../Makefile for details.

# ALLOPTIONS is needed for make depend. Should include all possible
#   switches that affect the include file dependency.
ALLOPTIONS =	-DSIASM -DLCCFIX -DPICFIX1 -DPICFIX3 -DPICFIX4 -DATMERGE -DATPROBE -DLCCSIO
CFLAGS =	-Ml $O $(DEBUG) $(INCS) $(OPTIONS)
CON =		console
CPP =		/lib/cpp -DiAPX286 $(DEBUG) $(INCS) $(ALLOPTIONS)
CT =		ct
DIR = 		io
FRC =	
INCS =		-I$(SRC) -I$(SRC)/sys -I$(ROOT)/usr/include 
LIBNAME =	../lib2
MAKEFILE =	Makefile
MAKENEW = 	.n.$(MAKEFILE)
MAKEOLD = 	.o.$(MAKEFILE)
O =		-O
OPT =		$(ROOT)/$(PFX)/lib/$(PFX)optim
OPTIONS =	-DSIASM -DLCCFIX -DPICFIX1 -DPICFIX3 -DPICFIX4 $(ATMERGE)
#OPTIONS =	-DSIASM -DLCCFIX -DATMERGE -DATPROBE 
SRC =		../../iAPX286
SRCDIR =	$(SRC)/$(DIR)

CFILES = \
	8237.c\
	asy1.c\
	asyio.c\
	asyfast.s\
	asyconf.c\
	asyint.s\
	atspec.c\
	bio.c\
	c8274.c\
	cico.c\
	clist.c\
	cram1.c\
	err.c\
	fd.subs.c\
	fd1.c\
	hd.conf.c\
	hd.dump.c\
	hd.subs.c\
	hd1.c\
	io_bufmgr.c\
	kd1.c\
	kd_ansi.c\
	kd_color.c\
	kd_keyint.c\
	kd_keymap.c\
	kd_merge.c\
	kd_mode.c\
	kd_setup.c\
	kd_subs.s\
	lp.c\
	mem.c\
	partab.c\
	rdswap.c\
	mplib.c\
	sys.c\
	tt0.c\
	tty.c\
	win.s

FILES =\
	$(LIBNAME)(8237.o)\
	$(LIBNAME)(asy1.o)\
	$(LIBNAME)(asyio.o)\
	$(LIBNAME)(asyfast.o)\
	$(LIBNAME)(asyconf.o)\
	$(LIBNAME)(asyint.o)\
	$(LIBNAME)(atspec.o)\
	$(LIBNAME)(bio.o)\
	$(LIBNAME)(c8274.o)\
	$(LIBNAME)(cico.o)\
	$(LIBNAME)(clist.o)\
	$(LIBNAME)(cram1.o)\
	$(LIBNAME)(err.o)\
	$(LIBNAME)(fd.subs.o)\
	$(LIBNAME)(fd1.o)\
	$(LIBNAME)(hd.conf.o)\
	$(LIBNAME)(hd.dump.o)\
	$(LIBNAME)(hd.subs.o)\
	$(LIBNAME)(hd1.o)\
	$(LIBNAME)(io_bufmgr.o)\
	$(LIBNAME)(kd1.o)\
	$(LIBNAME)(kd_ansi.o)\
	$(LIBNAME)(kd_color.o)\
	$(LIBNAME)(kd_keyint.o)\
	$(LIBNAME)(kd_keymap.o)\
	$(LIBNAME)(kd_merge.o)\
	$(LIBNAME)(kd_mode.o)\
	$(LIBNAME)(kd_setup.o)\
	$(LIBNAME)(kd_subs.o)\
	$(LIBNAME)(lp.o)\
	$(LIBNAME)(mem.o)\
	$(LIBNAME)(partab.o)\
	$(LIBNAME)(rdswap.o)\
	$(LIBNAME)(mplib.o)\
	$(LIBNAME)(sys.o)\
	$(LIBNAME)(tt0.o)\
	$(LIBNAME)(tty.o)\
	$(LIBNAME)(win.o)

.PRECIOUS:	$(LIBNAME)

#.s.a:
#	$(CC) -c $(CFLAGS) $*.s
#	$(AR) rv $(LIBNAME) $*.o

.s.o:
	$(CC) -c $(CFLAGS) $*.s

.c.s:
	$(CC) -S $(CFLAGS) $*.c
	$(AR) rv $(LIBNAME) $*.o

#.c.a:
#	$(CC) -c $(CFLAGS)  $*.c
#	$(AR) rv $(LIBNAME) $*.o

all:	$(LIBNAME) altlib

altlib:
	$(MAKE) -f ALTLIB.mk

$(LIBNAME):	$(FILES)
#	-chgrp bin $(LIBNAME)
#	-chmod 664 $(LIBNAME)
#	-chown bin $(LIBNAME)

clean:
	-rm -f *.[o]

clobber:	clean
	-rm -f $(LIBNAME)

kernel:	all
	cd ../cf; $(MAKE)
dbg:	kernel stop putty kd_debug test
	@echo Done.
debug:	all
	cd ../db; $(MAKE) kernel
test:;	cp ../system5 /sys5.test
	sync
stop:;	patch ../system5 kernstop 1
putty:;	patch ../system5 kernputc 0x3f8
	patch ../system5 kdbgputc 0x3f8
	patch ../system5 kdbggetc 0x3f8
floppy:; cd ../db; $(MAKE) floppy

FRC:

depend:
	echo '# DO NOT DELETE THIS LINE - make depend uses it\n' > .deplist
	echo "# Dependencies generated on `date`" >> .deplist
	echo 'FILES =\c' > .filelist
	for i in $(CFILES); \
	do \
	    echo "$$i" >> /dev/tty; \
	    file=`expr $$i : '\(.*\)\..*'`; \
 	    echo "\\\\\n\t$$(LIBNAME)($$file.o)\c" >> .filelist; \
 	    echo "$$(LIBNAME)($$file.o): \c"; \
# 	    echo "\\\\\n\t$$file.o\c" >> .filelist; \
# 	    echo "$$file.o: \c"; \
 	    $(CPP) $(SRCDIR)/$$i | \
	     grep -v "/usr/include" | \
	     awk '/^# 1 /{printf "\\\n\t%s",$$3};END{print;print "\t$$(CC) -c $$(CFLAGS) $$(SRCDIR)/'$$i'";print "\tar rv $$(LIBNAME) '$$file.o'";print}'|\
#	     awk '/^# 1 /{printf "\\\n\t%s",$$3};END{print;print "\t$$(CC) -c $$(CFLAGS) $$(SRCDIR)/'$$i'";print}'|\
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

# Dependencies generated on Fri Jan  8 16:29:53 PST 1988
$(LIBNAME)(8237.o): \
	../../iAPX286/io/8237.c\
	../../iAPX286/sys/types.h\
	../../iAPX286/sys/param.h\
	../../iAPX286/sys/8237.h\
	../../iAPX286/sys/buf.h
	$(CC) -c $(CFLAGS) $(SRCDIR)/8237.c
	ar rv $(LIBNAME) 8237.o

$(LIBNAME)(asy1.o): \
	../../iAPX286/io/asy1.c\
	../../iAPX286/sys/types.h\
	../../iAPX286/sys/param.h\
	../../iAPX286/sys/systm.h\
	../../iAPX286/sys/conf.h\
	../../iAPX286/sys/user.h\
	../../iAPX286/sys/tss.h\
	../../iAPX286/sys/proc.h\
	../../iAPX286/sys/tty.h\
	../../iAPX286/sys/sysinfo.h\
	../../iAPX286/sys/16450.h\
	../../iAPX286/sys/clock.h\
	../../iAPX286/sys/ppi.h\
	../../iAPX286/sys/asycnt.h\
	../../iAPX286/sys/realmode.h
	$(CC) -c $(CFLAGS) $(SRCDIR)/asy1.c
	ar rv $(LIBNAME) asy1.o

$(LIBNAME)(asyio.o): \
	../../iAPX286/io/asyio.c
	$(CC) -c $(CFLAGS) $(SRCDIR)/asyio.c
	ar rv $(LIBNAME) asyio.o

$(LIBNAME)(asyfast.o): \
	../../iAPX286/io/asyfast.s
	$(CC) -c $(CFLAGS) $(SRCDIR)/asyfast.s
	ar rv $(LIBNAME) asyfast.o

$(LIBNAME)(asyconf.o): \
	../../iAPX286/io/asyconf.c\
	../../iAPX286/sys/types.h\
	../../iAPX286/sys/param.h\
	../../iAPX286/sys/systm.h\
	../../iAPX286/sys/conf.h\
	../../iAPX286/sys/user.h\
	../../iAPX286/sys/tss.h\
	../../iAPX286/sys/proc.h\
	../../iAPX286/sys/tty.h\
	../../iAPX286/sys/sysinfo.h\
	../../iAPX286/sys/16450.h\
	../../iAPX286/sys/clock.h\
	../../iAPX286/sys/ppi.h\
	../../iAPX286/sys/asycnt.h
	$(CC) -c $(CFLAGS) $(SRCDIR)/asyconf.c
	ar rv $(LIBNAME) asyconf.o

$(LIBNAME)(asyint.o): \
	../../iAPX286/io/asyint.s\
	../../iAPX286/sys/param.h\
	../../iAPX286/sys/mmu.h\
	../../iAPX286/sys/trap.h\
	../../iAPX286/sys/8259.h
	$(CC) -c $(CFLAGS) $(SRCDIR)/asyint.s
	ar rv $(LIBNAME) asyint.o

$(LIBNAME)(atspec.o): \
	../../iAPX286/io/atspec.c\
	../../iAPX286/sys/param.h\
	../../iAPX286/sys/types.h\
	../../iAPX286/sys/sysinfo.h\
	../../iAPX286/sys/user.h\
	../../iAPX286/sys/tss.h\
	../../iAPX286/sys/uadmin.h
	$(CC) -c $(CFLAGS) $(SRCDIR)/atspec.c
	ar rv $(LIBNAME) atspec.o

$(LIBNAME)(bio.o): \
	../../iAPX286/io/bio.c\
	../../iAPX286/sys/param.h\
	../../iAPX286/sys/types.h\
	../../iAPX286/sys/systm.h\
	../../iAPX286/sys/sysinfo.h\
	../../iAPX286/sys/user.h\
	../../iAPX286/sys/tss.h\
	../../iAPX286/sys/buf.h\
	../../iAPX286/sys/iobuf.h\
	../../iAPX286/sys/conf.h\
	../../iAPX286/sys/proc.h\
	../../iAPX286/sys/seg.h\
	../../iAPX286/sys/var.h\
	../../iAPX286/sys/mmu.h
	$(CC) -c $(CFLAGS) $(SRCDIR)/bio.c
	ar rv $(LIBNAME) bio.o

$(LIBNAME)(c8274.o): \
	../../iAPX286/io/c8274.c\
	../../iAPX286/sys/mpsc.h
	$(CC) -c $(CFLAGS) $(SRCDIR)/c8274.c
	ar rv $(LIBNAME) c8274.o

$(LIBNAME)(cico.o): \
	../../iAPX286/io/cico.c
	$(CC) -c $(CFLAGS) $(SRCDIR)/cico.c
	ar rv $(LIBNAME) cico.o

$(LIBNAME)(clist.o): \
	../../iAPX286/io/clist.c\
	../../iAPX286/sys/param.h\
	../../iAPX286/sys/types.h\
	../../iAPX286/sys/tty.h
	$(CC) -c $(CFLAGS) $(SRCDIR)/clist.c
	ar rv $(LIBNAME) clist.o

$(LIBNAME)(cram1.o): \
	../../iAPX286/io/cram1.c\
	../../iAPX286/sys/param.h\
	../../iAPX286/sys/types.h\
	../../iAPX286/sys/user.h\
	../../iAPX286/sys/tss.h\
	../../iAPX286/sys/buf.h\
	../../iAPX286/sys/systm.h\
	../../iAPX286/sys/mmu.h\
	../../iAPX286/sys/seg.h\
	../../iAPX286/sys/io_op.h\
	../../iAPX286/sys/cram.h
	$(CC) -c $(CFLAGS) $(SRCDIR)/cram1.c
	ar rv $(LIBNAME) cram1.o

$(LIBNAME)(err.o): \
	../../iAPX286/io/err.c\
	../../iAPX286/sys/param.h\
	../../iAPX286/sys/types.h\
	../../iAPX286/sys/buf.h\
	../../iAPX286/sys/user.h\
	../../iAPX286/sys/tss.h
	$(CC) -c $(CFLAGS) $(SRCDIR)/err.c
	ar rv $(LIBNAME) err.o

$(LIBNAME)(fd.subs.o): \
	../../iAPX286/io/fd.subs.c\
	../../iAPX286/sys/fd.h\
	../../iAPX286/sys/types.h\
	../../iAPX286/sys/param.h\
	../../iAPX286/sys/systm.h\
	../../iAPX286/sys/buf.h\
	../../iAPX286/sys/iobuf.h\
	../../iAPX286/sys/conf.h\
	../../iAPX286/sys/user.h\
	../../iAPX286/sys/tss.h\
	../../iAPX286/sys/trap.h\
	../../iAPX286/sys/mmu.h\
	../../iAPX286/sys/seg.h\
	../../iAPX286/sys/8237.h
	$(CC) -c $(CFLAGS) $(SRCDIR)/fd.subs.c
	ar rv $(LIBNAME) fd.subs.o

$(LIBNAME)(fd1.o): \
	../../iAPX286/io/fd1.c\
	../../iAPX286/sys/fd.h\
	../../iAPX286/sys/types.h\
	../../iAPX286/sys/param.h\
	../../iAPX286/sys/systm.h\
	../../iAPX286/sys/buf.h\
	../../iAPX286/sys/iobuf.h\
	../../iAPX286/sys/conf.h\
	../../iAPX286/sys/user.h\
	../../iAPX286/sys/tss.h\
	../../iAPX286/sys/trap.h\
	../../iAPX286/sys/mmu.h\
	../../iAPX286/sys/seg.h\
	../../iAPX286/sys/8237.h\
	../../iAPX286/sys/format.h\
	../../iAPX286/sys/realmode.h
	$(CC) -c $(CFLAGS) $(SRCDIR)/fd1.c
	ar rv $(LIBNAME) fd1.o

$(LIBNAME)(hd.conf.o): \
	../../iAPX286/io/hd.conf.c\
	../../iAPX286/sys/types.h\
	../../iAPX286/sys/param.h\
	../../iAPX286/sys/buf.h\
	../../iAPX286/sys/iobuf.h\
	../../iAPX286/sys/hd.h\
	../../iAPX286/sys/hddefaults.h
	$(CC) -c $(CFLAGS) $(SRCDIR)/hd.conf.c
	ar rv $(LIBNAME) hd.conf.o

$(LIBNAME)(hd.dump.o): \
	../../iAPX286/io/hd.dump.c\
	../../iAPX286/sys/param.h\
	../../iAPX286/sys/types.h\
	../../iAPX286/sys/hd.h
	$(CC) -c $(CFLAGS) $(SRCDIR)/hd.dump.c
	ar rv $(LIBNAME) hd.dump.o

$(LIBNAME)(hd.subs.o): \
	../../iAPX286/io/hd.subs.c\
	../../iAPX286/sys/misc.h\
	../../iAPX286/sys/types.h\
	../../iAPX286/sys/param.h\
	../../iAPX286/sys/systm.h\
	../../iAPX286/sys/buf.h\
	../../iAPX286/sys/iobuf.h\
	../../iAPX286/sys/conf.h\
	../../iAPX286/sys/user.h\
	../../iAPX286/sys/tss.h\
	../../iAPX286/sys/hd.h\
	../../iAPX286/sys/tvi.h\
	../../iAPX286/sys/mmu.h\
	../../iAPX286/sys/seg.h\
	../../iAPX286/sys/8259.h
	$(CC) -c $(CFLAGS) $(SRCDIR)/hd.subs.c
	ar rv $(LIBNAME) hd.subs.o

$(LIBNAME)(hd1.o): \
	../../iAPX286/io/hd1.c\
	../../iAPX286/sys/types.h\
	../../iAPX286/sys/param.h\
	../../iAPX286/sys/systm.h\
	../../iAPX286/sys/buf.h\
	../../iAPX286/sys/iobuf.h\
	../../iAPX286/sys/conf.h\
	../../iAPX286/sys/user.h\
	../../iAPX286/sys/tss.h\
	../../iAPX286/sys/hd.h\
	../../iAPX286/sys/tvi.h\
	../../iAPX286/sys/trap.h\
	../../iAPX286/sys/mmu.h\
	../../iAPX286/sys/seg.h\
	../../iAPX286/sys/misc.h
	$(CC) -c $(CFLAGS) $(SRCDIR)/hd1.c
	ar rv $(LIBNAME) hd1.o

$(LIBNAME)(io_bufmgr.o): \
	../../iAPX286/io/io_bufmgr.c\
	../../iAPX286/sys/types.h\
	../../iAPX286/sys/param.h\
	../../iAPX286/sys/mmu.h\
	../../iAPX286/sys/seg.h\
	../../iAPX286/sys/map.h\
	../../iAPX286/sys/io_bufmgr.h
	$(CC) -c $(CFLAGS) $(SRCDIR)/io_bufmgr.c
	ar rv $(LIBNAME) io_bufmgr.o

$(LIBNAME)(kd1.o): \
	../../iAPX286/io/kd1.c\
	../../iAPX286/sys/types.h\
	../../iAPX286/sys/param.h\
	../../iAPX286/sys/conf.h\
	../../iAPX286/sys/proc.h\
	../../iAPX286/sys/ppi.h\
	../../iAPX286/sys/mmu.h\
	../../iAPX286/sys/user.h\
	../../iAPX286/sys/tty.h\
	../../iAPX286/sys/clock.h\
	../../iAPX286/sys/kd_video.h\
	../../iAPX286/sys/seg.h\
	../../iAPX286/sys/kd.h\
	../../iAPX286/sys/kd_info.h\
	../../iAPX286/sys/setkey.h\
	../../iAPX286/sys/kd_color.h
	$(CC) -c $(CFLAGS) $(SRCDIR)/kd1.c
	ar rv $(LIBNAME) kd1.o

$(LIBNAME)(kd_ansi.o): \
	../../iAPX286/io/kd_ansi.c\
	../../iAPX286/sys/types.h\
	../../iAPX286/sys/param.h\
	../../iAPX286/sys/systm.h\
	../../iAPX286/sys/conf.h\
	../../iAPX286/sys/proc.h\
	../../iAPX286/sys/ppi.h\
	../../iAPX286/sys/mmu.h\
	../../iAPX286/sys/user.h\
	../../iAPX286/sys/tss.h\
	../../iAPX286/sys/tty.h\
	../../iAPX286/sys/sysinfo.h\
	../../iAPX286/sys/clock.h\
	../../iAPX286/sys/kd_video.h\
	../../iAPX286/sys/seg.h\
	../../iAPX286/sys/kd.h\
	../../iAPX286/sys/kd_info.h\
	../../iAPX286/sys/setkey.h\
	../../iAPX286/sys/kd_color.h\
	../../iAPX286/sys/realmode.h
	$(CC) -c $(CFLAGS) $(SRCDIR)/kd_ansi.c
	ar rv $(LIBNAME) kd_ansi.o

$(LIBNAME)(kd_color.o): \
	../../iAPX286/io/kd_color.c\
	../../iAPX286/sys/types.h\
	../../iAPX286/sys/param.h\
	../../iAPX286/sys/systm.h\
	../../iAPX286/sys/conf.h\
	../../iAPX286/sys/proc.h\
	../../iAPX286/sys/ppi.h\
	../../iAPX286/sys/mmu.h\
	../../iAPX286/sys/user.h\
	../../iAPX286/sys/tss.h\
	../../iAPX286/sys/tty.h\
	../../iAPX286/sys/sysinfo.h\
	../../iAPX286/sys/clock.h\
	../../iAPX286/sys/kd_video.h\
	../../iAPX286/sys/seg.h\
	../../iAPX286/sys/kd.h\
	../../iAPX286/sys/kd_info.h\
	../../iAPX286/sys/setkey.h\
	../../iAPX286/sys/kd_color.h\
	../../iAPX286/sys/kd_mode.h\
	../../iAPX286/sys/realmode.h
	$(CC) -c $(CFLAGS) $(SRCDIR)/kd_color.c
	ar rv $(LIBNAME) kd_color.o

$(LIBNAME)(kd_keyint.o): \
	../../iAPX286/io/kd_keyint.c\
	../../iAPX286/sys/types.h\
	../../iAPX286/sys/param.h\
	../../iAPX286/sys/systm.h\
	../../iAPX286/sys/seg.h\
	../../iAPX286/sys/proc.h\
	../../iAPX286/sys/ppi.h\
	../../iAPX286/sys/mmu.h\
	../../iAPX286/sys/8259.h\
	../../iAPX286/sys/user.h\
	../../iAPX286/sys/tss.h\
	../../iAPX286/sys/conf.h\
	../../iAPX286/sys/tty.h\
	../../iAPX286/sys/sysinfo.h\
	../../iAPX286/sys/clock.h\
	../../iAPX286/sys/kd_video.h\
	../../iAPX286/sys/kd.h\
	../../iAPX286/sys/kd_info.h\
	../../iAPX286/sys/setkey.h\
	../../iAPX286/sys/kd_color.h
	$(CC) -c $(CFLAGS) $(SRCDIR)/kd_keyint.c
	ar rv $(LIBNAME) kd_keyint.o

$(LIBNAME)(kd_keymap.o): \
	../../iAPX286/io/kd_keymap.c\
	../../iAPX286/sys/types.h\
	../../iAPX286/sys/kd_video.h\
	../../iAPX286/sys/kd.h\
	../../iAPX286/sys/kd_info.h\
	../../iAPX286/sys/setkey.h\
	../../iAPX286/sys/kd_color.h\
	../../iAPX286/sys/kd_data.h
	$(CC) -c $(CFLAGS) $(SRCDIR)/kd_keymap.c
	ar rv $(LIBNAME) kd_keymap.o

$(LIBNAME)(kd_merge.o): \
	../../iAPX286/io/kd_merge.c\
	../../iAPX286/sys/types.h\
	../../iAPX286/sys/param.h\
	../../iAPX286/sys/systm.h\
	../../iAPX286/sys/seg.h\
	../../iAPX286/sys/proc.h\
	../../iAPX286/sys/ppi.h\
	../../iAPX286/sys/mmu.h\
	../../iAPX286/sys/8259.h\
	../../iAPX286/sys/user.h\
	../../iAPX286/sys/tss.h\
	../../iAPX286/sys/conf.h\
	../../iAPX286/sys/tty.h\
	../../iAPX286/sys/sysinfo.h\
	../../iAPX286/sys/clock.h\
	../../iAPX286/sys/realmode.h\
	../../iAPX286/sys/kd_video.h\
	../../iAPX286/sys/kd.h\
	../../iAPX286/sys/kd_info.h\
	../../iAPX286/sys/setkey.h\
	../../iAPX286/sys/kd_color.h
	$(CC) -c $(CFLAGS) $(SRCDIR)/kd_merge.c
	ar rv $(LIBNAME) kd_merge.o

$(LIBNAME)(kd_mode.o): \
	../../iAPX286/io/kd_mode.c\
	../../iAPX286/sys/kd_mode.h
	$(CC) -c $(CFLAGS) $(SRCDIR)/kd_mode.c
	ar rv $(LIBNAME) kd_mode.o

$(LIBNAME)(kd_setup.o): \
	../../iAPX286/io/kd_setup.c\
	../../iAPX286/sys/types.h\
	../../iAPX286/sys/param.h\
	../../iAPX286/sys/systm.h\
	../../iAPX286/sys/conf.h\
	../../iAPX286/sys/proc.h\
	../../iAPX286/sys/ppi.h\
	../../iAPX286/sys/mmu.h\
	../../iAPX286/sys/user.h\
	../../iAPX286/sys/tss.h\
	../../iAPX286/sys/tty.h\
	../../iAPX286/sys/sysinfo.h\
	../../iAPX286/sys/clock.h\
	../../iAPX286/sys/seg.h\
	../../iAPX286/sys/kd_video.h\
	../../iAPX286/sys/kd.h\
	../../iAPX286/sys/kd_info.h\
	../../iAPX286/sys/setkey.h\
	../../iAPX286/sys/kd_color.h\
	../../iAPX286/sys/kd_mode.h\
	../../iAPX286/sys/realmode.h
	$(CC) -c $(CFLAGS) $(SRCDIR)/kd_setup.c
	ar rv $(LIBNAME) kd_setup.o

$(LIBNAME)(kd_subs.o): \
	../../iAPX286/io/kd_subs.s
	$(CC) -c $(CFLAGS) $(SRCDIR)/kd_subs.s
	ar rv $(LIBNAME) kd_subs.o

$(LIBNAME)(lp.o): \
	../../iAPX286/io/lp.c\
	../../iAPX286/sys/param.h\
	../../iAPX286/sys/types.h\
	../../iAPX286/sys/user.h\
	../../iAPX286/sys/tss.h\
	../../iAPX286/sys/tty.h\
	../../iAPX286/sys/lprio.h\
	../../iAPX286/sys/realmode.h
	$(CC) -c $(CFLAGS) $(SRCDIR)/lp.c
	ar rv $(LIBNAME) lp.o

$(LIBNAME)(mem.o): \
	../../iAPX286/io/mem.c\
	../../iAPX286/sys/param.h\
	../../iAPX286/sys/types.h\
	../../iAPX286/sys/user.h\
	../../iAPX286/sys/tss.h\
	../../iAPX286/sys/buf.h\
	../../iAPX286/sys/systm.h\
	../../iAPX286/sys/mmu.h\
	../../iAPX286/sys/seg.h\
	../../iAPX286/sys/io_op.h
	$(CC) -c $(CFLAGS) $(SRCDIR)/mem.c
	ar rv $(LIBNAME) mem.o

$(LIBNAME)(partab.o): \
	../../iAPX286/io/partab.c
	$(CC) -c $(CFLAGS) $(SRCDIR)/partab.c
	ar rv $(LIBNAME) partab.o

$(LIBNAME)(rdswap.o): 
	$(CC) -c $(CFLAGS) $(SRCDIR)/rdswap.c
	ar rv $(LIBNAME) rdswap.o

$(LIBNAME)(mplib.o): 
	$(CC) -c $(CFLAGS) $(SRCDIR)/mplib.c
	ar rv $(LIBNAME) mplib.o

$(LIBNAME)(sys.o): \
	../../iAPX286/io/sys.c\
	../../iAPX286/sys/param.h\
	../../iAPX286/sys/types.h\
	../../iAPX286/sys/user.h\
	../../iAPX286/sys/tss.h\
	../../iAPX286/sys/conf.h\
	../../iAPX286/sys/proc.h
	$(CC) -c $(CFLAGS) $(SRCDIR)/sys.c
	ar rv $(LIBNAME) sys.o

$(LIBNAME)(tt0.o): \
	../../iAPX286/io/tt0.c\
	../../iAPX286/sys/types.h\
	../../iAPX286/sys/param.h\
	../../iAPX286/sys/systm.h\
	../../iAPX286/sys/conf.h\
	../../iAPX286/sys/user.h\
	../../iAPX286/sys/tss.h\
	../../iAPX286/sys/proc.h\
	../../iAPX286/sys/tty.h\
	../../iAPX286/sys/sysinfo.h
	$(CC) -c $(CFLAGS) $(SRCDIR)/tt0.c
	ar rv $(LIBNAME) tt0.o

$(LIBNAME)(tty.o): \
	../../iAPX286/io/tty.c\
	../../iAPX286/sys/param.h\
	../../iAPX286/sys/types.h\
	../../iAPX286/sys/systm.h\
	../../iAPX286/sys/user.h\
	../../iAPX286/sys/tss.h\
	../../iAPX286/sys/tty.h\
	../../iAPX286/sys/proc.h\
	../../iAPX286/sys/conf.h\
	../../iAPX286/sys/sysinfo.h
	$(CC) -c $(CFLAGS) $(SRCDIR)/tty.c
	ar rv $(LIBNAME) tty.o

$(LIBNAME)(win.o): \
	../../iAPX286/io/win.s\
	../../iAPX286/sys/param.h\
	../../iAPX286/sys/mmu.h
	$(CC) -c $(CFLAGS) $(SRCDIR)/win.s
	ar rv $(LIBNAME) win.o

