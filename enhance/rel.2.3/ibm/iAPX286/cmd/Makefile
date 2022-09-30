# makefile for the IBM AT specific commands.  10/21/86  uport!dwight
# commands currently implemented:
#	make clean: removes all binaries, and relinkable object code.
#		To be used when starting from scratch.
#	make all: makes all neccessary AT programs. Doesn't install them.

all:
	cd config; make -f IBMAT.mk all
	cd ddump; make -f IBMAT.mk all
	cd divvy; make -f IBMAT.mk all
	cd easyprep; make -f IBMAT.mk all
	cd fdisk; make -f IBMAT.mk BOOTDRVTBL="-DBOOTDRVTBL" all
	cd format; make -f IBMAT.mk BOOTDRVTBL="-DBOOTDRVTBL" all
	cd keytest; make -f IBMAT.mk all
	cd login; make -f IBMAT.mk all
	cd lpset; make -f IBMAT.mk all
	cd patch; make -f IBMAT.mk all
	cd reboot; make -f IBMAT.mk all
	cd rootname; make -f IBMAT.mk all
	cd setcolor; make -f IBMAT.mk all
	cd setkey; make -f IBMAT.mk all
	cd setup; make -f IBMAT.mk all
	cd shm; make -f IBMAT.mk all
	cd strm; make -f IBMAT.mk all
	cd stty; make -f IBMAT.mk all
#	cd tape; make -f IBMAT.mk all
	cd unixsyms; make -f IBMAT.mk all

clean:
	cd config; make -f IBMAT.mk clean
	cd ddump; make -f IBMAT.mk clean
	cd divvy; make -f IBMAT.mk clean
	cd easyprep; make -f IBMAT.mk clean
	cd fdisk; make -f IBMAT.mk clean
	cd format; make -f IBMAT.mk clean
	cd keytest; make -f IBMAT.mk clean
	cd login; make -f IBMAT.mk clean
	cd lpset; make -f IBMAT.mk clean
	cd patch; make -f IBMAT.mk clean
	cd reboot; make -f IBMAT.mk clean
	cd rootname; make -f IBMAT.mk clean
	cd setcolor; make -f IBMAT.mk clean
	cd setkey; make -f IBMAT.mk clean
	cd setup; make -f IBMAT.mk clean
	cd shm; make -f IBMAT.mk clean
	cd strm; make -f IBMAT.mk clean
	cd stty; make -f IBMAT.mk clean
#	cd tape; make -f IBMAT.mk clean
	cd unixsyms; make -f IBMAT.mk clean

clobber:
	cd config; make -f IBMAT.mk clobber
	cd ddump; make -f IBMAT.mk clobber
	cd divvy; make -f IBMAT.mk clobber
	cd easyprep; make -f IBMAT.mk clobber
	cd fdisk; make -f IBMAT.mk clobber
	cd format; make -f IBMAT.mk clobber
	cd keytest; make -f IBMAT.mk clobber
	cd login; make -f IBMAT.mk clobber
	cd lpset; make -f IBMAT.mk clobber
	cd patch; make -f IBMAT.mk clobber
	cd reboot; make -f IBMAT.mk clobber
	cd rootname; make -f IBMAT.mk clobber
	cd setcolor; make -f IBMAT.mk clobber
	cd setkey; make -f IBMAT.mk clobber
	cd setup; make -f IBMAT.mk clobber
	cd shm; make -f IBMAT.mk clobber
	cd strm; make -f IBMAT.mk clobber
	cd stty; make -f IBMAT.mk clobber
	cd tape; make -f IBMAT.mk clobber
	cd unixsyms; make -f IBMAT.mk clobber

install: 
	cd config; make -f IBMAT.mk install
	cd ddump; make -f IBMAT.mk install
	cd divvy; make -f IBMAT.mk install
	cd easyprep; make -f IBMAT.mk install
	cd fdisk; make -f IBMAT.mk install
	cd format; make -f IBMAT.mk install
	cd keytest; make -f IBMAT.mk install
	cd login; make -f IBMAT.mk install
	cd lpset; make -f IBMAT.mk install
	cd patch; make -f IBMAT.mk install
	cd reboot; make -f IBMAT.mk install
	cd rootname; make -f IBMAT.mk install
	cd setcolor; make -f IBMAT.mk install
	cd setkey; make -f IBMAT.mk install
	cd setup; make -f IBMAT.mk install
	cd shm; make -f IBMAT.mk install
	cd strm; make -f IBMAT.mk install
	cd stty; make -f IBMAT.mk install
#	cd tape; make -f IBMAT.mk install
	cd unixsyms; make -f IBMAT.mk install
