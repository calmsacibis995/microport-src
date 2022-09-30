#  iAPX286 @(#)brc.sh	1.3
#  Initialize the mount table, /etc/mnttab
/etc/devnm / | grep -v swap | /etc/setmnt
