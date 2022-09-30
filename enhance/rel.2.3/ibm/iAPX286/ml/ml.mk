# @(#)ml.mk	1.3 - 85/09/05
INCRT=/usr/include/sys
INCRT1= ..
INCRT2= ../sys
CFLAGS= -c -Ml -I$(INCRT1) -I$(INCRT2) -I$(INCRT) -DSIASM
ARFLAGS= rv
CC=cc
AS=as
AR=ar
LD=ld
LIBNAME = ../lib0
FRC =
SFILES=	copy.s cswitch.s end.s idtvec.s misc.s start.s trap.s userio.s \
	math.s power.s
OFILES=	$(LIBNAME)(copy.o)\
	$(LIBNAME)(cswitch.o)\
	$(LIBNAME)(end.o)\
	$(LIBNAME)(idtvec.o)\
	$(LIBNAME)(misc.o)\
	$(LIBNAME)(start.o)\
	$(LIBNAME)(trap.o)\
	$(LIBNAME)(userio.o)\
	$(LIBNAME)(math.o)\
	$(LIBNAME)(power.o)

.s.a: $(OFILES)
	$(CC) $(CFLAGS) $*.s
	$(AR) $(ARFLAGS) $(LIBNAME) $*.o

.PRECIOUS:	$(LIBNAME)

$(LIBNAME):	../ml.o $(FRC)
	-chgrp bin $(LIBNAME)
	-chmod 664 $(LIBNAME)
	-chown bin $(LIBNAME)

../ml.o:	$(OFILES) $(FRC)
	-$(LD) -r *.o -o ../ml.o

clean:
	-rm -f *.o

clobber:	clean
	-rm -f $(LIBNAME) ../ml.o

FRC:

copy.o:		$(INCRT)/param.h \
		$(INCRT)/mmu.h

end.o:		$(INCRT)/mmu.h

idtvec.o:	$(INCRT)/mmu.h

trap.o:		$(INCRT)/mmu.h \
		$(INCRT)/trap.h

userio.o:	$(INCRT)/mmu.h

start.o:	$(INCRT)/mmu.h \
		$(INCRT)/psl.h \
		$(INCRT)/param.h
