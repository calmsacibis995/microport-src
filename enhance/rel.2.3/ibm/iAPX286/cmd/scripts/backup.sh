:
date=`date '+%m%d'`
files='.files.'$date
disk=/dev/rdsk/fd
temp=/tmp/.files.$$
wait=yes

flag=.archive.list

case $1 in
    -f)	flag=$2;	shift; shift	;;
    -w) wait=;		shift		;;
esac

if [ -f $files ]; then
    date=`date '+%H'`
    files=$files$date
    if [ -f $files ]; then
	date=`date +%M`
	files=$files$date
    fi
fi

echo ========= Backup list in $files ''
find . -type f -newer $flag -print |
    sort |
    sed -e 's/^\.\///' |
    tee $temp |
    sed -f .backup.sed > $files

echo ========= Backup ''
cpio -ocv < $files > $disk

echo ========= Verify ''
if [ "$wait" ]; then
    echo "Insert floppy #1: \c"
    read c
fi
cpio -ict < $disk | diff $files -

if [ $? -eq 0 ]; then
    echo ========= Backup done.''
else
    echo ========= Error in backup.''
fi

echo ========= Files not copied: ''
diff $temp $files | grep '^<' | sort | pg -20

