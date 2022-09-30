#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#       @(#)os.mk.unopt	1.1 - 85/08/15
#
# uport!mike Mon May 11 13:30:45 PDT 1987
#	Modified to allow AT MERGE kernel in parallel directory.
#	See ../Makefile for details.

# ALLOPTIONS is needed for make depend. Should include all possible
#   switches that affect the include file dependency.
ALLOPTIONS =	-DSIASM -DIBMAT -DLCCFIX -DPICFIX1 -DPICFIX3 -DPICFIX4 $(ATMERGE)
CFLAGS =	-Ml $O $(DEBUG) $(INCS) $(OPTIONS)
CPP =		/lib/cpp -DiAPX286 $(DEBUG) $(INCS) $(ALLOPTIONS)
DIR = 		os
FRC =	
INCS =		-I$(SRC) -I$(SRC)/sys -I$(ROOT)/usr/include 
LIBNAME =	../lib1
MAKEFILE =	Makefile
MAKENEW = 	.n.$(MAKEFILE)
MAKEOLD = 	.o.$(MAKEFILE)
O =		-O
OPT =		$(ROOT)/$(PFX)/lib/$(PFX)optim
OPTIONS =	-DIBMAT -DLCCFIX -DPICFIX1 -DPICFIX3 -DPICFIX4 $(ATMERGE)
#OPTIONS =	-DIBMAT -DLCCFIX -DATMERGE -DATPROBE
SRC =		../../iAPX286
SRCDIR =	$(SRC)/$(DIR)

CFILES = *.c

FILES =\
	$(LIBNAME)(acct.o)\
	$(LIBNAME)(alloc.o)\
	$(LIBNAME)(clock.o)\
	$(LIBNAME)(dos.o)\
	$(LIBNAME)(errlog.o)\
	$(LIBNAME)(fio.o)\
	$(LIBNAME)(flock.o)\
	$(LIBNAME)(fp.o)\
	$(LIBNAME)(iget.o)\
	$(LIBNAME)(ipc.o)\
	$(LIBNAME)(ldt.o)\
	$(LIBNAME)(lock.o)\
	$(LIBNAME)(machdep.o)\
	$(LIBNAME)(macherr.o)\
	$(LIBNAME)(main.o)\
	$(LIBNAME)(malloc.o)\
	$(LIBNAME)(msg.o)\
	$(LIBNAME)(nami.o)\
	$(LIBNAME)(pipe.o)\
	$(LIBNAME)(prf.o)\
	$(LIBNAME)(rdwri.o)\
	$(LIBNAME)(sem.o)\
	$(LIBNAME)(shm.o)\
	$(LIBNAME)(sig.o)\
	$(LIBNAME)(sigcode.o)\
	$(LIBNAME)(slp.o)\
	$(LIBNAME)(spl.o)\
	$(LIBNAME)(subr.o)\
	$(LIBNAME)(sys1.o)\
	$(LIBNAME)(sys2.o)\
	$(LIBNAME)(sys3.o)\
	$(LIBNAME)(sys4.o)\
	$(LIBNAME)(sysent.o)\
	$(LIBNAME)(text.o)\
	$(LIBNAME)(trap.o)\
	$(LIBNAME)(userio.o)\
	$(LIBNAME)(utssys.o)

.PRECIOUS:	$(LIBNAME)

#.c.a:
#	$(CC) $(CFLAGS)  $*.c
#	$(AR) rv $(LIBNAME) $*.o

$(LIBNAME):	$(FILES)
#	-chgrp bin $(LIBNAME)
#	-chmod 664 $(LIBNAME)
#	-chown bin $(LIBNAME)

clean:
	-rm -f *.[so]

clobber:	clean
	-rm -f $(LIBNAME)

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
 	    $(CPP) $(SRCDIR)/$$i | \
	     grep -v "/usr/include" | \
	     awk '/^# 1 /{printf "\\\n\t%s",$$3};END{print;print "\t$$(CC) -c $$(CFLAGS) $$(SRCDIR)/'$$i'";print "\tar rv $$(LIBNAME) '$$file.o'";print}'|\
 	     sed 's/"//g'; \
 	done >> .deplist; \
 	echo '\n' >> .filelist; \
 	echo '/^FILES /\n.,/^$$/d\n-1r .filelist\n/^# DO NOT DELETE/,$$d\nr .deplist\nw $(MAKENEW)\nq' | ed - $(MAKEFILE)
	mv $(MAKEFILE) $(MAKEOLD)
	mv $(MAKENEW) $(MAKEFILE)
	ln $(MAKEFILE) IBMAT.mk
	rm -f .deplist .filelist

# anything after the next line will disappear!
# DO NOT DELETE THIS LINE - make depend uses it

# Dependencies generated on Mon May 11 14:01:06 PDT 1987
$(LIBNAME)(acct.o): \
	../../iAPX286/os/acct.c\
	../../iAPX286/sys/param.h\
	../../iAPX286/sys/types.h\
	../../iAPX286/sys/systm.h\
	../../iAPX286/sys/user.h\
	../../iAPX286/sys/tss.h\
	../../iAPX286/sys/inode.h
	$(CC) -c $(CFLAGS) $(SRCDIR)/acct.c
	ar rv $(LIBNAME) acct.o

$(LIBNAME)(alloc.o): \
	../../iAPX286/os/alloc.c\
	../../iAPX286/sys/param.h\
	../../iAPX286/sys/types.h\
	../../iAPX286/sys/systm.h\
	../../iAPX286/sys/filsys.h\
	../../iAPX286/sys/buf.h\
	../../iAPX286/sys/inode.h\
	../../iAPX286/sys/user.h\
	../../iAPX286/sys/tss.h\
	../../iAPX286/sys/var.h
	$(CC) -c $(CFLAGS) $(SRCDIR)/alloc.c
	ar rv $(LIBNAME) alloc.o

$(LIBNAME)(clock.o): \
	../../iAPX286/os/clock.c\
	../../iAPX286/sys/param.h\
	../../iAPX286/sys/types.h\
	../../iAPX286/sys/systm.h\
	../../iAPX286/sys/sysinfo.h\
	../../iAPX286/sys/callo.h\
	../../iAPX286/sys/user.h\
	../../iAPX286/sys/tss.h\
	../../iAPX286/sys/proc.h\
	../../iAPX286/sys/text.h\
	../../iAPX286/sys/psl.h\
	../../iAPX286/sys/var.h\
	../../iAPX286/sys/mmu.h\
	../../iAPX286/sys/reg.h\
	../../iAPX286/sys/trap.h\
	../../iAPX286/sys/realmode.h
	$(CC) -c $(CFLAGS) $(SRCDIR)/clock.c
	ar rv $(LIBNAME) clock.o

$(LIBNAME)(dos.o): \
	../../iAPX286/os/dos.c\
	../../iAPX286/sys/param.h\
	../../iAPX286/sys/types.h\
	../../iAPX286/sys/systm.h\
	../../iAPX286/sys/user.h\
	../../iAPX286/sys/tss.h\
	../../iAPX286/sys/proc.h\
	../../iAPX286/sys/conf.h\
	../../iAPX286/sys/map.h\
	../../iAPX286/sys/buf.h\
	../../iAPX286/sys/tty.h\
	../../iAPX286/os/../sys/realmode.h\
	../../iAPX286/sys/mmu.h\
	../../iAPX286/sys/seg.h\
	../../iAPX286/sys/sxt.h\
	../../iAPX286/sys/8259.h
	$(CC) -c $(CFLAGS) $(SRCDIR)/dos.c
	ar rv $(LIBNAME) dos.o

$(LIBNAME)(errlog.o): \
	../../iAPX286/os/errlog.c\
	../../iAPX286/sys/param.h\
	../../iAPX286/sys/types.h\
	../../iAPX286/sys/systm.h\
	../../iAPX286/sys/buf.h\
	../../iAPX286/sys/conf.h\
	../../iAPX286/sys/map.h\
	../../iAPX286/sys/iobuf.h\
	../../iAPX286/sys/var.h
	$(CC) -c $(CFLAGS) $(SRCDIR)/errlog.c
	ar rv $(LIBNAME) errlog.o

$(LIBNAME)(fio.o): \
	../../iAPX286/os/fio.c\
	../../iAPX286/sys/param.h\
	../../iAPX286/sys/types.h\
	../../iAPX286/sys/systm.h\
	../../iAPX286/sys/user.h\
	../../iAPX286/sys/tss.h\
	../../iAPX286/sys/filsys.h\
	../../iAPX286/sys/conf.h\
	../../iAPX286/sys/inode.h\
	../../iAPX286/sys/var.h\
	../../iAPX286/sys/sysinfo.h
	$(CC) -c $(CFLAGS) $(SRCDIR)/fio.c
	ar rv $(LIBNAME) fio.o

$(LIBNAME)(flock.o): \
	../../iAPX286/os/flock.c\
	../../iAPX286/sys/types.h\
	../../iAPX286/sys/param.h\
	../../iAPX286/sys/inode.h\
	../../iAPX286/sys/proc.h\
	../../iAPX286/sys/user.h\
	../../iAPX286/sys/tss.h\
	../../iAPX286/sys/flock.h
	$(CC) -c $(CFLAGS) $(SRCDIR)/flock.c
	ar rv $(LIBNAME) flock.o

$(LIBNAME)(fp.o): \
	../../iAPX286/os/fp.c\
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
	../../iAPX286/sys/fp.h
	$(CC) -c $(CFLAGS) $(SRCDIR)/fp.c
	ar rv $(LIBNAME) fp.o

$(LIBNAME)(iget.o): \
	../../iAPX286/os/iget.c\
	../../iAPX286/sys/types.h\
	../../iAPX286/sys/param.h\
	../../iAPX286/sys/systm.h\
	../../iAPX286/sys/sysinfo.h\
	../../iAPX286/sys/user.h\
	../../iAPX286/sys/tss.h\
	../../iAPX286/sys/inode.h\
	../../iAPX286/sys/filsys.h\
	../../iAPX286/sys/buf.h\
	../../iAPX286/sys/var.h
	$(CC) -c $(CFLAGS) $(SRCDIR)/iget.c
	ar rv $(LIBNAME) iget.o

$(LIBNAME)(ipc.o): \
	../../iAPX286/os/ipc.c\
	../../iAPX286/sys/types.h\
	../../iAPX286/sys/param.h\
	../../iAPX286/sys/seg.h\
	../../iAPX286/sys/proc.h\
	../../iAPX286/sys/user.h\
	../../iAPX286/sys/tss.h\
	../../iAPX286/sys/ipc.h
	$(CC) -c $(CFLAGS) $(SRCDIR)/ipc.c
	ar rv $(LIBNAME) ipc.o

$(LIBNAME)(ldt.o): \
	../../iAPX286/os/ldt.c\
	../../iAPX286/sys/param.h\
	../../iAPX286/sys/types.h\
	../../iAPX286/sys/systm.h\
	../../iAPX286/sys/user.h\
	../../iAPX286/sys/tss.h\
	../../iAPX286/sys/proc.h\
	../../iAPX286/sys/seg.h\
	../../iAPX286/sys/mmu.h\
	../../iAPX286/sys/fp.h
	$(CC) -c $(CFLAGS) $(SRCDIR)/ldt.c
	ar rv $(LIBNAME) ldt.o

$(LIBNAME)(lock.o): \
	../../iAPX286/os/lock.c\
	../../iAPX286/sys/param.h\
	../../iAPX286/sys/types.h\
	../../iAPX286/sys/proc.h\
	../../iAPX286/sys/user.h\
	../../iAPX286/sys/tss.h\
	../../iAPX286/sys/text.h
	$(CC) -c $(CFLAGS) $(SRCDIR)/lock.c
	ar rv $(LIBNAME) lock.o

$(LIBNAME)(machdep.o): \
	../../iAPX286/os/machdep.c\
	../../iAPX286/sys/param.h\
	../../iAPX286/sys/types.h\
	../../iAPX286/sys/systm.h\
	../../iAPX286/sys/map.h\
	../../iAPX286/sys/user.h\
	../../iAPX286/sys/tss.h\
	../../iAPX286/sys/proc.h\
	../../iAPX286/sys/seg.h\
	../../iAPX286/sys/mmu.h\
	../../iAPX286/sys/reg.h\
	../../iAPX286/sys/psl.h\
	../../iAPX286/sys/8259.h\
	../../iAPX286/sys/fp.h\
	../../iAPX286/sys/ppi.h\
	../../iAPX286/sys/uadmin.h\
	../../iAPX286/sys/realmode.h
	$(CC) -c $(CFLAGS) $(SRCDIR)/machdep.c
	ar rv $(LIBNAME) machdep.o

$(LIBNAME)(macherr.o): \
	../../iAPX286/os/macherr.c
	$(CC) -c $(CFLAGS) $(SRCDIR)/macherr.c
	ar rv $(LIBNAME) macherr.o

$(LIBNAME)(main.o): \
	../../iAPX286/os/main.c\
	../../iAPX286/sys/param.h\
	../../iAPX286/sys/types.h\
	../../iAPX286/sys/systm.h\
	../../iAPX286/sys/user.h\
	../../iAPX286/sys/tss.h\
	../../iAPX286/sys/filsys.h\
	../../iAPX286/sys/proc.h\
	../../iAPX286/sys/inode.h\
	../../iAPX286/sys/seg.h\
	../../iAPX286/sys/mmu.h\
	../../iAPX286/sys/conf.h\
	../../iAPX286/sys/buf.h\
	../../iAPX286/sys/iobuf.h\
	../../iAPX286/sys/tty.h\
	../../iAPX286/sys/var.h\
	../../iAPX286/sys/clock.h\
	../../iAPX286/sys/map.h\
	../../iAPX286/sys/realmode.h
	$(CC) -c $(CFLAGS) $(SRCDIR)/main.c
	ar rv $(LIBNAME) main.o

$(LIBNAME)(malloc.o): \
	../../iAPX286/os/malloc.c\
	../../iAPX286/sys/param.h\
	../../iAPX286/sys/types.h\
	../../iAPX286/sys/realmode.h\
	../../iAPX286/sys/systm.h\
	../../iAPX286/sys/map.h
	$(CC) -c $(CFLAGS) $(SRCDIR)/malloc.c
	ar rv $(LIBNAME) malloc.o

$(LIBNAME)(msg.o): \
	../../iAPX286/os/msg.c\
	../../iAPX286/sys/types.h\
	../../iAPX286/sys/param.h\
	../../iAPX286/sys/user.h\
	../../iAPX286/sys/tss.h\
	../../iAPX286/sys/seg.h\
	../../iAPX286/sys/proc.h\
	../../iAPX286/sys/buf.h\
	../../iAPX286/sys/map.h\
	../../iAPX286/sys/ipc.h\
	../../iAPX286/sys/systm.h\
	../../iAPX286/sys/mmu.h
	$(CC) -c $(CFLAGS) $(SRCDIR)/msg.c
	ar rv $(LIBNAME) msg.o

$(LIBNAME)(nami.o): \
	../../iAPX286/os/nami.c\
	../../iAPX286/sys/param.h\
	../../iAPX286/sys/types.h\
	../../iAPX286/sys/systm.h\
	../../iAPX286/sys/sysinfo.h\
	../../iAPX286/sys/inode.h\
	../../iAPX286/sys/user.h\
	../../iAPX286/sys/tss.h\
	../../iAPX286/sys/buf.h\
	../../iAPX286/sys/var.h
	$(CC) -c $(CFLAGS) $(SRCDIR)/nami.c
	ar rv $(LIBNAME) nami.o

$(LIBNAME)(pipe.o): \
	../../iAPX286/os/pipe.c\
	../../iAPX286/sys/param.h\
	../../iAPX286/sys/types.h\
	../../iAPX286/sys/systm.h\
	../../iAPX286/sys/user.h\
	../../iAPX286/sys/tss.h\
	../../iAPX286/sys/inode.h
	$(CC) -c $(CFLAGS) $(SRCDIR)/pipe.c
	ar rv $(LIBNAME) pipe.o

$(LIBNAME)(prf.o): \
	../../iAPX286/os/prf.c\
	../../iAPX286/sys/param.h\
	../../iAPX286/sys/types.h\
	../../iAPX286/sys/systm.h\
	../../iAPX286/sys/buf.h\
	../../iAPX286/sys/conf.h
	$(CC) -c $(CFLAGS) $(SRCDIR)/prf.c
	ar rv $(LIBNAME) prf.o

$(LIBNAME)(rdwri.o): \
	../../iAPX286/os/rdwri.c\
	../../iAPX286/sys/param.h\
	../../iAPX286/sys/types.h\
	../../iAPX286/sys/inode.h\
	../../iAPX286/sys/user.h\
	../../iAPX286/sys/tss.h\
	../../iAPX286/sys/buf.h\
	../../iAPX286/sys/conf.h\
	../../iAPX286/sys/systm.h\
	../../iAPX286/sys/tty.h\
	../../iAPX286/sys/mmu.h
	$(CC) -c $(CFLAGS) $(SRCDIR)/rdwri.c
	ar rv $(LIBNAME) rdwri.o

$(LIBNAME)(sem.o): \
	../../iAPX286/os/sem.c\
	../../iAPX286/sys/types.h\
	../../iAPX286/sys/param.h\
	../../iAPX286/sys/map.h\
	../../iAPX286/sys/ipc.h\
	../../iAPX286/sys/user.h\
	../../iAPX286/sys/tss.h\
	../../iAPX286/sys/seg.h\
	../../iAPX286/sys/proc.h\
	../../iAPX286/sys/buf.h
	$(CC) -c $(CFLAGS) $(SRCDIR)/sem.c
	ar rv $(LIBNAME) sem.o

$(LIBNAME)(shm.o): \
	../../iAPX286/os/shm.c\
	../../iAPX286/sys/types.h\
	../../iAPX286/sys/param.h\
	../../iAPX286/sys/user.h\
	../../iAPX286/sys/tss.h\
	../../iAPX286/sys/seg.h\
	../../iAPX286/sys/ipc.h\
	../../iAPX286/sys/shm.h\
	../../iAPX286/sys/proc.h\
	../../iAPX286/sys/systm.h\
	../../iAPX286/sys/mmu.h\
	../../iAPX286/sys/map.h
	$(CC) -c $(CFLAGS) $(SRCDIR)/shm.c
	ar rv $(LIBNAME) shm.o

$(LIBNAME)(sig.o): \
	../../iAPX286/os/sig.c\
	../../iAPX286/sys/param.h\
	../../iAPX286/sys/types.h\
	../../iAPX286/sys/mmu.h\
	../../iAPX286/sys/systm.h\
	../../iAPX286/sys/user.h\
	../../iAPX286/sys/tss.h\
	../../iAPX286/sys/proc.h\
	../../iAPX286/sys/inode.h\
	../../iAPX286/sys/reg.h\
	../../iAPX286/sys/text.h\
	../../iAPX286/sys/seg.h\
	../../iAPX286/sys/var.h\
	../../iAPX286/sys/psl.h\
	../../iAPX286/sys/fp.h
	$(CC) -c $(CFLAGS) $(SRCDIR)/sig.c
	ar rv $(LIBNAME) sig.o

$(LIBNAME)(sigcode.o): \
	../../iAPX286/os/sigcode.c\
	../../iAPX286/sys/param.h\
	../../iAPX286/sys/types.h\
	../../iAPX286/sys/proc.h\
	../../iAPX286/sys/user.h\
	../../iAPX286/sys/tss.h\
	../../iAPX286/sys/reg.h\
	../../iAPX286/sys/psl.h
	$(CC) -c $(CFLAGS) $(SRCDIR)/sigcode.c
	ar rv $(LIBNAME) sigcode.o

$(LIBNAME)(slp.o): \
	../../iAPX286/os/slp.c\
	../../iAPX286/sys/param.h\
	../../iAPX286/sys/types.h\
	../../iAPX286/sys/user.h\
	../../iAPX286/sys/tss.h\
	../../iAPX286/sys/proc.h\
	../../iAPX286/sys/mmu.h\
	../../iAPX286/sys/seg.h\
	../../iAPX286/sys/text.h\
	../../iAPX286/sys/systm.h\
	../../iAPX286/sys/sysinfo.h\
	../../iAPX286/sys/map.h\
	../../iAPX286/sys/inode.h\
	../../iAPX286/sys/buf.h\
	../../iAPX286/sys/var.h\
	../../iAPX286/sys/fp.h\
	../../iAPX286/sys/uadmin.h\
	../../iAPX286/sys/realmode.h
	$(CC) -c $(CFLAGS) $(SRCDIR)/slp.c
	ar rv $(LIBNAME) slp.o

$(LIBNAME)(spl.o): \
	../../iAPX286/os/spl.c\
	../../iAPX286/sys/8259.h
	$(CC) -c $(CFLAGS) $(SRCDIR)/spl.c
	ar rv $(LIBNAME) spl.o

$(LIBNAME)(subr.o): \
	../../iAPX286/os/subr.c\
	../../iAPX286/sys/param.h\
	../../iAPX286/sys/types.h\
	../../iAPX286/sys/systm.h\
	../../iAPX286/sys/inode.h\
	../../iAPX286/sys/user.h\
	../../iAPX286/sys/tss.h\
	../../iAPX286/sys/buf.h\
	../../iAPX286/sys/var.h
	$(CC) -c $(CFLAGS) $(SRCDIR)/subr.c
	ar rv $(LIBNAME) subr.o

$(LIBNAME)(sys1.o): \
	../../iAPX286/os/sys1.c\
	../../iAPX286/sys/param.h\
	../../iAPX286/sys/types.h\
	../../iAPX286/sys/systm.h\
	../../iAPX286/sys/map.h\
	../../iAPX286/sys/user.h\
	../../iAPX286/sys/tss.h\
	../../iAPX286/sys/proc.h\
	../../iAPX286/sys/buf.h\
	../../iAPX286/sys/reg.h\
	../../iAPX286/sys/inode.h\
	../../iAPX286/sys/seg.h\
	../../iAPX286/sys/mmu.h\
	../../iAPX286/sys/sysinfo.h\
	../../iAPX286/sys/var.h\
	../../iAPX286/sys/ipc.h\
	../../iAPX286/sys/shm.h\
	../../iAPX286/sys/text.h\
	../../iAPX286/sys/realmode.h
	$(CC) -c $(CFLAGS) $(SRCDIR)/sys1.c
	ar rv $(LIBNAME) sys1.o

$(LIBNAME)(sys2.o): \
	../../iAPX286/os/sys2.c\
	../../iAPX286/sys/param.h\
	../../iAPX286/sys/types.h\
	../../iAPX286/sys/systm.h\
	../../iAPX286/sys/user.h\
	../../iAPX286/sys/tss.h\
	../../iAPX286/sys/inode.h\
	../../iAPX286/sys/sysinfo.h
	$(CC) -c $(CFLAGS) $(SRCDIR)/sys2.c
	ar rv $(LIBNAME) sys2.o

$(LIBNAME)(sys3.o): \
	../../iAPX286/os/sys3.c\
	../../iAPX286/sys/param.h\
	../../iAPX286/sys/types.h\
	../../iAPX286/sys/systm.h\
	../../iAPX286/sys/buf.h\
	../../iAPX286/sys/filsys.h\
	../../iAPX286/sys/user.h\
	../../iAPX286/sys/tss.h\
	../../iAPX286/sys/inode.h\
	../../iAPX286/sys/conf.h\
	../../iAPX286/sys/var.h\
	../../iAPX286/sys/flock.h
	$(CC) -c $(CFLAGS) $(SRCDIR)/sys3.c
	ar rv $(LIBNAME) sys3.o

$(LIBNAME)(sys4.o): \
	../../iAPX286/os/sys4.c\
	../../iAPX286/sys/param.h\
	../../iAPX286/sys/types.h\
	../../iAPX286/sys/systm.h\
	../../iAPX286/sys/user.h\
	../../iAPX286/sys/tss.h\
	../../iAPX286/sys/inode.h\
	../../iAPX286/sys/proc.h\
	../../iAPX286/sys/var.h\
	../../iAPX286/sys/mmu.h\
	../../iAPX286/sys/seg.h
	$(CC) -c $(CFLAGS) $(SRCDIR)/sys4.c
	ar rv $(LIBNAME) sys4.o

$(LIBNAME)(sysent.o): \
	../../iAPX286/os/sysent.c\
	../../iAPX286/sys/param.h\
	../../iAPX286/sys/types.h\
	../../iAPX286/sys/systm.h\
	../../iAPX286/sys/realmode.h
	$(CC) -c $(CFLAGS) $(SRCDIR)/sysent.c
	ar rv $(LIBNAME) sysent.o

$(LIBNAME)(text.o): \
	../../iAPX286/os/text.c\
	../../iAPX286/sys/param.h\
	../../iAPX286/sys/types.h\
	../../iAPX286/sys/systm.h\
	../../iAPX286/sys/map.h\
	../../iAPX286/sys/user.h\
	../../iAPX286/sys/tss.h\
	../../iAPX286/sys/proc.h\
	../../iAPX286/sys/text.h\
	../../iAPX286/sys/inode.h\
	../../iAPX286/sys/buf.h\
	../../iAPX286/sys/seg.h\
	../../iAPX286/sys/mmu.h\
	../../iAPX286/sys/var.h\
	../../iAPX286/sys/sysinfo.h\
	../../iAPX286/sys/fp.h\
	../../iAPX286/sys/realmode.h
	$(CC) -c $(CFLAGS) $(SRCDIR)/text.c
	ar rv $(LIBNAME) text.o

$(LIBNAME)(trap.o): \
	../../iAPX286/os/trap.c\
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
	$(CC) -c $(CFLAGS) $(SRCDIR)/trap.c
	ar rv $(LIBNAME) trap.o

$(LIBNAME)(userio.o): \
	../../iAPX286/os/userio.c\
	../../iAPX286/sys/param.h\
	../../iAPX286/sys/types.h\
	../../iAPX286/sys/systm.h\
	../../iAPX286/sys/user.h\
	../../iAPX286/sys/tss.h\
	../../iAPX286/sys/seg.h\
	../../iAPX286/sys/mmu.h
	$(CC) -c $(CFLAGS) $(SRCDIR)/userio.c
	ar rv $(LIBNAME) userio.o

$(LIBNAME)(utssys.o): \
	../../iAPX286/os/utssys.c\
	../../iAPX286/sys/types.h\
	../../iAPX286/sys/param.h\
	../../iAPX286/sys/systm.h\
	../../iAPX286/sys/buf.h\
	../../iAPX286/sys/filsys.h\
	../../iAPX286/sys/user.h\
	../../iAPX286/sys/tss.h\
	../../iAPX286/sys/inode.h\
	../../iAPX286/sys/proc.h\
	../../iAPX286/sys/var.h\
	../../iAPX286/sys/uadmin.h
	$(CC) -c $(CFLAGS) $(SRCDIR)/utssys.c
	ar rv $(LIBNAME) utssys.o

