#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#	@(#)	1.1
#	sa2.sh 1.1 of 7/15/83
DATE=`date +%d`
RPT=/usr/adm/sa/sar$DATE
DFILE=/usr/adm/sa/sa$DATE
ENDIR=/usr/bin
cd $ENDIR
$ENDIR/sar $* -f $DFILE > $RPT
find /usr/adm/sa \( -name 'sar*' -o -name 'sa*' \) -mtime +7 -exec rm {} \;
