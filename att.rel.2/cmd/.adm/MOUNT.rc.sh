#  iAPX286 @(#)MOUNT.rc.sh	1.2 - 85/09/18
#  Mount file systems
cd /
/etc/mountall /etc/fstab

#  If vi installed, preserve files
if [ -x /usr/lib/preserve ]
then
	/usr/lib/preserve -
fi

# Clean up /tmp and /usr/tmp
# Use this method in case /tmp or /usr/tmp are separate filesystems.
for t in /tmp /usr/tmp
do
   if [ ! -d $t ]
   then 
      rm -f $t
      mkdir $t
   elif [ -x /bin/ls -a -x /bin/rm -a -x /bin/sed ]; then
      cd $t
      rm -rf `ls -a | sed '/^\.$/d;/^\.\.$/d'`
   else echo "Couldn't clean $t"
   fi
   chmod 777 $t
   chown sys $t
   chgrp sys $t
done
