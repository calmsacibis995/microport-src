#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#	@(#)	1.2
########
#
#	sysdef makefile for dec
#
########
#
#	Standard Macros
#
########
CC = cc
CCLD1_CMD = $(CCLD_CMD) $(PPDEFS) $(CFLAGS) $(INC_LIST)
CCLD_CMD = $(CC) $(LDFLAGS)
CFLAGS = -O -s -Ml
LDFLAGS = 
PPDEFS =

INC_LIST	=

INS = /etc/install

all:
install:	all

PRODUCTS = sysdef

UINC = $(ROOT)/usr/include

install:
	$(INS) -n $(ROOT)/etc sysdef

sysdef:	sysdef.c
	$(CCLD1_CMD) -o sysdef sysdef.c -lld -lc -la

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
		rm -f $(PRODUCTS)
