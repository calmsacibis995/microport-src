# iAPX286 @(#)umountall.sh	1.1
mount|sed -n -e '/^\/ /d' -e 's/^.* on\(.*\) read.*/umount \1/p' | sh -
