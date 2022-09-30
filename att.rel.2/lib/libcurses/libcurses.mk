#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#	@(#)	1.11
MODEL=
MSIZE=
LIB = $(ROOT)/usr/lib
LIBP = $(ROOT)/usr/lib/libp
BIN = $(ROOT)/usr/bin
TIC = screen/tic
install:	terminfo ismall ilarge 

ismall:	small
	cp screen/libcurses.a $(LIB)/small

ilarge:	large
	cp screen/libcurses.a $(LIB)/large

small:
	make -f makefile "MODEL=small" clean
	make -f makefile MODEL=small MSIZE=s libcurses.a
large:
	make -f makefile clean
	make -f makefile MODEL=large MSIZE=l libcurses.a
terminfo: clean
	if [ ! -d $(ROOT)/usr/bin ] ;\
		then \
		mkdir $(ROOT)/usr/bin; \
	fi
	(cd screen ; make install.tic.native )
	(cd screen ; make clobber)
	(cd screen ; make tic)
	(cd terminfo ; sh terminfo.sh )

clean:
	make -f makefile clean

clobber:	clean
	rm -f screen/libcurses.a screen/tic
