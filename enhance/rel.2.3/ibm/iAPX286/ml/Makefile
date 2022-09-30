# @(#)ml.mk	1.3 - 85/09/05
#
# uport!mike Mon May 11 13:30:45 PDT 1987
#	Modified to allow AT MERGE kernel in parallel directory.
#	See ../Makefile for details.

# ALLOPTIONS is needed for make depend. Should include all possible
#   switches that affect the include file dependency.
ALLOPTIONS =	-DSIASM -DUNIX_ASSEMBLER -DIBMAT -DLCCFIX -DPICFIX1 -DPICFIX3 -DPICFIX4 -DLCCSIO $(ATMERGE)
CFLAGS =	-Ml $O $(DEBUG) $(INCS) $(OPTIONS)
CPP =		/lib/cpp -DiAPX286 $(DEBUG) $(INCS) $(ALLOPTIONS)
DIR = 		ml
FRC =	
INCS =		-I$(SRC) -I$(SRC)/sys -I$(ROOT)/usr/include 
LIBNAME = 	../ml.o
MAKEFILE =	Makefile
MAKENEW = 	.n.$(MAKEFILE)
MAKEOLD = 	.o.$(MAKEFILE)
O =		-O
OPT =		$(ROOT)/$(PFX)/lib/$(PFX)optim
OPTIONS =	-DSIASM -DUNIX_ASSEMBLER -DIBMAT -DLCCFIX -DPICFIX1 -DPICFIX3 -DPICFIX4 $(ATMERGE)
#OPTIONS =	-DSIASM -DUNIX_ASSEMBLER -DIBMAT -DLCCFIX -DATMERGE -DATPROBE 
SRC =		../../iAPX286
SRCDIR =	$(SRC)/$(DIR)

CFILES = *.s

FILES =\
	copy.o\
	cswitch.o\
	end.o\
	idtvec.o\
	math.o\
	misc.o\
	power.o\
	realmode.o\
	start.o\
	trap.o\
	userio.o

.s.o:
	$(CC) $(CFLAGS) $<

.PRECIOUS:	$(LIBNAME)

all: $(LIBNAME)

$(LIBNAME):	$(FILES) $(FRC)
	-$(LD) -r *.o -o $(LIBNAME)

clean:
	-rm -f *.o

clobber:	clean
	-rm -f $(LIBNAME) ../ml.o

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

# Dependencies generated on Mon May 11 14:14:43 PDT 1987
copy.o: \
	../../iAPX286/ml/copy.s\
	../../iAPX286/sys/param.h\
	../../iAPX286/sys/mmu.h
	$(CC) -c $(CFLAGS) $(SRCDIR)/copy.s

cswitch.o: \
	../../iAPX286/ml/cswitch.s
	$(CC) -c $(CFLAGS) $(SRCDIR)/cswitch.s

end.o: \
	../../iAPX286/ml/end.s\
	../../iAPX286/sys/mmu.h
	$(CC) -c $(CFLAGS) $(SRCDIR)/end.s

idtvec.o: \
	../../iAPX286/ml/idtvec.s\
	../../iAPX286/sys/mmu.h
	$(CC) -c $(CFLAGS) $(SRCDIR)/idtvec.s

math.o: \
	../../iAPX286/ml/math.s\
	../../iAPX286/sys/psl.h
	$(CC) -c $(CFLAGS) $(SRCDIR)/math.s

misc.o: \
	../../iAPX286/ml/misc.s\
	../../iAPX286/sys/8259.h\
	../../iAPX286/sys/clock.h
	$(CC) -c $(CFLAGS) $(SRCDIR)/misc.s

power.o: \
	../../iAPX286/ml/power.s
	$(CC) -c $(CFLAGS) $(SRCDIR)/power.s

realmode.o: \
	../../iAPX286/ml/realmode.s\
	../../iAPX286/ml/../sys/realmode.h\
	../../iAPX286/ml/../sys/mmu.h\
	../../iAPX286/ml/../sys/8259.h\
	../../iAPX286/ml/../sys/clock.h
	$(CC) -c $(CFLAGS) $(SRCDIR)/realmode.s

start.o: \
	../../iAPX286/ml/start.s\
	../../iAPX286/sys/mmu.h\
	../../iAPX286/sys/psl.h\
	../../iAPX286/sys/param.h
	$(CC) -c $(CFLAGS) $(SRCDIR)/start.s

trap.o: \
	../../iAPX286/ml/trap.s\
	../../iAPX286/sys/param.h\
	../../iAPX286/sys/mmu.h\
	../../iAPX286/sys/trap.h\
	../../iAPX286/sys/8259.h
	$(CC) -c $(CFLAGS) $(SRCDIR)/trap.s

userio.o: \
	../../iAPX286/ml/userio.s\
	../../iAPX286/sys/mmu.h
	$(CC) -c $(CFLAGS) $(SRCDIR)/userio.s

