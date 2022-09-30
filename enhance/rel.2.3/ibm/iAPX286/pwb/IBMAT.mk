#       @(#)pwb.mk.unopt	1.1 - 85/08/15

# uport!mike Mon May 11 13:30:45 PDT 1987
#	Merged in new make depend.
#	Modified to allow AT MERGE kernel in parallel directory.
#	See ../Makefile for details.

# ALLOPTIONS is needed for make depend. Should include all possible
#   switches that affect the include file dependency.
ALLOPTIONS =	 $(ATMERGE)
CFLAGS =	-Ml $O $(DEBUG) $(INCS) $(OPTIONS)
CPP =		/lib/cpp -DiAPX286 $(DEBUG) $(INCS) $(ALLOPTIONS)
DIR = 		pwb
FRC =	
INCS =		-I$(SRC) -I$(SRC)/sys -I$(ROOT)/usr/include 
LIBNAME =	../lib3
MAKEFILE =	Makefile
MAKENEW = 	.n.$(MAKEFILE)
MAKEOLD = 	.o.$(MAKEFILE)
O =		-O
OPT =		$(ROOT)/$(PFX)/lib/$(PFX)optim
OPTIONS =	$(ATMERGE)
SRC =		../../iAPX286
SRCDIR =	$(SRC)/$(DIR)

CFILES = prof.c

FILES =\
	$(LIBNAME)(prof.o)

.PRECIOUS:	$(LIBNAME)

all:		$(LIBNAME)

$(LIBNAME):	$(FILES)
#	-chgrp bin $(LIBNAME)
#	-chmod 664 $(LIBNAME)
#	-chown bin $(LIBNAME)

#.c.a:
#	$(CC) -c $(CFLAGS) $*.c
#	$(AR) rv $@ $*.o

clean:
	-rm -f *.o

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

# Dependencies generated on Mon May 11 14:35:52 PDT 1987
$(LIBNAME)(prof.o): \
	../../iAPX286/pwb/prof.c\
	../../iAPX286/sys/mmu.h\
	../../iAPX286/sys/param.h\
	../../iAPX286/sys/psl.h\
	../../iAPX286/sys/types.h\
	../../iAPX286/sys/user.h\
	../../iAPX286/sys/tss.h\
	../../iAPX286/sys/buf.h
	$(CC) -c $(CFLAGS) $(SRCDIR)/prof.c
	ar rv $(LIBNAME) prof.o

