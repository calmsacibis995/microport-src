#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#	@(#)	1.4
#	ctrace makefile
# installation directories:
#	This makefile has been modified for compiling
#	in a cross environment and, subsequently, downloading
#	to the target processor. The installation directories,
#	therefore, are relative to ROOT, while the compiled-in
#	pathnames are absolute so that it executes properly
#	when installed on the target machine.	- zb
ROOT=
BIN = $(ROOT)/usr/bin
XEQLIB = /usr/lib/ctrace
LIB = $(ROOT)/usr/lib/ctrace

# setting preprocessor symbols:
# set for UNIX/370:
# U370 = -b1,0
CC=cc
CFLAGS = -O -DLIB=\"$(XEQLIB) $(U370)

YFLAGS = -d

SOURCE	 = constants.h global.h main.c parser.y scanner.l lookup.c trace.c \
	   runtime.c ctcr
OTHER	 = ctrace.mk
OBJECTS	 = main.o parser.o scanner.o lookup.o trace.o

ctrace: $(OBJECTS)
	$(CC) -s $(OBJECTS) -o $@

all: ctrace

install: all
	-rm -f $(BIN)/ctc $(BIN)/ctcr $(BIN)/ctrace
	install -f $(BIN) ctrace
	install -f $(BIN) ctcr
	ln $(BIN)/ctcr $(BIN)/ctc
	-mkdir $(LIB)
	install -f $(LIB) runtime.c

clean:
	rm -f *.o y.tab.h y.output

clobber: clean
	rm -f ctrace
