#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#	@(#)	1.1
#	sa1.sh 1.1 of 7/15/83
DATE=`date +%d`
ENDIR=/usr/lib/sa
DFILE=/usr/adm/sa/sa$DATE
cd $ENDIR
if [ $# = 0 ]
then
	$ENDIR/sadc 1 1 $DFILE
else
	$ENDIR/sadc $* $DFILE
fi
