#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#	@(#)	1.2
# Install /etc/termcap
#
install:
	if [ ! -d $(ROOT)/etc ] ;\
	then \
		mkdir $(ROOT)/etc ;\
	fi
	cp termcap $(ROOT)/etc

all:

clean:

clobber: