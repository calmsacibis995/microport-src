#	iAPX286 @(#)makefile	1.3 85/09/16
# makefile for 286/libI77
#
#
CFLAGS=-O
COMMON=ROOT= DEST=/cmd/export CC=cc PFX=

all: smallm clean largem hugem

smallm:
	$(MAKE)   -e -f libI77.mk $(COMMON) MODEL=small \
		CFLAGS="$(CFLAGS)  -Ms" \
		CC=cc AR=ar STRIP=strip
		mv libI77.a libI77.a.sm

largem:
	$(MAKE)   -e -f libI77.mk  $(COMMON) MODEL=large \
		CFLAGS="$(CFLAGS)  -Ml" \
		CC=cc AR=ar STRIP=strip
		mv libI77.a libI77.a.lg

hugem:
#	$(MAKE)   -e -f libI77.mk  $(COMMON) MODEL=huge \
		CFLAGS="$(CFLAGS)  -Mh" \
		CC=cc AR=ar STRIP=strip \
		install
	echo "No huge model C compiler yet."

clean:
	make -e -f libI77.mk clean

clobber:
	make -e -f libI77.mk clobber
