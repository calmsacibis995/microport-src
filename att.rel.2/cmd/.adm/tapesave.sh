# iAPX286 @(#)tapesave.sh	1.2
#	This is a prototype file for floppy saves of file systems
#		other than root.
#	It may need to be modified for local use.
#	It will copy the named file system onto floppy drive 0.

echo 'Enter file system name (e.g., usr):'
read f
echo 'Enter device name (e.g., /dev/rdsk/0s11):'
read d
echo 'Enter pack volume label (e.g., p0045):'
read v
t=/dev/rdsk/0s24
echo 'Enter floppy volume label (e.g., f0005):'
read l

/etc/labelit  $t
if test  $? -gt 0
then
  /etc/labelit $t $f  $l -n
  if test $? -gt 0
  then
    exit 1
  fi
fi
/etc/volcopy $f $d $v $t $l

# <@(#)tapesave.sh	1.2>
