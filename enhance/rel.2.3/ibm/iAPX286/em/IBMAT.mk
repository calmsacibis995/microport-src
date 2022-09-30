#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)Makefile	1.2"

# uport!mike Mon May 11 13:30:45 PDT 1987
#	Modified to allow AT MERGE kernel in parallel directory.
#	See ../Makefile for details.

# ALLOPTIONS is needed for make depend. Should include all possible
#   switches that affect the include file dependency.
ALLOPTIONS =	 $(ATMERGE)
CFLAGS =	-Ml $O $(DEBUG) $(INCS) $(OPTIONS)
CPP =		/lib/cpp -DiAPX286 $(DEBUG) $(INCS) $(ALLOPTIONS)
DIR = 		em
FRC =	
INCS =		-I$(SRC) -I$(SRC)/sys -I$(ROOT)/usr/include 
LIBNAME = 	../lib9
MAKEFILE =	Makefile
MAKENEW = 	.n.$(MAKEFILE)
MAKEOLD = 	.o.$(MAKEFILE)
O =		-O
OFILE =		emull.o
OPT =		$(ROOT)/$(PFX)/lib/$(PFX)optim
OPTIONS =	$(ATMERGE)
PACK =		$(ROOT)/etc/conf/pack.d/emul
SRC =		../../iAPX286
SRCDIR =	$(SRC)/$(DIR)

CFILES =\
	arith.s\
	dcode.s\
	divmul.s\
	emul_entry.c\
	emul_init.c\
	lipsq.s\
	reg.s\
	remsc.s\
	round.s\
	spec_init.c\
	status.s\
	store.s\
	subadd.s\
	trans.s

FILES =\
	arith.o\
	dcode.o\
	divmul.o\
	emul_entry.o\
	emul_init.o\
	lipsq.o\
	reg.o\
	remsc.o\
	round.o\
	spec_init.o\
	status.o\
	store.o\
	subadd.o\
	trans.o

all:		$(LIBNAME)

$(LIBNAME):	$(OFILE)
		ar rv ../lib9 $(OFILE)

$(OFILE):	$(FILES)
		-chmod 660 $(FILES)
		$(LD) -r $(FILES) -o $(OFILE)

install:	all
		-cp $(OFILE) $(PACK)/Driver.o 
clean:
		-rm -f *.o

clobber:	clean
		-rm -f $(PACK)/Driver.o
FRC:

depend:
	echo '# DO NOT DELETE THIS LINE - make depend uses it\n' > .deplist
	echo "# Dependencies generated on `date`" >> .deplist
	echo 'FILES =\c' > .filelist
	for i in $(CFILES); \
	do \
	    echo "$$i" >> /dev/tty; \
	    file=`expr $$i : '\(.*\)\..*'`; \
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
	ln $(MAKEFILE) IBMAT.mk
	rm -f .deplist .filelist

# anything after the next line will disappear!
# DO NOT DELETE THIS LINE - make depend uses it

# Dependencies generated on Mon May 11 14:29:42 PDT 1987
arith.o: \
	../../iAPX286/em/arith.s\
	../../iAPX286/em/e80287.h
	$(CC) -c $(CFLAGS) $(SRCDIR)/arith.s

dcode.o: \
	../../iAPX286/em/dcode.s\
	../../iAPX286/em/e80287.h\
	../../iAPX286/sys/reg.h
	$(CC) -c $(CFLAGS) $(SRCDIR)/dcode.s

divmul.o: \
	../../iAPX286/em/divmul.s\
	../../iAPX286/em/e80287.h
	$(CC) -c $(CFLAGS) $(SRCDIR)/divmul.s

emul_entry.o: \
	../../iAPX286/em/emul_entry.c\
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
	$(CC) -c $(CFLAGS) $(SRCDIR)/emul_entry.c

emul_init.o: \
	../../iAPX286/em/emul_init.c\
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
	$(CC) -c $(CFLAGS) $(SRCDIR)/emul_init.c

lipsq.o: \
	../../iAPX286/em/lipsq.s\
	../../iAPX286/em/e80287.h
	$(CC) -c $(CFLAGS) $(SRCDIR)/lipsq.s

reg.o: \
	../../iAPX286/em/reg.s\
	../../iAPX286/em/e80287.h
	$(CC) -c $(CFLAGS) $(SRCDIR)/reg.s

remsc.o: \
	../../iAPX286/em/remsc.s\
	../../iAPX286/em/e80287.h
	$(CC) -c $(CFLAGS) $(SRCDIR)/remsc.s

round.o: \
	../../iAPX286/em/round.s\
	../../iAPX286/em/e80287.h
	$(CC) -c $(CFLAGS) $(SRCDIR)/round.s

spec_init.o: \
	../../iAPX286/em/spec_init.c\
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
	$(CC) -c $(CFLAGS) $(SRCDIR)/spec_init.c

status.o: \
	../../iAPX286/em/status.s\
	../../iAPX286/em/e80287.h
	$(CC) -c $(CFLAGS) $(SRCDIR)/status.s

store.o: \
	../../iAPX286/em/store.s\
	../../iAPX286/em/e80287.h
	$(CC) -c $(CFLAGS) $(SRCDIR)/store.s

subadd.o: \
	../../iAPX286/em/subadd.s\
	../../iAPX286/em/e80287.h
	$(CC) -c $(CFLAGS) $(SRCDIR)/subadd.s

trans.o: \
	../../iAPX286/em/trans.s\
	../../iAPX286/em/e80287.h
	$(CC) -c $(CFLAGS) $(SRCDIR)/trans.s

