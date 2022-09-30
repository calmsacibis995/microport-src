#	iAPX286 @(#)bcheckrc.sh	1.3
#	This must be the first entry in inittab
#	check the root file system

rootfs=`/etc/devnm / |
	sed '
		/^swap[ 	]/d
		/^\/dev\/swap[ 	]/d
		s;[ 	]\{1,\}/$;;
		/^[^/]/s;^;/dev/;
		q
	'`


msg=`/etc/fsstat ${rootfs} 2>&1`

if [ $? -ne 0 ]
then
	echo "
	${msg}
	The root file system (${rootfs}) is being checked automatically."
	/etc/fsck -y -D -b ${rootfs}
fi
