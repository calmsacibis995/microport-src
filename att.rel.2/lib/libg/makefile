#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#	@(#)	1.1

ROOT=
MODEL=
MSIZE=
FULLPATH=$(ROOT)/usr/src/lib/libg

all:
	if vax; then							\
		cd $(FULLPATH)/vax;					\
		$(MAKE) -f libg.mk all ROOT=$(ROOT);			\
	elif u3b; then							\
		cd $(FULLPATH)/u3b;					\
		$(MAKE) -f libg.mk all ROOT=$(ROOT);			\
	elif u3b5; then							\
		cd $(FULLPATH)/u3b5;					\
		$(MAKE) -f libg.mk all ROOT=$(ROOT);			\
	elif u3b2; then							\
		cd $(FULLPATH)/u3b2;					\
		$(MAKE) -f libg.mk all ROOT=$(ROOT);			\
	elif iAPX286; then						\
		cd $(FULLPATH)/iAPX286;					\
		$(MAKE) -f libg.mk all ROOT=$(ROOT) MODEL=$(MODEL)	\
			MSIZE=$(MSIZE);					\
	fi;

install:	all
	if vax; then							\
		cd $(FULLPATH)/vax;					\
		$(MAKE) -f libg.mk install ROOT=$(ROOT);		\
	elif u3b; then							\
		cd $(FULLPATH)/u3b;					\
		$(MAKE) -f libg.mk install ROOT=$(ROOT);		\
	elif u3b5; then							\
		cd $(FULLPATH)/u3b5;					\
		$(MAKE) -f libg.mk install ROOT=$(ROOT);		\
	elif u3b2; then							\
		cd $(FULLPATH)/u3b2;					\
		$(MAKE) -f libg.mk install ROOT=$(ROOT);		\
	elif iAPX286; then						\
		cd $(FULLPATH)/iAPX286;					\
		$(MAKE) -f libg.mk install MODEL=$(MODEL) ROOT=$(ROOT);	\
	fi;

clean:
	if vax; then							\
		cd $(FULLPATH)/vax;					\
		$(MAKE) -f libg.mk clean ROOT=$(ROOT);			\
	elif u3b; then							\
		cd $(FULLPATH)/u3b;					\
		$(MAKE) -f libg.mk clean ROOT=$(ROOT);			\
	elif u3b5; then							\
		cd $(FULLPATH)/u3b5;					\
		$(MAKE) -f libg.mk clean ROOT=$(ROOT);			\
	elif u3b2; then							\
		cd $(FULLPATH)/u3b2;					\
		$(MAKE) -f libg.mk clean ROOT=$(ROOT);			\
	elif iAPX286; then						\
		cd $(FULLPATH)/iAPX286;					\
		$(MAKE) -f libg.mk clean ROOT=$(ROOT);			\
	fi;

clobber:
	if vax; then							\
		cd $(FULLPATH)/vax;					\
		$(MAKE) -f libg.mk clobber ROOT=$(ROOT);		\
	elif u3b; then							\
		cd $(FULLPATH)/u3b;					\
		$(MAKE) -f libg.mk clobber ROOT=$(ROOT);		\
	elif u3b5; then							\
		cd $(FULLPATH)/u3b5;					\
		$(MAKE) -f libg.mk clobber ROOT=$(ROOT);		\
	elif u3b2; then							\
		cd $(FULLPATH)/u3b2;					\
		$(MAKE) -f libg.mk clobber ROOT=$(ROOT);		\
	elif iAPX286; then							\
		cd $(FULLPATH)/iAPX286;					\
		$(MAKE) -f libg.mk clobber ROOT=$(ROOT);		\
	fi;
