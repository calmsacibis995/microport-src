#	Copyright (c) 1985 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#/*   @(#)error.mk	1.3 - 85/08/09 */
########
#
#	Standard Macros
#
########
CC = cc
CCLD1_CMD = $(CCLD_CMD) $(PPDEFS) $(CFLAGS) $(INC_LIST)
CCLD_CMD = $(CC) $(LDFLAGS)
CFLAGS = -Ml -O
LDFLAGS = -s
PPDEFS = 

INC_LIST	=

INS = /etc/install

all:
install:	all

PRODUCTS = errpt errdead errstop errdemon

UINC = $(ROOT)/usr/include

errstop:
	cp errstop.sh errstop

install:
	$(INS) -n $(ROOT)/usr/bin errpt
	$(INS) -n $(ROOT)/etc errdead
	$(INS) -n $(ROOT)/etc errstop
	$(INS) -n $(ROOT)/usr/lib errdemon

errdead:	errdead.c
	$(CCLD1_CMD) -o errdead errdead.c $(LIB_LIST)

errdemon:	errdemon.c
	$(CCLD1_CMD) -o errdemon errdemon.c $(LIB_LIST)

errpt:	errpt.c
	$(CCLD1_CMD) -o errpt errpt.c $(LIB_LIST)

########
#
#	Standard Targets
#
#	all		builds all the products specified by PRODUCTS
#	clean		removes all temporary files (ex. installable object)
#	clobber		"cleans", and then removes $(PRODUCTS)
#
########

all:		$(PRODUCTS)

clean:

clobber:	clean
		rm -f  $(PRODUCTS)
