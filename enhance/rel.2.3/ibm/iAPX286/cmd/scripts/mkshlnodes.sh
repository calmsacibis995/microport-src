:
# mkshlnodes: make the device nodes necessary to run shell layers
if [ ! -d /dev/sxt ]
then
	mkdir /dev/sxt
	chown bin /dev/sxt
	chgrp bin /dev/sxt
	chmod 777 /dev/sxt
fi
/etc/mksxt -v /dev/sxt 12 0 64 8
