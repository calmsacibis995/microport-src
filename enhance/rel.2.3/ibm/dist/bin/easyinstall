#
# easyinstall
#

if /etc/easyprep 
then
  NEC=`patch -k /system5 fdtimeout`
  TVI=`patch -k /system5 TVI`
  echo "[H[J"
  echo "\nThe necessary file systems have now been created.\n"
  echo "\n\n\nFiles will now be copied from your boot floppy "
  echo "to your hard disk. "
  echo "This should take no more than a few minutes.\n\n\n"
  labelit /dev/rdsk/0s0 root 0s0 > /dev/null 
  labelit /dev/rdsk/0s2 usr 0s2 > /dev/null
  mount /dev/dsk/0s0 /mnt 1> /dev/null 2>&1
  if  [ "$?" -ne 0 ]
  then
    echo "\n\n\nmount failed."
    echo "\n\n\neasyinstall has been aborted at this point.\n\n\n"
    exit
  fi
  mkdir /mnt/dev /mnt/dev/dsk /mnt/dev/rdsk /mnt/bin /mnt/usr 
  mkdir /mnt/tmp /mnt/etc /mnt/mnt /mnt/etc/rc.d /mnt/lib
  cpio -pvdlmua /mnt < /list > /dev/null
  rm /mnt/dev/swap; mknod /mnt/dev/swap b 0 1 > /dev/null
  hdrt.patch /mnt/system5 > /dev/null
  if [ $NEC = 0x0 ]
  then
    patch /mnt/system5 fdtimeout 0 > /dev/null
  fi
  if [ $TVI = 0x1 ]
  then
    patch /mnt/system5 TVI 1 > /dev/null
  fi
  mv /mnt/bin/installit /mnt/bin/installit.00
  mv /mnt/profile /mnt/.profile
  for i in installit checklist fstab mnttab; do
    mv /mnt/etc/$i.hd /mnt/etc/$i
  done
  cd
  umount /dev/dsk/0s0
# dd if=/etc/boot.hd of=/dev/dsk/0s0 > /dev/null
  dd if=/etc/Boot.hd of=/dev/dsk/0s255 > /dev/null
  sync; sync
  echo "\n\n\nThe copying of files has now been completed.\n"
  echo "Please wait for the message,"
  echo " 'The system has stopped, and can now be rebooted'."
  echo "After this message appears, REMOVE the boot"
  echo "floppy from drive A, and reboot your computer."
  echo "You will then be able to proceed with the"
  echo "installation of any other System V/AT "
  echo "floppy disks."
  init 0
fi
