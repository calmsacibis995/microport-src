#	Copyright (c) 1985 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#/*   @(#)man.mk	1.3 - 85/08/09 */
#	makefile for man command and term help message.
#
# DSL 2.

OL = $(ROOT)/
INS = :
CINSDIR = ${OL}usr/bin
HINSDIR = ${OL}usr/lib/help
FILES = man.sh term
MAKE = make

compile all:  man termh
	:

man:
	cp man.sh man
	${INS} man ${CINSDIR}
	-if [ ${INS} != ":" ] ;\
	then cd ${CINSDIR}; chmod 755 man; chgrp bin man; chown bin man ;\
	fi
helpdir:
	-mkdir ${OL}usr/lib/help

termh:	helpdir
	${INS} term ${HINSDIR}
	-if [ ${INS} != ":" ] ;\
	then cd ${HINSDIR}; chmod 664 term;\
	chgrp bin term; chown bin term  ;\
	fi

install:
	${MAKE} -f man.mk INS=cp ROOT=$(ROOT) CH=$(CH)
insman:	;  ${MAKE} -f shells.mk INS=cp ROOT=$(ROOT) CH=$(CH) man
instermh: ; ${MAKE} -f shells.mk INS=cp ROOT=$(ROOT) CH=$(CH) termh

clean manclean:
	:
clobber:  clean
	rm -f man

manclobber:  ;  rm -f man
