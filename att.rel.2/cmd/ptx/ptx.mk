#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#	@(#)	1.1
#

ROOT = 

ULIB = $(ROOT)/usr/lib

UBIN = $(ROOT)/usr/bin

LDFLAGS = -s

all: ptx eign

install: all
	-cpset ptx $(UBIN)/ptx 
	-cpset eign $(ULIB)/eign 644

clean:
	-rm -f *.o

clobber: clean
	-rm  -f eign ptx
