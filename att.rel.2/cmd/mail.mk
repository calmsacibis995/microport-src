#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#	@(#)	1.2
#	mail make file
ROOT = 
INSDIR = $(ROOT)/bin
LDFLAGS = -O -s
INS=:

mail:
	$(CC) $(LDFLAGS)  -o mail mail.c
	$(INS) $(INSDIR) mail
	if [ "$(INS)" != ":" ]; \
	then \
		rm -f $(INSDIR)/rmail; \
		ln $(INSDIR)/mail $(INSDIR)/rmail; \
	fi

all:	install clobber

install:
	$(MAKE) -f mail.mk INS="install -f"

clean:
	-rm -f *.o

clobber: clean
	rm -f mail
