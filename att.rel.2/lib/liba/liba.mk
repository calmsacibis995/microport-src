#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#	@(#)	1.1
#
# must make clean before making libc since the .o files from
# the previous make are probably for the wrong sized model
install:
	make -f makefile clean
	make -f makefile MODEL=small MSIZE=s $@
	make -f makefile clean
	make -f makefile MODEL=large MSIZE=l $@

clean:
	make -f makefile clean

clobber:	clean
	make -f makefile clobber
