# make boot floppies and linkkits
LINKFILES=Files.linkkit
LINKTMP=files.tmp
RELDATE=09031500

all:
	@echo "usage: make [ ibmboot | tviboot | linkkit | linkdisk ]"

ibmboot: ibmpatch floppyA

tviboot: tvipatch floppyA

ibmpatch:
	/etc/patch dist/system5 TVI 0
	@touch $(RELDATE) dist/system5

tvipatch:
	/etc/patch dist/system5 TVI 1
	@touch $(RELDATE) dist/system5

floppyA:
	mkfs /dev/dsk/0s25 2120:200 2 15
	mount /dev/dsk/0s25 /mnt
	cd dist ; find . -depth -print > list ; cat list | cpio -pvdlmua /mnt
	umount /dev/dsk/0s25
	dd if=./dist/etc/flboot.hd of=/dev/dsk/0s24 count=1
	dd if=./dist/etc/Boot.flhd of=/dev/dsk/0s24 seek=15 count=8
#	sleep 4
	fsck /dev/dsk/0s25
	sync; sync
	@echo "Finished with creating the distribution floppy A "

lloginboot:
	@echo "making Limited Login kernel in the distribution directory"
	@/etc/patch dist/system5 cprlmv 1
	@/etc/patch dist/system5 -b +33 utsname 0x4c
	@touch $(RELDATE) dist/system5
	/etc/patch dist/system5 -s +27 utsname
	@echo "remote users: `/etc/patch dist/system5 cprlmv`"

uloginboot:
	@echo "making Unlimited Login kernel in the distribution directory"
	@/etc/patch dist/system5 cprlmv 32
	@/etc/patch dist/system5 -b +33 utsname 0x55
	@touch $(RELDATE) dist/system5
	/etc/patch dist/system5 -s +27 utsname
	@echo "remote users: `/etc/patch dist/system5 cprlmv`"

backup: 
	find . -depth -print | 0>&1 cpio -ocv > /dev/rdsk/fd096

linkkit:	linkclean iAPX286/cf usr/linkkit/doc/linkkit.list
	cd iAPX286 ;cpio -pvdlmua ../usr/linkkit < ../usr/linkkit/doc/linkkit.list
	find usr/linkkit -depth -print > $(LINKFILES)

lloginlink:
	@echo "making Limited Login link kit"
	cd iAPX286/cf;$(MAKE) -f ll.mk llogin

uloginlink:
	@echo "making Unlimited Login link kit"
	cd iAPX286/cf;$(MAKE) -f ll.mk ulogin

ibmlink:
	cd iAPX286/cf;$(MAKE) -f ll.mk ibmlink

tvilink:
	cd iAPX286/cf;$(MAKE) -f ll.mk tvilink

linkdisk: 
	cp usr/tmp/linkkitlinks /tmp
	cp $(LINKFILES) $(LINKTMP)
	echo "/tmp/linkkitlinks" >> $(LINKTMP)
	echo "linkkit 1 1" > diskdata
	echo diskdata | cpio -ov > /dev/dsk/0s24
	cpio -ov < $(LINKTMP) > /dev/dsk/0s25
	rm -f diskdata $(LINKTMP) /tmp/linkkitlinks

.IGNORE:

linkclean:
	cd usr/linkkit;rm * cf/* io/* os/* sys/* ml/* em/* cmd/config/* cmd/unixsyms/*
	rm -f $(LINKFILES) $(LINKTMP) diskdata
