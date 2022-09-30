# iAPX286 @(#)filesave.sh	1.2
#	The following prototype is meant as a guide in setting
#	up local filesave procedures.  It copies the BOOT, ROOT
#	and USR file systems from winchester drive 0 to drive 1.

if [ $# != 1 ]
then
	echo usage: filesave packname
	exit 2
fi
date >> /etc/log/filesave.log
dd if=/dev/rdsk/0s1 of=/dev/rdsk/1s1 bs=1024 count=9
volcopy root /dev/rdsk/0s10 S3B000 /dev/rdsk/1s10 $1
volcopy usr /dev/rdsk/0s11 S3B000 /dev/rdsk/1s11 $1
echo FILESAVE COMPLETED >> /etc/log/filesave.log

# <@(#)filesave.sh	1.2>
