#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#	@(#)	1.3
install all:
#
#	must make clean before making library since object from
#	previous make may be for the wrong sized model
#
	make -f makefile clean
	make -f makefile MODEL=small MSIZE=s $@
	make -f makefile clean
	make -f makefile MODEL=large MSIZE=l $@

clobber:
	make -f makefile clobber
