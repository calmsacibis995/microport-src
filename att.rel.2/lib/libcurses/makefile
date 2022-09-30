#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#	@(#)	1.2
#
#	Curses Library High Level Makefile.
#	To install libcurses.a, the tic compiler, & the terminfo database type:
#		"make install"
#	To compile libcurses & the tic database compiler normally, type:
#		"make all tic"
#	To compile a particular file with normal compilation type:
#		"make FILES='<particular .o files>"
#	If debugging is desired then type:
#		"make O=debug FILES='<particular .o files>"
#	If tracing is desired then type:
#		"make O=trace FILES='<particular .o files>"
#	If profiling is desired then type:
#		"make O=profile FILES='<particular .o files>"
#	To compile only the tic compiler type:
#		"make tic"
#	To compile & install just the terminfo database type:
#		"make terminfo"
#	To compile the tic compiler & terminfo database (install also) type:
#		"make tic terminfo"
#	To compile the show program type:
#		"make show"
#	To compile the test program ti4 that uses MINICURSES type:
#		"make ti4"
#
SHELL=/bin/sh
ROOT=
all:	libcurses.a tic terminfo

libcurses.a:
	cd screen ; make MODEL=$(MODEL) MSIZE=$(MSIZE) libcurses.a
	@echo "Libcurses.a has been made."

tic:
	cd screen ; make tic
	@echo "The tic compiler has been made."
	
install:
	# make and install native libcurses.a and native tic
	cd screen ; make install CC2=$(CC)
	# now make cross tic for compiling terminfo database
	rm screen/tic  
	cd screen ; make tic
	# now make terminfo database
	@echo It will take 20 minutes or more to generate /usr/lib/terminfo/*/*.
	(cd terminfo ; make all 2>&1 > errs ; echo "terminfo database done" )

terminfo:
	@echo It will take 20 minutes or more to generate /usr/lib/terminfo/*/*.
	(cd terminfo ; make all 2>&1 > errs ; echo "terminfo database done" )

show:
	cd screen ; make show
	@echo "The show program has been made."
	
ti4:
	cd screen ; make ti4
	@echo "The ti4 program has been made."
	
clean:
	cd screen ; make clean
	cd terminfo ; make clean

clobber:
	cd screen ; make clobber
	cd terminfo ; make clean
