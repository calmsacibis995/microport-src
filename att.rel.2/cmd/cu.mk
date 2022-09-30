#       Copyright (c) 1985 AT&T
#         All Rights Reserved
#
#       THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#       The copyright notice above does not evidence any
#       actual or intended publication of such source code.
#
# /* @(#)cu.mk	1.2 - 85/09/03 */

CFLAGS=-s -O
CC = cc
ROOT =
INSDIR = $(ROOT)/usr/bin

all:	cu

install:	all
	cp cu $(INSDIR)
	chown root $(INSDIR)/cu
	chgrp sys $(INSDIR)/cu
	chmod 4755 $(INSDIR)/cu

cu:
	$(CC) $(CFLAGS) -o cu  cu.c

clean:
	-rm -f *.o

clobber: clean
	rm -f cu
