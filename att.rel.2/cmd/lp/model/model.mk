#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#
#	@(#)	1.6
#	model make file
ROOT = $(S4ROOT)
OL	= $(ROOT)/
SL	= $(ROOT)/usr/src/cmd
RDIR	= $(SL)/lp/model
INS	= :
REL	= current
OWN	= lp 
GRP	= bin
LIST	= lp
BIN	= $(OL)usr/spool/lp/model
SOURCE = 1640 dumb f450 hp prx pprx
MAKE	= make

compile all:    no_models # $(SOURCE)


no_models $(SOURCE):
	$(INS) if [ ! -d $(BIN) ] ; then mkdir $(BIN); fi
	$(INS) for i in "$(SOURCE)"; \
	  $(INS) do cp $$i $(BIN); \
	  $(INS) chmod 775 $(BIN)/$$i; \
	  $(INS) if [ "$(OL)" = "/" ] ; \
	     $(INS) then (cd $(BIN); $(INS) chgrp $(GRP) $$i; $(INS) chown $(OWN) $$i;) \
	  $(INS) fi; \
	$(INS) done
	$(INS) rm -f $(BIN)/dumbS
	$(INS) ln $(BIN)/dumb   $(BIN)/dumbS
	$(INS) rm -f $(BIN)/other
	$(INS) ln $(BIN)/dumb   $(BIN)/other
	$(INS) rm -f $(BIN)/otherS
	$(INS) ln $(BIN)/dumb   $(BIN)/otherS

install:
	$(MAKE) -f model.mk INS= OL=$(OL) $(ARGS)
	
clean:	; :
clobber:  ; :
delete:	; :
	rm -f $(SOURCE)


