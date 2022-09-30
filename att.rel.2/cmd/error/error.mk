#	Copyright (c) 1985 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#/*   @(#)error.mk	1.3 - 85/08/09 */
install:
	cd iAPX286; make -f error.mk install

clean:
	cd iAPX286; make -f error.mk clean

clobber:
	cd iAPX286; make -f error.mk clobber
