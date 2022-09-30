#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#	@(#)	1.2
#
#	killall make file

ROOT =
INCLUDE = $(ROOT)/usr/include
INSDIR = $(ROOT)/etc
LIST = lp
CFLAGS = -s -O -Ml
INS=:

all:	killall

killall:	$(INCLUDE)/stdio.h\
	$(INCLUDE)/a.out.h\
	$(INCLUDE)/fcntl.h\
	$(INCLUDE)/sys/types.h\
	$(INCLUDE)/sys/param.h\
	$(INCLUDE)/sys/proc.h\
	$(INCLUDE)/sys/var.h\
	killall.c
	$(CC) -I$(INCLUDE) $(CFLAGS) -o killall killall.c
	$(INS) $(INSDIR)  killall

install:
	$(MAKE) -f killall.mk INS="install -f"

listing:
	pr killall.mk killall.c | $(LIST)

clean:
	-rm -f *.o

clobber: clean
	rm -f killall
