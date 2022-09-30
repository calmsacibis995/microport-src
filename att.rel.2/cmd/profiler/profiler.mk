#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#	@(#)	1.1
FFLAG =

CC = cc

CFLAGS = -O -Ml

install:
	-if vax; then cd vax; \
		make -f profiler.mk ARGS="$(ARGS)" CFLAGS="$(CFLAGS)" \
		FFLAG="$(FFLAG)" install clobber; \
		fi
	-if pdp11; then cd pdp11; \
		make -f profiler.mk ARGS="$(ARGS)" CFLAGS="$(CFLAGS)" \
		FFLAG="$(FFLAG)" install clobber; fi
	-if u3b; then cd u3b20; \
		make -f profiler.mk ARGS="$(ARGS)" CFLAGS="$(CFLAGS)" \
		FFLAG="$(FFLAG)" install clobber; fi
	-if u3b5 || u3b2; then cd u3b5; \
		make -f profiler.mk ARGS="$(ARGS)" CFLAGS="$(CFLAGS)" \
		FFLAG="$(FFLAG)" install clobber; fi
	-if iAPX286; then cd iAPX286; \
		make -f profiler.mk ARGS="$(ARGS)" CFLAGS="$(CFLAGS)" \
		FFLAG="$(FFLAG)" install clobber; fi

clobber:
