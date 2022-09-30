: /bin/sh
curdir=`pwd`
srcdir=/
if [ "-$1" != - ]; then
	if [ -d "$1" ]; then
		srcdir="$1"
	else
		echo "$1 is not a directory."
		exit 1
	fi
fi

echo "\n\t\tCopyright (c) 1985 AT&T"
echo "\t\tAll Rights Reserved\n"
echo "\tMkinstall creates a set of diskettes that are installable by the"
echo "\tMicroport standard utility \"/etc/installit\"."
if [ -f files1 ]
then
	read filename < files1
	cd $srcdir
	if [ -f $filename ]
	then
		cd $curdir
		echo "\n"
	else
		echo "\n\tThe file \"$filename\" from files1 was not found.\n"
		echo "\tThe installit program installs \c"
		echo "all files relative to the"
		echo "\troot directory.  If the file names are \c"
		echo "relative to a directory"
		echo "\tother than the root directory, \c"
		echo "the \"installit\" program will"
		echo "\tnot put files where they belong.\n"
		exit 1
	fi
else
	echo "\n\tThis program needs a file containing a list of file"
	echo "\tnames for each diskette of the set.   They are called"
	echo "\t\"files[123...]\" and are expected to be in the"
	echo "\tcurrent directory.   They were not found.\n"
	exit 1
fi
echo "\tPlease enter the following information.\n"
echo "\tThe program set name ( i.e. application name ): \c"
read diskset
echo "\tThe number of diskettes needed for the set ( <ENTER> = 1 ): \c"
read disklast
if [ ! "$disklast" ]
then
	disklast=1
fi
echo "\tThe number of the diskette to start with ( <ENTER> = 1 ): \c"
read diskname
if [ ! "$diskname" ]
then
	diskname=1
fi
if [ -f $diskset\links ]
then
	cp $diskset\links /tmp
fi
echo "\nMaking  $disklast  \"installit\" diskettes for  $diskset.\n"
for i in 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20
do
	if [ $i -ne $diskname ]
	then
		continue
	fi
	echo "\nPlease insert a high density diskette for $i of $disklast,"
	echo "then press <ENTER>.\c"
	read R
	echo "$diskset $i $disklast" >diskdata
	/bin/ls diskdata | cpio -ou >/dev/rdsk/0s24 
	if [ $? -ne 0 ]
	then
		echo "\naborting due to the above error\n"
		exit 1
	fi
	if [ -f files$i ]
	then
		echo "\nThe following files have been included:\n"
		cd $srcdir
		cat $curdir\/files$i | cpio -ovu >/dev/rdsk/0s25
:				       cpio -ovduma
		cd $curdir
		if [ $? -ne 0 ]
		then
			echo "\naborting due to the above error\n"
			exit 1
		fi
	else
		echo "files$i was not found, leaving mkinstall.\n"
		exit 1
	fi
	if [ $i -eq $disklast ]
	then
		echo "\nDone with floppy set $diskset #$i"
		if [ ! "$1" ]
		then
			rm -f diskdata
		fi
		rm -f /tmp/$diskset\links
		break
	else
		echo "\c"
	fi
	diskname=`expr $i + 1`
done
