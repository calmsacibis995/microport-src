#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#	@(#)	1.3
CC = cc

CFLAGS = -O -Ml
FFLAG =
LDFLAGS = -s
LIBS = -lld -lc -la

all:	prfld prfdc prfpr prfsnap prfstat

install:
	make -f profiler.mk $(ARGS) FFLAG="$(FFLAG)" CFLAGS="$(CFLAGS)"


prfld:		prfld.c
	$(CC) $(CFLAGS) $(FFLAG) $(LDFLAGS) -o prfld prfld.c $(LIBS)
	cp prfld $(ROOT)/etc/prfld
#	-chmod 775 $(ROOT)/etc/prfld
#	-chown bin $(ROOT)/etc/prfld
#	-chgrp bin $(ROOT)/etc/prfld

prfdc:		prfdc.c
	$(CC) $(CFLAGS) $(FFLAG) $(LDFLAGS) -o prfdc prfdc.c $(LIBS)
	cp prfdc $(ROOT)/etc/prfdc
#	-chmod 775 $(ROOT)/etc/prfdc
#	-chown bin $(ROOT)/etc/prfdc
#	-chgrp bin $(ROOT)/etc/prfdc

prfpr:		prfpr.c
	$(CC) $(CFLAGS) $(FFLAG) $(LDFLAGS) -o prfpr prfpr.c $(LIBS)
	cp prfpr $(ROOT)/etc/prfpr
#	-chmod 775 $(ROOT)/etc/prfpr
#	-chown bin $(ROOT)/etc/prfpr
#	-chgrp bin $(ROOT)/etc/prfpr

prfsnap:	prfsnap.c
	$(CC) $(CFLAGS) $(FFLAG) $(LDFLAGS) -o prfsnap prfsnap.c $(LIBS)
	cp prfsnap $(ROOT)/etc/prfsnap
#	-chmod 775 $(ROOT)/etc/prfsnap
#	-chown bin $(ROOT)/etc/prfsnap
#	-chgrp bin $(ROOT)/etc/prfsnap

prfstat:	prfstat.c
	$(CC) $(CFLAGS) $(FFLAG) $(LDFLAGS) -o prfstat prfstat.c $(LIBS)
	cp prfstat $(ROOT)/etc/prfstat
#	-chmod 775 $(ROOT)/etc/prfstat
#	-chown bin $(ROOT)/etc/prfstat
#	-chgrp bin $(ROOT)/etc/prfstat

clean:
	-rm -f prfdc prfld prfpr prfsnap prfstat

clobber:	clean
