#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#	@(#)	1.9
# Cross-environment makefile for lint
#
ROOT=
FLG =
UBIN = $(ROOT)/usr/bin
ULIB = $(ROOT)/usr/lib
MIP = $(ROOT)/usr/src/cmd/sgs/comp/mip
LINT = lint
LINT1 = ./lintpass1
INS = install
STRIP = strip
TMPDIR = /usr/tmp
FFLAG = 
#LFLAGS = -i $(LDFLAGS)
LFLAGS = $(LDFLAGS)
#  -DBUG4 is necessary to turn off pcc debugging tools (these tools cannot
#   be used with the lint shell script since their output conflicts)
CFLAGS = -O -Ml -DLINT -I$(MIP) -I. -DBUG4 -DFLEXNAMES
FRC =
MFILES = macdefs $(MIP)/manifest $(MIP)/mfile1

#object files for the first and second passes
OFILES1 = trees.o pftn.o scan.o comm1.o messages.o optim.o xdefs.o cgram.o\
	lerror.o msgbuf.o lint.o
OFILES2 = lpass2.o lerror2.o messages2 msgbuf2.o

all:	mk_local chk_pdp  lint1 lint2 llib-lc.ln llib-port.ln llib-lm.ln mk_nat

mk_nat:
	cp lint.nmk $(ROOT)/local/lint/lint.mk

mk_local:
	if [ ! -d $(ROOT)/local/lint ]; then mkdir $(ROOT)/local/lint; fi

chk_pdp:
	if pdp11 && [ "$(FLG)" != "x" ]; \
	then \
		$(MAKE) -$(MAKEFLAGS) CFLAGS="$(CFLAGS) -UFLEXNAME" \
			CC="$(CC)" LDFLAGS="$(LDFLAGS)" FLG=x -f lint.mk all; \
	elif (u3b5 || u3b2) && [ "$(FLG)" != "x" ]; \
	then \
		$(MAKE) -$(MAKEFLAGS) CFLAGS="$(CFLAGS)" CC="$(CC)" \
		LDFLAGS="$(LDFLAGS)" FFLAG="-f" FLG=x -f lint.mk all; \
	fi

#makes for the first pass of lint
lint1:	$(OFILES1) $(FRC)
	$(CC) -Ml $(FFLAG) $(LFLAGS) -o lint1 $(OFILES1)

# Make a lint1 that will run on the host processor when compiling
# lint in the cross-compilation environment.  This is needed so that
# the lint libraries may be built in the cross-compilation environment.

#$(LINT1):	lint1
#	if [ "$(ROOT)" != "" ]; \
#	then \
#		echo "Making lint1 to run on host"; \
#		rm $(OFILES1) cgram.c; \
#		/bin/make ROOT="$(ROOT)" FFLAG="$(FFLAG)" CC=cc AS=/bin/as LD=/bin/ld -f lint.mk $(OFILES1); \
#		cc -o $(LINT1) $(OFILES1); \
#		rm $(OFILES1); \
#	else \
#		echo "Copying lint1 to $(LINT1)"; \
#		cp lint1 $(LINT1); \
#	fi
trees.o:	$(MIP)/messages.h $(MFILES) $(MIP)/trees.c
			$(CC) $(CFLAGS) -c  $(MIP)/trees.c
pftn.o:		$(MIP)/messages.h $(MFILES) $(MIP)/pftn.c
			$(CC) $(CFLAGS) -c  $(MIP)/pftn.c
scan.o:		$(MIP)/messages.h $(MFILES) $(MIP)/scan.c
			$(CC) $(CFLAGS) -c  $(MIP)/scan.c
comm1.o:	$(MIP)/common $(MFILES)     $(MIP)/comm1.c
			$(CC) $(CFLAGS) -c  $(MIP)/comm1.c
messages.o:	$(MIP)/messages.h           $(MIP)/messages.c
			$(CC) $(CFLAGS) -c  $(MIP)/messages.c
optim.o:	$(MFILES)                   $(MIP)/optim.c
			$(CC) $(CFLAGS) -c  $(MIP)/optim.c
xdefs.o:	$(MFILES)                   $(MIP)/xdefs.c
			$(CC) $(CFLAGS) -c  $(MIP)/xdefs.c
cgram.o:	$(MIP)/messages.h $(MFILES) 
cgram.c:	$(MIP)/cgram.y
		yacc $(MIP)/cgram.y ; mv y.tab.c cgram.c
lerror.o:	lerror.h $(MIP)/messages.h 
msgbuf.o:	lerror.h $(MIP)/messages.h
lint.o:		lerror.h lmanifest $(MIP)/messages.h $(MFILES)

#makes for the lint libraries
llib-lc.ln:  llib-lc
	cp llib-lc $(ULIB)

llib-port.ln:  llib-port
	cp llib-port $(ULIB)

llib-lm.ln:  llib-lm
	cp llib-lm $(ULIB)

#makes for the second pass of lint
lint2:	$(OFILES2) $(FRC)
	$(CC) -Ml $(LFLAGS) -o lint2 $(OFILES2)
messages2:	$(MIP)/messages.h           $(MIP)/messages.c
		$(CC) $(CFLAGS) -c $(MIP)/messages.c
		mv messages.o messages2
msgbuf2.o:	lerror.h 
lpass2.o:	lerror.h lmanifest lpass2.h $(MIP)/manifest
lerror2.o:	lerror.h lmanifest lpass2.h $(MIP)/manifest $(MIP)/messages.h

#miscellaneous utilities

install: all 
	$(INS) -n $(ULIB) lint1
	$(INS) -n $(ULIB) lint2
	$(STRIP) $(ULIB)/lint1
	$(STRIP) $(ULIB)/lint2
	cp lint.sh lint
	chmod 0755 lint
	$(INS) -n $(UBIN) lint
	(cd $(ROOT) ; find local/lint usr/lib/lint1 usr/lib/llib-lc \
		usr/lib/llib-lm usr/lib/llib-port -print \
		| cpio -oc > $(ROOT)/local/lint.cpio )

lintall:
	$(LINT) -DLINT  -I. -I$(MIP)  $(MIP)/xdefs.c $(MIP)/scan.c \
	$(MIP)/pftn.c $(MIP)/trees.c $(MIP)/optim.c lint.c $(MIP)/messages.c \
	$(MIP)/comm1.c msgbuf.c lerror.c -b cgram.c 
	$(LINT) -DLINT  -I$(MIP) -I. lpass2.c lerror2.c msgbuf2.c \
	$(MIP)/messages.c
clean:
	rm -f *.o messages2 cgram.c
clobber: clean
	rm -f $(LINT1) lint1 lint2 llib-lc.ln llib-port.ln llib-lm.ln lint
FRC:

