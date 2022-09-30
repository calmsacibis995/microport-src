:
# installit v. 2.3.
# Copyright @ 1986,1987 Microport Systems Inc. All rights reserved.
# In order to properly install system5, one must run:
#	/etc/fdisk:	To set up the partitions correctly
#	/etc/divvy:	To make the appropriate filesystems
#	cpio -pdlm:	To set up a minimal harddisk system

NOW=

while :
do
    echo "Are you upgrading an existing Microport installation? (y or n): \c"
    read reply
    case "$reply" in
	n|N )	LIST="list"
		break ;;
	y|Y )	LIST="list.ug"
		UPGRADE=1
		break ;;
	* )
		echo "Invalid input, type 'y' or 'n'"
		continue ;;
    esac
done

if [ x$UPGRADE = "x" ]
then
    # use fdisk to establish the partitions:
    while :
    do
	echo "Are the hard disk partitions set up correctly $NOW? (y or n): \c"
	read reply
	case "$reply" in
	    y|Y )	break ;;
	    n|N )	echo "Executing /etc/fdisk, to establish the \c"
			echo "hard disk partitions (one moment please)"
#			/etc/fdisk -s
			/etc/fdisk
		  if  [ "$?" -ne 0 ]
		  then
			  echo "\n\n\nfdisk failed."
			  echo "\n\n\ninstallit has been aborted at this point.\n\n\n"
			  exit
		  fi
			NOW="NOW"
			continue ;;
	    * )
			echo "Invalid input, type 'y' or 'n'"
			continue ;;
	esac
    done

    # use divvy to run the appropriate mkfs on the desired filesystems:
    NOW=

    while :
    do
	echo "Have you set up the file systems correctly $NOW? (y or n): \c"
	read reply
	case "$reply" in
	    y|Y )	break ;;
	    n|N )	echo "Executing /etc/divvy, to establish the \c"
			echo "filesystems (one moment please)"
			/etc/divvy
		  if  [ "$?" -ne 0 ]
		  then
			  echo "\n\n\ndivvy failed."
			  echo "\n\n\ninstallit has been aborted at this point\n\n\n."
			  exit
		  fi
			NOW="NOW"
			continue ;;
	    * )
			echo "Invalid input, type 'y' or 'n'"
			continue ;;
	esac
    done

    echo "\nThe hard disk is now configured for system5"
    echo "Copying over the necessary files:"
    set -x
    labelit /dev/rdsk/0s0 root 0s0
    labelit /dev/rdsk/0s2 usr 0s2
fi
NEC=`patch -k /system5 fdtimeout`
TVI=`patch -k /system5 TVI`
mount /dev/dsk/0s0 /mnt
if  [ "$?" -ne 0 ]
then
	echo "\n\n\nmount failed."
	echo "\n\n\ninstallit has been aborted at this point.\n\n\n"
	exit
fi
if [ x$UPGRADE != "x1" ]
then
    mkdir /mnt/dev /mnt/dev/dsk /mnt/dev/rdsk /mnt/bin /mnt/usr 
    mkdir /mnt/tmp /mnt/etc /mnt/mnt /mnt/etc/rc.d /mnt/lib
fi
cpio -pvdlmua /mnt < /$LIST
if [ x$UPGRADE != "x1" ]
then
    rm /mnt/dev/swap; mknod /mnt/dev/swap b 0 1
    mv /mnt/bin/installit /mnt/bin/installit.00
    mv /mnt/profile /mnt/.profile
    for i in installit checklist fstab mnttab; do
	mv /mnt/etc/$i.hd /mnt/etc/$i
    done
else
    mv /mnt/etc/release/motd /mnt/etc
    mv /mnt/etc/release/releasenotes /mnt
fi
hdrt.patch /mnt/system5
if [ $NEC = 0x0 ]
then
    patch /mnt/system5 fdtimeout 0
fi
if [ $TVI = 0x1 ]
then
    patch /mnt/system5 TVI 1
fi
cd
umount /dev/dsk/0s0
if [ x$UPGRADE = "x1" ]
then
	dd if=/etc/boot.hd of=/dev/dsk/0s0
fi
dd if=/etc/Boot.hd of=/dev/dsk/0s255
sync;sync
set +x
echo "The hard disk is now initialized."
echo "Please wait for the message 'The system has stopped'."
echo "After the system has come to a stop, remove the floppy in drive A, "
echo "and reboot your computer (via control-alt-del)."
if [ x$UPGRADE != "x1" ]
then
echo "When you see the # prompt, type 'installit' to finish the installation."
else
echo "When you are logged in again, you may install the utility disk."
fi
init 0
