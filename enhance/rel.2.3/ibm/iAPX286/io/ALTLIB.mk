#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#       @(#)altio.mk	2.3 - 4/15/87
#	@(#)Makefile	1.7

# ALLOPTIONS is needed for make depend. Should include all possible
#   switches that affect the include file dependency.
ALLOPTIONS =	-DSIASM -DLCCFIX -DPICFIX1 -DPICFIX3 -DPICFIX4 -DATMERGE -DATPROBE -DLCCSIO
OPTIONS =	-DSIASM -DLCCFIX -DPICFIX1 -DPICFIX3 -DPICFIX4 $(ATMERGE)
#OPTIONS =	-DSIASM -DLCCFIX -DATMERGE -DATPROBE 
CFLAGS =	-Ml $O $(DEBUG) $(INCS) $(OPTIONS)
CON =		console
CPP =		/lib/cpp -DiAPX286 $(DEBUG) $(INCS) $(ALLOPTIONS)
CT =		ct
DIR = 		io
FRC =	
INCS =		-I$(SRC) -I$(SRC)/sys -I$(ROOT)/usr/include 
LIBNAME =	../lib7
MAKEFILE =	Makefile
MAKENEW = 	.n.$(MAKEFILE)
MAKEOLD = 	.o.$(MAKEFILE)
O =		-O
OPT =		$(ROOT)/$(PFX)/lib/$(PFX)optim
SRC =		../../iAPX286
SRCDIR =	$(SRC)/$(DIR)

CFILES = \
	dbgstubs.c\
	sxt.c\
	$(CT)conf.c\
	$(CT)int.c\
	qic02.c\
	qicsubs.s\
	spkr.c

FILES =\
	$(LIBNAME)(dbgstubs.o)\
	$(LIBNAME)(sxt.o)\
	$(LIBNAME)(ctconf.o)\
	$(LIBNAME)(ctint.o)\
	$(LIBNAME)(qic02.o)\
	$(LIBNAME)(qicsubs.o)\
	$(LIBNAME)(spkr.o)

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

all:	$(LIBNAME)

$(LIBNAME):	$(FILES)
#	-chgrp bin $(LIBNAME)
#	-chmod 664 $(LIBNAME)
#	-chown bin $(LIBNAME)

clean:
	-rm -f *.[o]

clobber:	clean
	-rm -f $(LIBNAME)

ctlib: ctconf.o ctint.o qic02.o qicsubs.o
	ar rv $(LIBNAME) ctconf.o ctint.o qic02.o qicsubs.o

noctlib:
	ar dv $(LIBNAME) ctconf.o ctint.o qic02.o qicsubs.o

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

# Dependencies generated on Mon Jun  1 10:22:50 PDT 1987
$(LIBNAME)(dbgstubs.o): \
	../../iAPX286/io/dbgstubs.c
	$(CC) -c $(CFLAGS) $(SRCDIR)/dbgstubs.c
	ar rv $(LIBNAME) dbgstubs.o

$(LIBNAME)(sxt.o):\
	$(SRC)/io/sxt.c\
	$(SRC)/sys/param.h\
	$(SRC)/sys/types.h\
	$(SRC)/sys/seg.h\
	$(SRC)/sys/systm.h\
	$(SRC)/sys/conf.h\
	$(SRC)/sys/proc.h\
	$(SRC)/sys/tty.h\
	$(SRC)/sys/user.h\
	$(SRC)/sys/sxt.h\
	$(FRC)

$(LIBNAME)(ctconf.o): \
	../../iAPX286/io/ctconf.c\
	../../iAPX286/sys/systm.h\
	../../iAPX286/sys/buf.h\
	../../iAPX286/sys/iobuf.h\
	../../iAPX286/sys/conf.h\
	../../iAPX286/sys/proc.h\
	../../iAPX286/sys/user.h\
	../../iAPX286/sys/tss.h\
	../../iAPX286/sys/trap.h\
	../../iAPX286/sys/mmu.h\
	../../iAPX286/sys/seg.h\
	../../iAPX286/sys/8237.h\
	../../iAPX286/sys/8259.h\
	../../iAPX286/sys/map.h\
	../../iAPX286/sys/qic2tvi.h\
	../../iAPX286/sys/ct.h\
	../../iAPX286/sys/ctdebug.h\
	../../iAPX286/sys/ctioctl.h
	$(CC) -c $(CFLAGS) $(SRCDIR)/ctconf.c
	ar rv $(LIBNAME) ctconf.o

$(LIBNAME)(ctint.o): \
	../../iAPX286/io/ctint.c\
	../../iAPX286/sys/types.h\
	../../iAPX286/sys/param.h\
	../../iAPX286/sys/systm.h\
	../../iAPX286/sys/iobuf.h\
	../../iAPX286/sys/buf.h\
	../../iAPX286/sys/conf.h\
	../../iAPX286/sys/user.h\
	../../iAPX286/sys/tss.h\
	../../iAPX286/sys/mmu.h\
	../../iAPX286/sys/8237.h\
	../../iAPX286/sys/8259.h\
	../../iAPX286/sys/qic2tvi.h\
	../../iAPX286/sys/ct.h\
	../../iAPX286/sys/ctdebug.h\
	../../iAPX286/sys/ctioctl.h
	$(CC) -c $(CFLAGS) $(SRCDIR)/ctint.c
	ar rv $(LIBNAME) ctint.o

$(LIBNAME)(qic02.o): \
	../../iAPX286/io/qic02.c\
	../../iAPX286/sys/types.h\
	../../iAPX286/sys/param.h\
	../../iAPX286/sys/systm.h\
	../../iAPX286/sys/buf.h\
	../../iAPX286/sys/iobuf.h\
	../../iAPX286/sys/conf.h\
	../../iAPX286/sys/user.h\
	../../iAPX286/sys/tss.h\
	../../iAPX286/sys/mmu.h\
	../../iAPX286/sys/8237.h\
	../../iAPX286/sys/8259.h\
	../../iAPX286/sys/qic2tvi.h\
	../../iAPX286/sys/ct.h\
	../../iAPX286/sys/ctdebug.h\
	../../iAPX286/sys/ctioctl.h
	$(CC) -c $(CFLAGS) $(SRCDIR)/qic02.c
	ar rv $(LIBNAME) qic02.o

$(LIBNAME)(qicsubs.o): \
	../../iAPX286/io/qicsubs.s\
	../../iAPX286/sys/qic2tvi.h
	$(CC) -c $(CFLAGS) $(SRCDIR)/qicsubs.s
	ar rv $(LIBNAME) qicsubs.o

$(LIBNAME)(spkr.o):\
	$(SRC)/io/spkr.c\
	$(SRC)/sys/types.h\
	$(SRC)/sys/param.h\
	$(SRC)/sys/user.h\
	$(SRC)/io/spkr.h\
	$(FRC)

