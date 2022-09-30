# ll.mk
# makefile for Link Kits
#	Limited Login patching for link kits
#	Library specific changes for TVI vs IBM compatibles
# version strings are in the form of "2.2.2-L" or "2.2.2-U"
LNKDATE=09031500
LIBDATE=09092141

llogin:
	sed "/^VER/s/-./-L/" <Linkkit.mk >tmp.mk
	cp tmp.mk Linkkit.mk; rm tmp.mk
	touch $(LNKDATE) Linkkit.mk
	@/etc/patch name.o -b +33 utsname 0x4c
	@/etc/patch lomem.o cprlmv 1
	@touch $(LNKDATE) name.o lomem.o
	/etc/patch name.o -s +27 utsname

ulogin:
	sed "/^VER/s/-./-U/" <Linkkit.mk >tmp.mk
	cp tmp.mk Linkkit.mk; rm tmp.mk
	touch $(LNKDATE) Linkkit.mk
	@/etc/patch name.o -b +33 utsname 0x55
	@/etc/patch lomem.o cprlmv 32
	@touch $(LNKDATE) name.o lomem.o
	/etc/patch name.o -s +27 utsname

ibmlink:
	cd ../io;mv hd1.o.ibm hd1.o;ar rv ../lib2 hd1.o;mv hd1.o hd1.o.ibm
	@touch $(LIBDATE) ../lib2
	cd ..;rm -f ../usr/linkkit/lib2;ln lib2 ../usr/linkkit/lib2
	cd ..;rm -f ../usr/linkkit/lib7;ln lib7.ibm ../usr/linkkit/lib7

tvilink:
	cd ../io;mv hd1.o.tvi hd1.o;ar rv ../lib2 hd1.o;mv hd1.o hd1.o.tvi
	@touch $(LIBDATE) ../lib2
	cd ..;rm -f ../usr/linkkit/lib2;ln lib2 ../usr/linkkit/lib2
	cd ..;rm -f ../usr/linkkit/lib7;ln lib7.tvi ../usr/linkkit/lib7

