#	Copyright (c) 1985 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

# @(#)os.mk	1.7 - 85/09/03
LIBNAME = ../lib1
INCRT = $(ROOT)/usr/include
CC=cc
AR=ar
CFLAGS = -I$(INCRT) -Ml -c
OPT = $(ROOT)/$(PFX)/lib/$(PFX)optim
FRC =
FILES =\
	$(LIBNAME)(acct.o)\
	$(LIBNAME)(alloc.o)\
	$(LIBNAME)(clock.o)\
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

.c.a:
	$(CC) $(CFLAGS) -S $*.c
	-sed -f ../SEDaa $*.s |\
		sed -f ../SEDab |\
		sed -f ../SEDac |\
		sed -f ../SEDad > temp
	-$(OPT) < temp > $*.s
	$(CC) $(CFLAGS) $*.s
	-rm -f temp
	$(AR) rv $(LIBNAME) $*.o

$(LIBNAME):	$(FILES)
	-chgrp bin $(LIBNAME)
	-chmod 664 $(LIBNAME)
	-chown bin $(LIBNAME)

clean:
	-rm -f *.o

clobber:	clean
	-rm -f $(LIBNAME)

acct.o: $(INCRT)/sys/param.h\
	$(INCRT)/sys/types.h\
	$(INCRT)/sys/systm.h\
	$(INCRT)/sys/acct.h\
	$(INCRT)/sys/dir.h\
	$(INCRT)/sys/signal.h\
	$(INCRT)/sys/user.h\
	$(INCRT)/sys/errno.h\
	$(INCRT)/sys/inode.h\
	$(INCRT)/sys/file.h\
	$(FRC)

alloc.o: $(INCRT)/sys/param.h\
	$(INCRT)/sys/types.h\
	$(INCRT)/sys/sysmacros.h\
	$(INCRT)/sys/systm.h\
	$(INCRT)/sys/mount.h\
	$(INCRT)/sys/filsys.h\
	$(INCRT)/sys/fblk.h\
	$(INCRT)/sys/buf.h\
	$(INCRT)/sys/inode.h\
	$(INCRT)/sys/file.h\
	$(INCRT)/sys/ino.h\
	$(INCRT)/sys/dir.h\
	$(INCRT)/sys/signal.h\
	$(INCRT)/sys/user.h\
	$(INCRT)/sys/errno.h\
	$(INCRT)/sys/var.h\
	$(FRC)

clock.o: $(INCRT)/sys/param.h\
	$(INCRT)/sys/types.h\
	$(INCRT)/sys/sysmacros.h\
	$(INCRT)/sys/systm.h\
	$(INCRT)/sys/sysinfo.h\
	$(INCRT)/sys/callo.h\
	$(INCRT)/sys/dir.h\
	$(INCRT)/sys/signal.h\
	$(INCRT)/sys/user.h\
	$(INCRT)/sys/proc.h\
	$(INCRT)/sys/text.h\
	$(INCRT)/sys/psl.h\
	$(INCRT)/sys/var.h\
	$(INCRT)/sys/mmu.h\
	$(INCRT)/sys/reg.h\
	$(FRC)

errlog.o: $(INCRT)/sys/param.h\
	$(INCRT)/sys/types.h\
	$(INCRT)/sys/sysmacros.h\
	$(INCRT)/sys/systm.h\
	$(INCRT)/sys/buf.h\
	$(INCRT)/sys/conf.h\
	$(INCRT)/sys/map.h\
	$(INCRT)/sys/utsname.h\
	$(INCRT)/sys/elog.h\
	$(INCRT)/sys/erec.h\
	$(INCRT)/sys/err.h\
	$(INCRT)/sys/iobuf.h\
	$(INCRT)/sys/var.h\
	$(FRC)

fio.o: $(INCRT)/sys/param.h\
	$(INCRT)/sys/types.h\
	$(INCRT)/sys/sysmacros.h\
	$(INCRT)/sys/systm.h\
	$(INCRT)/sys/dir.h\
	$(INCRT)/sys/signal.h\
	$(INCRT)/sys/user.h\
	$(INCRT)/sys/errno.h\
	$(INCRT)/sys/filsys.h\
	$(INCRT)/sys/file.h\
	$(INCRT)/sys/conf.h\
	$(INCRT)/sys/inode.h\
	$(INCRT)/sys/mount.h\
	$(INCRT)/sys/var.h\
	$(INCRT)/sys/acct.h\
	$(INCRT)/sys/sysinfo.h\
	$(FRC)

iget.o: $(INCRT)/sys/param.h\
	$(INCRT)/sys/types.h\
	$(INCRT)/sys/sysmacros.h\
	$(INCRT)/sys/systm.h\
	$(INCRT)/sys/sysinfo.h\
	$(INCRT)/sys/mount.h\
	$(INCRT)/sys/dir.h\
	$(INCRT)/sys/signal.h\
	$(INCRT)/sys/user.h\
	$(INCRT)/sys/errno.h\
	$(INCRT)/sys/inode.h\
	$(INCRT)/sys/file.h\
	$(INCRT)/sys/ino.h\
	$(INCRT)/sys/filsys.h\
	$(INCRT)/sys/buf.h\
	$(INCRT)/sys/var.h\
	$(FRC)

ipc.o: $(INCRT)/sys/errno.h\
	$(INCRT)/sys/types.h\
	$(INCRT)/sys/param.h\
	$(INCRT)/sys/seg.h\
	$(INCRT)/sys/signal.h\
	$(INCRT)/sys/proc.h\
	$(INCRT)/sys/dir.h\
	$(INCRT)/sys/user.h\
	$(INCRT)/sys/ipc.h\
	$(FRC)

ldt.o: $(INCRT)/sys/param.h\
	$(INCRT)/sys/types.h\
	$(INCRT)/sys/sysmacros.h\
	$(INCRT)/sys/systm.h\
	$(INCRT)/sys/dir.h\
	$(INCRT)/sys/signal.h\
	$(INCRT)/sys/user.h\
	$(INCRT)/sys/errno.h\
	$(INCRT)/sys/proc.h\
	$(INCRT)/sys/seg.h\
	$(INCRT)/sys/mmu.h\
	$(FRC)

lock.o: $(INCRT)/sys/param.h\
	$(INCRT)/sys/types.h\
	$(INCRT)/sys/proc.h\
	$(INCRT)/sys/dir.h\
	$(INCRT)/sys/signal.h\
	$(INCRT)/sys/user.h\
	$(INCRT)/sys/errno.h\
	$(INCRT)/sys/text.h\
	$(INCRT)/sys/lock.h\
	$(FRC)

machdep.o: $(INCRT)/sys/param.h\
	$(INCRT)/sys/types.h\
	$(INCRT)/sys/sysmacros.h\
	$(INCRT)/sys/systm.h\
	$(INCRT)/sys/map.h\
	$(INCRT)/sys/dir.h\
	$(INCRT)/sys/signal.h\
	$(INCRT)/sys/user.h\
	$(INCRT)/sys/errno.h\
	$(INCRT)/sys/proc.h\
	$(INCRT)/sys/seg.h\
	$(INCRT)/sys/mmu.h\
	$(INCRT)/sys/reg.h\
	$(INCRT)/sys/psl.h\
	$(INCRT)/sys/utsname.h\
	$(FRC)

main.o: $(INCRT)/sys/param.h\
	$(INCRT)/sys/types.h\
	$(INCRT)/sys/sysmacros.h\
	$(INCRT)/sys/systm.h\
	$(INCRT)/sys/dir.h\
	$(INCRT)/sys/signal.h\
	$(INCRT)/sys/user.h\
	$(INCRT)/sys/filsys.h\
	$(INCRT)/sys/mount.h\
	$(INCRT)/sys/proc.h\
	$(INCRT)/sys/inode.h\
	$(INCRT)/sys/seg.h\
	$(INCRT)/sys/conf.h\
	$(INCRT)/sys/buf.h\
	$(INCRT)/sys/iobuf.h\
	$(INCRT)/sys/tty.h\
	$(INCRT)/sys/var.h\
	$(FRC)

malloc.o: $(INCRT)/sys/param.h\
	$(INCRT)/sys/types.h\
	$(INCRT)/sys/systm.h\
	$(INCRT)/sys/map.h\
	$(FRC)

msg.o: $(INCRT)/sys/types.h\
	$(INCRT)/sys/param.h\
	$(INCRT)/sys/dir.h\
	$(INCRT)/sys/signal.h\
	$(INCRT)/sys/user.h\
	$(INCRT)/sys/seg.h\
	$(INCRT)/sys/proc.h\
	$(INCRT)/sys/buf.h\
	$(INCRT)/sys/errno.h\
	$(INCRT)/sys/map.h\
	$(INCRT)/sys/ipc.h\
	$(INCRT)/sys/msg.h\
	$(INCRT)/sys/systm.h\
	$(INCRT)/sys/sysmacros.h\
	$(INCRT)/sys/mmu.h\
	$(FRC)

nami.o: $(INCRT)/sys/param.h\
	$(INCRT)/sys/types.h\
	$(INCRT)/sys/systm.h\
	$(INCRT)/sys/sysinfo.h\
	$(INCRT)/sys/inode.h\
	$(INCRT)/sys/mount.h\
	$(INCRT)/sys/dir.h\
	$(INCRT)/sys/signal.h\
	$(INCRT)/sys/user.h\
	$(INCRT)/sys/errno.h\
	$(INCRT)/sys/buf.h\
	$(INCRT)/sys/var.h\
	$(FRC)

pipe.o: $(INCRT)/sys/param.h\
	$(INCRT)/sys/types.h\
	$(INCRT)/sys/systm.h\
	$(INCRT)/sys/dir.h\
	$(INCRT)/sys/signal.h\
	$(INCRT)/sys/user.h\
	$(INCRT)/sys/errno.h\
	$(INCRT)/sys/inode.h\
	$(INCRT)/sys/file.h\
	$(FRC)

prf.o: $(INCRT)/sys/param.h\
	$(INCRT)/sys/types.h\
	$(INCRT)/sys/sysmacros.h\
	$(INCRT)/sys/systm.h\
	$(INCRT)/sys/buf.h\
	$(INCRT)/sys/conf.h\
	$(FRC)

rdwri.o: $(INCRT)/sys/param.h\
	$(INCRT)/sys/types.h\
	$(INCRT)/sys/sysmacros.h\
	$(INCRT)/sys/inode.h\
	$(INCRT)/sys/dir.h\
	$(INCRT)/sys/signal.h\
	$(INCRT)/sys/user.h\
	$(INCRT)/sys/errno.h\
	$(INCRT)/sys/buf.h\
	$(INCRT)/sys/conf.h\
	$(INCRT)/sys/file.h\
	$(INCRT)/sys/systm.h\
	$(INCRT)/sys/tty.h\
	$(INCRT)/sys/mmu.h\
	$(FRC)

text.o: $(INCRT)/sys/param.h\
	$(INCRT)/sys/types.h\
	$(INCRT)/sys/sysmacros.h\
	$(INCRT)/sys/systm.h\
	$(INCRT)/sys/map.h\
	$(INCRT)/sys/dir.h\
	$(INCRT)/sys/signal.h\
	$(INCRT)/sys/user.h\
	$(INCRT)/sys/proc.h\
	$(INCRT)/sys/text.h\
	$(INCRT)/sys/inode.h\
	$(INCRT)/sys/buf.h\
	$(INCRT)/sys/seg.h\
	$(INCRT)/sys/mmu.h\
	$(INCRT)/sys/var.h\
	$(INCRT)/sys/sysinfo.h\
	$(FRC)

trap.o: $(INCRT)/sys/param.h\
	$(INCRT)/sys/types.h\
	$(INCRT)/sys/systm.h\
	$(INCRT)/sys/dir.h\
	$(INCRT)/sys/signal.h\
	$(INCRT)/sys/user.h\
	$(INCRT)/sys/errno.h\
	$(INCRT)/sys/proc.h\
	$(INCRT)/sys/reg.h\
	$(INCRT)/sys/psl.h\
	$(INCRT)/sys/trap.h\
	$(INCRT)/sys/seg.h\
	$(INCRT)/sys/sysinfo.h\
	$(INCRT)/sys/mmu.h\
	$(FRC)

userio.o: $(INCRT)/sys/param.h\
	$(INCRT)/sys/types.h\
	$(INCRT)/sys/sysmacros.h\
	$(INCRT)/sys/systm.h\
	$(INCRT)/sys/dir.h\
	$(INCRT)/sys/signal.h\
	$(INCRT)/sys/user.h\
	$(INCRT)/sys/seg.h\
	$(INCRT)/sys/mmu.h\
	$(FRC)

utssys.o: $(INCRT)/sys/param.h\
	$(INCRT)/sys/types.h\
	$(INCRT)/sys/sysmacros.h\
	$(INCRT)/sys/buf.h\
	$(INCRT)/sys/filsys.h\
	$(INCRT)/sys/mount.h\
	$(INCRT)/sys/dir.h\
	$(INCRT)/sys/signal.h\
	$(INCRT)/sys/user.h\
	$(INCRT)/sys/errno.h\
	$(INCRT)/sys/var.h\
	$(INCRT)/sys/utsname.h\
	$(FRC)

sem.o: $(INCRT)/sys/types.h\
	$(INCRT)/sys/param.h\
	$(INCRT)/sys/dir.h\
	$(INCRT)/sys/map.h\
	$(INCRT)/sys/errno.h\
	$(INCRT)/sys/signal.h\
	$(INCRT)/sys/ipc.h\
	$(INCRT)/sys/sem.h\
	$(INCRT)/sys/user.h\
	$(INCRT)/sys/seg.h\
	$(INCRT)/sys/proc.h\
	$(INCRT)/sys/buf.h\
	$(FRC)

shm.o: $(INCRT)/sys/types.h\
	$(INCRT)/sys/param.h\
	$(INCRT)/sys/sysmacros.h\
	$(INCRT)/sys/dir.h\
	$(INCRT)/sys/errno.h\
	$(INCRT)/sys/signal.h\
	$(INCRT)/sys/user.h\
	$(INCRT)/sys/seg.h\
	$(INCRT)/sys/ipc.h\
	$(INCRT)/sys/shm.h\
	$(INCRT)/sys/proc.h\
	$(INCRT)/sys/systm.h\
	$(INCRT)/sys/mmu.h\
	$(INCRT)/sys/map.h\
	$(FRC)

sig.o: $(INCRT)/sys/param.h\
	$(INCRT)/sys/types.h\
	$(INCRT)/sys/sysmacros.h\
	$(INCRT)/sys/mmu.h\
	$(INCRT)/sys/systm.h\
	$(INCRT)/sys/dir.h\
	$(INCRT)/sys/signal.h\
	$(INCRT)/sys/user.h\
	$(INCRT)/sys/errno.h\
	$(INCRT)/sys/proc.h\
	$(INCRT)/sys/inode.h\
	$(INCRT)/sys/file.h\
	$(INCRT)/sys/reg.h\
	$(INCRT)/sys/text.h\
	$(INCRT)/sys/seg.h\
	$(INCRT)/sys/var.h\
	$(INCRT)/sys/psl.h\
	$(FRC)

slp.o: $(INCRT)/sys/param.h\
	$(INCRT)/sys/types.h\
	$(INCRT)/sys/sysmacros.h\
	$(INCRT)/sys/dir.h\
	$(INCRT)/sys/signal.h\
	$(INCRT)/sys/user.h\
	$(INCRT)/sys/proc.h\
	$(INCRT)/sys/seg.h\
	$(INCRT)/sys/text.h\
	$(INCRT)/sys/systm.h\
	$(INCRT)/sys/sysinfo.h\
	$(INCRT)/sys/map.h\
	$(INCRT)/sys/file.h\
	$(INCRT)/sys/inode.h\
	$(INCRT)/sys/buf.h\
	$(INCRT)/sys/var.h\
	$(INCRT)/sys/errno.h\
	$(FRC)

subr.o: $(INCRT)/sys/param.h\
	$(INCRT)/sys/types.h\
	$(INCRT)/sys/sysmacros.h\
	$(INCRT)/sys/systm.h\
	$(INCRT)/sys/inode.h\
	$(INCRT)/sys/dir.h\
	$(INCRT)/sys/signal.h\
	$(INCRT)/sys/user.h\
	$(INCRT)/sys/errno.h\
	$(INCRT)/sys/buf.h\
	$(INCRT)/sys/mount.h\
	$(INCRT)/sys/var.h\
	$(FRC)

sys1.o: $(INCRT)/sys/param.h\
	$(INCRT)/sys/types.h\
	$(INCRT)/sys/sysmacros.h\
	$(INCRT)/sys/systm.h\
	$(INCRT)/sys/map.h\
	$(INCRT)/sys/dir.h\
	$(INCRT)/sys/signal.h\
	$(INCRT)/sys/user.h\
	$(INCRT)/sys/errno.h\
	$(INCRT)/sys/proc.h\
	$(INCRT)/sys/buf.h\
	$(INCRT)/sys/reg.h\
	$(INCRT)/sys/file.h\
	$(INCRT)/sys/inode.h\
	$(INCRT)/sys/seg.h\
	$(INCRT)/sys/mmu.h\
	$(INCRT)/sys/acct.h\
	$(INCRT)/sys/sysinfo.h\
	$(INCRT)/sys/var.h\
	$(INCRT)/sys/ipc.h\
	$(INCRT)/sys/shm.h\
	$(FRC)

sys2.o: $(INCRT)/sys/param.h\
	$(INCRT)/sys/types.h\
	$(INCRT)/sys/systm.h\
	$(INCRT)/sys/dir.h\
	$(INCRT)/sys/signal.h\
	$(INCRT)/sys/user.h\
	$(INCRT)/sys/errno.h\
	$(INCRT)/sys/file.h\
	$(INCRT)/sys/inode.h\
	$(INCRT)/sys/sysinfo.h\
	$(FRC)

sys3.o: $(INCRT)/sys/param.h\
	$(INCRT)/sys/types.h\
	$(INCRT)/sys/sysmacros.h\
	$(INCRT)/sys/systm.h\
	$(INCRT)/sys/mount.h\
	$(INCRT)/sys/ino.h\
	$(INCRT)/sys/buf.h\
	$(INCRT)/sys/filsys.h\
	$(INCRT)/sys/dir.h\
	$(INCRT)/sys/signal.h\
	$(INCRT)/sys/user.h\
	$(INCRT)/sys/errno.h\
	$(INCRT)/sys/inode.h\
	$(INCRT)/sys/file.h\
	$(INCRT)/sys/conf.h\
	$(INCRT)/sys/stat.h\
	$(INCRT)/sys/ttold.h\
	$(INCRT)/sys/var.h\
	$(INCRT)/sys/ioctl.h\
	$(FRC)

sys4.o: $(INCRT)/sys/param.h\
	$(INCRT)/sys/types.h\
	$(INCRT)/sys/sysmacros.h\
	$(INCRT)/sys/systm.h\
	$(INCRT)/sys/dir.h\
	$(INCRT)/sys/signal.h\
	$(INCRT)/sys/user.h\
	$(INCRT)/sys/errno.h\
	$(INCRT)/sys/inode.h\
	$(INCRT)/sys/file.h\
	$(INCRT)/sys/proc.h\
	$(INCRT)/sys/var.h\
	$(INCRT)/sys/mmu.h\
	$(INCRT)/sys/seg.h\
	$(FRC)

sysent.o: $(INCRT)/sys/param.h\
	$(INCRT)/sys/types.h\
	$(INCRT)/sys/systm.h\
	$(FRC)

FRC:
