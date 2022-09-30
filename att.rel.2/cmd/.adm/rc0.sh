#	iAPX286 @(#)rc0.sh	1.1
#	Run commands needed when shutting down the system

stty sane 2>/dev/null
echo 'The system is coming down. Please wait.'
for f in /etc/shutdown.d/*
{
	if [ -f ${f} ]
	then
		/bin/sh ${f}
	fi
}
