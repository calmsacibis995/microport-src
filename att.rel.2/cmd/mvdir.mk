#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#	@(#)	1.1
#
# AUTHOR:	Zev Berger
# DATE	:	June 4, 1985
# NAME	:	mvdir.mk
# PURPOSE: This makefile serves to ensure that 'mvdir' installs
#	in the proper directory, and not in $ROOT/usr/bin (the default).

OL=$(ROOT)
INSDIR=$(OL)/etc
INS= :

all mvdir: mvdir.sh
	cp mvdir.sh mvdir
	$(INS) -f $(INSDIR) mvdir
	# REMINDER:	mvdir is a super-user only command.
	#		Permissions should be set accordingly.

install:
	rm -f $(INSDIR)/mvdir
	make -f mvdir.mk all INS=install OL=$(OL)
	chmod 744 $(INSDIR)/mvdir
	chown root $(INSDIR)/mvdir

clean clobber:
	rm -f mvdir
