#       Copyright (c) 1985 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

ROOT =
INCRT = $(ROOT)/usr/include
INCLUDES = $(INCRT)/sys/dir.h        \
	$(INCRT)/sys/errno.h      \
	$(INCRT)/sys/fp.h         \
	$(INCRT)/sys/mmu.h        \
	$(INCRT)/sys/param.h      \
	$(INCRT)/sys/proc.h       \
	$(INCRT)/sys/psl.h        \
	$(INCRT)/sys/reg.h        \
	$(INCRT)/sys/seg.h        \
	$(INCRT)/sys/signal.h     \
	$(INCRT)/sys/sysinfo.h    \
	$(INCRT)/sys/sysmacros.h  \
	$(INCRT)/sys/systm.h      \
	$(INCRT)/sys/trap.h       \
	$(INCRT)/sys/types.h      \
	$(INCRT)/sys/user.h

SOBJECTS = arith.o dcode.o divmul.o lipsq.o reg.o remsc.o round.o \
           status.o store.o subadd.o trans.o
COBJECTS = emul_init.o emul_entry.o spec_init.o
OBJECTS = $(COBJECTS) $(SOBJECTS)
SSOURCE = $(SOBJECTS:.o=.s)
CSOURCE = $(COBJECTS:.o=.c)
LIBNAME = ../lib9
CC=cc
AR=ar
CFLAGS = -I$(INCRT) -Ml
FRC =

$(LIBNAME): $(LIBNAME)(emull.o)
	-chgrp bin $(LIBNAME)
	-chmod 664 $(LIBNAME)
	-chown bin $(LIBNAME)

$(LIBNAME)(emull.o): emull.o
	$(AR) rv $(LIBNAME) emull.o

emull.o: $(OBJECTS)
	rm -f emull.o
	if [ '*.o' != *.o ] ; \
	then $(LD) -r -o emull.o $(OBJECTS) ; \
	else $(AR) xv $(LIBNAME) emull.o ; \
	fi

.s.o:   e80287.h $(FRC)
	if [ -f $< ] ; \
	then $(CC) -c $(CFLAGS) $< ; \
	else : ; \
	fi

.c.o: $(INCLUDES) $(FRC)
	if [ -f $< ] ; \
	then $(CC) -c $(CFLAGS) $< ; \
	else : ; \
	fi

clean:
	-rm -f *.o

clobber: clean
	-rm -f $(LIBNAME)

$(SSOURCE):
	:

$(CSOURCE):
	:

*.c:
	:

*.s:
	:

FRC:
