#  iAPX286 @(#)acct.rc.sh	1.2
rm -f /usr/adm/acct/nite/lock*
/bin/su - adm -c /usr/lib/acct/startup
echo process accounting started
/usr/lib/errdemon
echo errdemon started
/bin/su - sys -c "/usr/lib/sa/sadc /usr/adm/sa/sa`date +%d`"
