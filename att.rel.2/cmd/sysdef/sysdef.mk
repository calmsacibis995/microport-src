#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#	@(#)	1.1
#
# sysdef commands makefile
#
TARGET =
FRC =

all:
	-if vax; then cd dec; make -f sysdef.mk $(TARGET); fi
	-if pdp11; then cd dec; make -f sysdef.mk $(TARGET); fi
	-if u3b; then cd u3b20; make -f sysdef.mk $(TARGET); fi
	-if iAPX286; then cd iAPX286; make -f sysdef.mk $(TARGET); fi

install: 
	make -f ./sysdef.mk TARGET='install'
clean:
	make -f ./sysdef.mk TARGET='clean'
clobber:
	make -f ./sysdef.mk TARGET='clobber'

FRC:
