#  iAPX286 @(#)bsetdate.sh	1.1
#  Check and set date

. /etc/TIMEZONE

while :
do
	echo "Is the date `date` correct? (y or n) \c"
	read reply
	if
		[ "$reply" = y ]
	then
		break
	else
		echo "Enter the correct date: (mmddhhmmyy) \c"
		read reply
		date "$reply"
	fi
done
