#       @(#)rc2.sh	1.4 - 85/09/18
#	"Run Commands" executed when the system is changing to init state 2,
#	traditionally called "multi-user".

. /etc/TIMEZONE

stty sane 2>/dev/null
#
#  Empty the wtmp file
> /etc/wtmp

#  empty sulog IFF it exists
if [ -f /usr/adm/sulog ]
then
    mv /usr/adm/sulog /usr/adm/Osulog
    > /usr/adm/sulog
fi
#
#	Pickup start-up packages for mounts, daemons, services, etc.
for f in /etc/rc.d/*
{
	if [ -f ${f} ]
	then
		/bin/sh ${f}
	fi
}
