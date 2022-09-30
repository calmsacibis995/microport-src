#	iAPX286 @(#)libI77.mk	1.3 85/09/16
#	LIBI77 MAKEFILE

CC = $(PFX)cc
AR = $(PFX)ar
STRIP = $(PFX)strip
CFLAGS = -O
INCRT=$(ROOT)/usr/include
INS = install
INSDIR = $(ROOT)/usr/lib

OBJ =	Version.o backspace.o dfe.o due.o iio.o inquire.o rewind.o rsfe.o \
	rdfmt.o sue.o uio.o wsfe.o sfe.o fmt.o nio.o lio.o lread.o open.o \
	close.o util.o endfile.o wrtfmt.o err.o fmtlib.o ecvt.o ltostr.o

all:
	$(MAKE)  libI77.a clean -e -f libI77.mk $(COMMON) MODEL=small \
		CFLAGS="$(CFLAGS)  -Ms -I$(INCRT)" \
		CC=$(PFX)cc AR=$(PFX)ar STRIP=$(PFX)strip
		mv libI77.a libI77.a.sm
	$(MAKE)  libI77.a clean   -e -f libI77.mk  $(COMMON) MODEL=large \
		CFLAGS="$(CFLAGS)  -Ml -I$(INCRT)" \
		CC=$(PFX)cc AR=$(PFX)ar STRIP=$(PFX)strip
		mv libI77.a libI77.a.lg


libI77.a:	$(OBJ)
		$(AR) r libI77.a $?
	if pdp11; then \
		$(STRIP) libI77.a; \
	else \
		$(STRIP) -r libI77.a; \
		$(AR) ts libI77.a; \
	fi;

lio.o:	lio.h

lread.o: lio.h

.c.o:
	$(CC) -c $(CFLAGS) $<

install:	all
	cp libI77.a.sm libI77.a
	$(INS) -f $(INSDIR)/small libI77.a
	cp libI77.a.lg libI77.a
	$(INS) -f $(INSDIR)/large libI77.a

clean:
	-rm -f $(OBJ)

clobber:	clean
	-rm -f libI77.a
