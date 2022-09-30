:	# @(#)mkcons.sh	1.2
REM=	NUMTTYS=16
if [ "$1" = "-r" ]; then
    REM="rm -f"; shift
fi
dev=${1:-"."} 

if [ "$REM" ]; then
    echo $REM $dev/cons[1-9]* $dev/cons[1-9]*ega &&
	 $REM $dev/cons[1-9]* $dev/cons[1-9]*ega
else
    i=1
    while [ $i -lt $NUMTTYS ]
    do
	egadev=`expr $i + 128`
	echo "mknod $dev/cons$i c 0 $i && mknod $dev/cons$i'ega' c 0 $egadev"
	mknod $dev/cons$i c 0 $i && mknod $dev/cons$i'ega' c 0 $egadev
	i=`expr $i + 1`
    done
fi
