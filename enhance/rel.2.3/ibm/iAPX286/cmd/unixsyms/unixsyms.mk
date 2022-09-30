#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

# THIS VERSION FOR CROSS-BUILDING ONLY!

#ident "@(#)unixsyms.mk	1.1"

INCRT = $(ROOT)/usr/include
INSDIR = $(ROOT)/i286/bin
CC=
CFLAGS = -Ml
LDFLAGS = -s
LIBS = -lld
AR =
FRC =
FILES =  unixsyms.c unixsyms2.c unixsyms3.c

all:    unixsyms

unixsyms: \
	$(FILES)\
	$(INCRT)/stdio.h\
	$(INCRT)/fcntl.h\
	$(INCRT)/filehdr.h\
	$(INCRT)/syms.h\
	$(INCRT)/storclass.h\
	$(INCRT)/scnhdr.h\
	$(INCRT)/ldfcn.h\
	unixsyms.h
	$(CC) $(CFLAGS) $(LDFLAGS) -o unixsyms $(FILES) $(LIBS)

install:        all
	rm -f $(INSDIR)/unixsyms
	mv unixsyms $(INSDIR)/unixsyms
	chmod 755 $(INSDIR)/unixsyms
	chgrp bin $(INSDIR)/unixsyms
	chown bin $(INSDIR)/unixsyms

clean:
	-rm -f *.o

clobber:        clean
	-rm unixsyms
