#	Copyright (c) 1985 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#       @(#)io.mk	1.7 - 85/09/06
LIBNAME = ../lib2
INCRT = $(ROOT)/usr/include
CC=cc
AR=ar
CFLAGS = -I$(INCRT) -Ml -c
OPT = $(ROOT)/$(PFX)/lib/$(PFX)optim
FRC =
FILES =\
	$(LIBNAME)(bio.o)\
	$(LIBNAME)(tt0.o)\
	$(LIBNAME)(tty.o)\
	$(LIBNAME)(err.o)\
	$(LIBNAME)(mem.o)\
	$(LIBNAME)(sxt.o)\
	$(LIBNAME)(sys.o)\
	$(LIBNAME)(partab.o)\
	$(LIBNAME)(clist.o)\
	$(LIBNAME)(console.o)\
	$(LIBNAME)(wn.o)\
	$(LIBNAME)(wn.subs.o)\
	$(LIBNAME)(wn.conf.o)\
	$(LIBNAME)(wn.dump.o)\
	$(LIBNAME)(sio.o)\
	$(LIBNAME)(cico.o)\
	$(LIBNAME)(c8274.o)\
	$(LIBNAME)(lp.o)

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

bio.o:\
	$(INCRT)/sys/param.h\
	$(INCRT)/sys/types.h\
	$(INCRT)/sys/sysmacros.h\
	$(INCRT)/sys/systm.h\
	$(INCRT)/sys/sysinfo.h\
	$(INCRT)/sys/dir.h\
	$(INCRT)/sys/signal.h\
	$(INCRT)/sys/user.h\
	$(INCRT)/sys/errno.h\
	$(INCRT)/sys/buf.h\
	$(INCRT)/sys/iobuf.h\
	$(INCRT)/sys/conf.h\
	$(INCRT)/sys/proc.h\
	$(INCRT)/sys/seg.h\
	$(INCRT)/sys/var.h\
	$(FRC)

clist.o:\
	$(INCRT)/sys/param.h\
	$(INCRT)/sys/types.h\
	$(INCRT)/sys/tty.h\
	$(FRC)

$(CON).o:\
	$(INCRT)/sys/param.h\
	$(INCRT)/sys/types.h\
	$(INCRT)/sys/dir.h\
	$(INCRT)/sys/signal.h\
	$(INCRT)/sys/user.h\
	$(INCRT)/sys/errno.h\
	$(INCRT)/sys/tty.h\
	$(INCRT)/sys/termio.h\
	$(INCRT)/sys/systm.h\
	$(INCRT)/sys/conf.h\
	$(INCRT)/sys/sysinfo.h\
	$(FRC)

err.o:\
	$(INCRT)/sys/param.h\
	$(INCRT)/sys/types.h\
	$(INCRT)/sys/buf.h\
	$(INCRT)/sys/dir.h\
	$(INCRT)/sys/signal.h\
	$(INCRT)/sys/user.h\
	$(INCRT)/sys/errno.h\
	$(INCRT)/sys/file.h\
	$(INCRT)/sys/utsname.h\
	$(INCRT)/sys/elog.h\
	$(INCRT)/sys/erec.h\
	$(FRC)

wn.conf.o:\
	$(INCRT)/sys/types.h\
	$(INCRT)/sys/param.h\
	$(INCRT)/sys/buf.h\
	$(INCRT)/sys/iobuf.h\
	$(INCRT)/sys/wn.h\
	$(FRC)

wn.dump.o:\
	$(INCRT)/sys/param.h\
	$(INCRT)/sys/sysmacros.h\
	$(INCRT)/sys/types.h\
	$(INCRT)/sys/wn.h\
	$(FRC)

wn.o:\
	$(INCRT)/sys/signal.h\
	$(INCRT)/sys/types.h\
	$(INCRT)/sys/sysmacros.h\
	$(INCRT)/sys/param.h\
	$(INCRT)/sys/systm.h\
	$(INCRT)/sys/buf.h\
	$(INCRT)/sys/elog.h\
	$(INCRT)/sys/erec.h\
	$(INCRT)/sys/iobuf.h\
	$(INCRT)/sys/conf.h\
	$(INCRT)/sys/dir.h\
	$(INCRT)/sys/user.h\
	$(INCRT)/sys/wn.h\
	$(INCRT)/sys/errno.h\
	$(INCRT)/sys/utsname.h\
	$(FRC)

wn.subs.o:\
	$(INCRT)/sys/signal.h\
	$(INCRT)/sys/types.h\
	$(INCRT)/sys/sysmacros.h\
	$(INCRT)/sys/param.h\
	$(INCRT)/sys/systm.h\
	$(INCRT)/sys/buf.h\
	$(INCRT)/sys/iobuf.h\
	$(INCRT)/sys/conf.h\
	$(INCRT)/sys/dir.h\
	$(INCRT)/sys/user.h\
	$(INCRT)/sys/wn.h\
	$(INCRT)/sys/errno.h\
	$(FRC)

mem.o:\
	$(INCRT)/sys/param.h\
	$(INCRT)/sys/types.h\
	$(INCRT)/sys/dir.h\
	$(INCRT)/sys/signal.h\
	$(INCRT)/sys/user.h\
	$(INCRT)/sys/errno.h\
	$(INCRT)/sys/buf.h\
	$(INCRT)/sys/systm.h\
	$(FRC)

partab.o:\
	$(FRC)

sxt.o:\
	$(INCRT)/sys/param.h\
	$(INCRT)/sys/types.h\
	$(INCRT)/sys/sysmacros.h\
	$(INCRT)/sys/seg.h\
	$(INCRT)/sys/systm.h\
	$(INCRT)/sys/file.h\
	$(INCRT)/sys/conf.h\
	$(INCRT)/sys/proc.h\
	$(INCRT)/sys/dir.h\
	$(INCRT)/sys/tty.h\
	$(INCRT)/sys/signal.h\
	$(INCRT)/sys/user.h\
	$(INCRT)/sys/errno.h\
	$(INCRT)/sys/termio.h\
	$(INCRT)/sys/ttold.h\
	$(INCRT)/sys/sxt.h\
	$(FRC)

sys.o:\
	$(INCRT)/sys/param.h\
	$(INCRT)/sys/types.h\
	$(INCRT)/sys/sysmacros.h\
	$(INCRT)/sys/dir.h\
	$(INCRT)/sys/signal.h\
	$(INCRT)/sys/user.h\
	$(INCRT)/sys/errno.h\
	$(INCRT)/sys/conf.h\
	$(INCRT)/sys/proc.h\
	$(FRC)

tt0.o:\
	$(INCRT)/sys/param.h\
	$(INCRT)/sys/types.h\
	$(INCRT)/sys/systm.h\
	$(INCRT)/sys/conf.h\
	$(INCRT)/sys/dir.h\
	$(INCRT)/sys/signal.h\
	$(INCRT)/sys/user.h\
	$(INCRT)/sys/errno.h\
	$(INCRT)/sys/proc.h\
	$(INCRT)/sys/file.h\
	$(INCRT)/sys/tty.h\
	$(INCRT)/sys/termio.h\
	$(INCRT)/sys/crtctl.h\
	$(INCRT)/sys/sysinfo.h\
	$(FRC)

tt1.o:\
	$(INCRT)/sys/param.h\
	$(INCRT)/sys/types.h\
	$(INCRT)/sys/systm.h\
	$(INCRT)/sys/conf.h\
	$(INCRT)/sys/dir.h\
	$(INCRT)/sys/signal.h\
	$(INCRT)/sys/user.h\
	$(INCRT)/sys/errno.h\
	$(INCRT)/sys/proc.h\
	$(INCRT)/sys/file.h\
	$(INCRT)/sys/tty.h\
	$(INCRT)/sys/termio.h\
	$(INCRT)/sys/sysinfo.h\
	$(FRC)

tty.o:\
	$(INCRT)/sys/param.h\
	$(INCRT)/sys/types.h\
	$(INCRT)/sys/systm.h\
	$(INCRT)/sys/dir.h\
	$(INCRT)/sys/signal.h\
	$(INCRT)/sys/user.h\
	$(INCRT)/sys/errno.h\
	$(INCRT)/sys/tty.h\
	$(INCRT)/sys/ttold.h\
	$(INCRT)/sys/proc.h\
	$(INCRT)/sys/file.h\
	$(INCRT)/sys/conf.h\
	$(INCRT)/sys/termio.h\
	$(INCRT)/sys/sysinfo.h\
	$(FRC)

sio.o:\
	$(INCRT)/sys/types.h\
	$(INCRT)/sys/param.h\
	$(INCRT)/sys/systm.h\
	$(INCRT)/sys/conf.h\
	$(INCRT)/sys/dir.h\
	$(INCRT)/sys/signal.h\
	$(INCRT)/sys/user.h\
	$(INCRT)/sys/errno.h\
	$(INCRT)/sys/proc.h\
	$(INCRT)/sys/file.h\
	$(INCRT)/sys/tty.h\
	$(INCRT)/sys/termio.h\
	$(INCRT)/sys/sysinfo.h\
	$(INCRT)/sys/sio.h\
	$(INCRT)/sys/clock.h\
	$(INCRT)/sys/ioctl.h\
	$(INCRT)/sys/mmu.h\
	$(INCRT)/sys/seg.h\
	$(FRC)

FRC:
