# makefile for microport's fdisk program.  1/22/86 uport!dwight
#
#	make, make all, make fdisk:	This is what you want to type.
#
SRC	= easyin.sh installit.sh ttypatch.sh nec.sh tvi.sh
OBJ	=
CC	= /bin/cc
LD	= /bin/ld
CFLAGS	= -s
INCRT	= ../..
INCRT1	= /usr/include
ROOT	= ../../../dist


all: easyin installit ttypatch mkinstall nec tvi

clean:
	-rm -f easyin
	-rm -f installit
	-rm -f ttypatch
	-rm -f mkinstall
	-rm -f nec
	-rm -f tvi

clobber: clean
	-rm -f $(ROOT)/bin/easyinstall
	-rm -f $(ROOT)/bin/installit
	-rm -f $(ROOT)/etc/ttypatch
	-rm -f $(ROOT)/etc/nec
	-rm -f $(ROOT)/etc/tvi

install: all
	cp easyin $(ROOT)/bin/easyinstall
	chmod 500 $(ROOT)/bin/easyinstall 
	chown bin $(ROOT)/bin/easyinstall 
	chgrp sys $(ROOT)/bin/easyinstall 
	cp installit $(ROOT)/bin
	chmod 500 $(ROOT)/bin/installit 
	chown bin $(ROOT)/bin/installit
	chgrp sys $(ROOT)/bin/installit
	cp ttypatch $(ROOT)/etc
	chmod 500 $(ROOT)/etc/ttypatch 
	chown bin $(ROOT)/etc/ttypatch
	chgrp sys $(ROOT)/etc/ttypatch
	cp nec $(ROOT)/bin
	chmod 500 $(ROOT)/bin/nec 
	chown bin $(ROOT)/bin/nec 
	chgrp sys $(ROOT)/bin/nec 
	cp tvi $(ROOT)/bin
	chmod 500 $(ROOT)/bin/tvi 
	chown bin $(ROOT)/bin/tvi 
	chgrp sys $(ROOT)/bin/tvi 
