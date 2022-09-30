#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#	@(#)	1.2
install:
	make -f makefile clean
	make -f makefile MSIZE=s MODEL=small install
	make -f makefile clean
	make -f makefile MSIZE=l MODEL=large install

clean:
	make -f makefile clean

clobber:	clean
	make -f makefile clobber
