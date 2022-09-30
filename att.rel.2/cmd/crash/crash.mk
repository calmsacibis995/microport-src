#	Copyright (c) 1985 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#/*   @(#)crash.mk	1.3 - 85/08/09 */
#
# crash commands makefile
#
TARGET =
FRC =

all:
	-if vax; then cd dec; make -f crash.mk $(TARGET); fi
	-if pdp11; then cd dec; make -f crash.mk $(TARGET); fi
	-if u3b; then cd u3b20; make -f crash.mk $(TARGET); fi
	-if iAPX286; then cd iAPX286; make -f crash.mk $(TARGET); fi

install: 
	make -f ./crash.mk TARGET='install'
clean:
	make -f ./crash.mk TARGET='clean'
clobber: clean
	make -f ./crash.mk TARGET='clobber'

FRC:
