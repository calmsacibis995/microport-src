#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#	@(#)	1.3	
#	filter make file
ROOT =
OL	= $(ROOT)/
SL	= $(ROOT)/usr/src/cmd
RDIR	= $(SL)/lp/filter
INS	= :
REL	= current
OWN	= lp 
GRP	= bin
LIST	= lp
BIN	= $(OL)usr/lib
IFLAG	= -n
SOURCE  =  hp2631a.c prx.c pprx.c
FILES   =  hp2631a.o prx.o pprx.o
MAKE	= make
INSBIN	= $(INS) cp $@ $(BIN); \
	  $(INS) chmod 775 $(BIN)/$@; \
	  $(INS) if [ "$(OL)" = "/" ]; \
	     $(INS) then cd $(BIN); $(INS) chgrp $(GRP) $@; $(INS) chown $(OWN) $@; \
	  $(INS) fi
LDFLAGS = -s

compile all:  hp2631a prx pprx
	:

hp2631a:	hp2631a.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o hp2631a hp2631a.o
	$(INSBIN)

prx:	prx.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o prx prx.o
	$(INSBIN)

pprx:	pprx.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o pprx pprx.o
	$(INSBIN)

install:
	$(MAKE) -f filter.mk INS= OL=$(OL) $(ARGS)

clean:
	rm -f $(FILES)

clobber:  clean
	rm -f hp2631a prx pprx

delete:	clobber
	rm -f $(SOURCE)
