#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#	@(#)	1.4
#
# makefile for libmalloc
#

.SUFFIXES: .p
ROOT=
AR=
ARFLAGS=r
CC=cc
CFLAGS=-O
CP=cp
ENV=
INC= $(ROOT)/usr/include
LIB = $(ROOT)/usr/lib
LIBP = $(ROOT)/usr/lib/libp
LINT=lint
LINTFLAGS=-p
MV=mv
OBJECTS=malloc.o
POBJECTS=malloc.p
RM=rm
SGS=
STRIP=strip
SPFLAGS=-r
MODEL=
MSIZE=

all: noprof prof llib-lmalloc.l

noprof: $(OBJECTS)
	$(RM) -rf objects
	mkdir objects
	$(CP) $(OBJECTS) objects
	cd objects; $(AR) $(ARFLAGS) libmalloc.a *

prof: $(POBJECTS)
	$(RM) -rf pobjects
	mkdir pobjects
	for i in $(POBJECTS);\
	do\
		$(CP) $$i pobjects/`basename $$i .p`.o;\
	done
	cd pobjects; $(AR) $(ARFLAGS) libmalloc.a *

malloc.o malloc.p: $(INC)/malloc.h mallint.h

.c.o .c.p:
	@echo $*.c:
	$(PROF)$(CC) -p $(CFLAGS) -M$(MSIZE) -c $*.c    && $(MV) $(*F).o $*.p
	$(CC) $(CFLAGS) -M$(MSIZE) -c $*.c

llib-lmalloc.l:  llib-lmall.c
	cp llib-lmall.c llib-lmalloc.c
	lint -c llib-lmalloc.c
	rm llib-lmalloc.c

install: all
	if [ ! -d $(LIBP)/$(MODEL) ];\
	then\
		mkdir $(LIBP)/$(MODEL);\
	fi
	if [ ! -d $(LIB)/$(MODEL) ];\
	then\
		mkdir $(LIB)/$(MODEL);\
	fi
	$(CP) objects/libmalloc.a $(LIB)/$(MODEL)/libmalloc.a
	$(CP) pobjects/libmalloc.a $(LIBP)/$(MODEL)/libmalloc.a
	if pdp11; then \
		$(STRIP) $(LIB)/libmalloc.a; \
		$(STRIP) $(LIBP)/libmalloc.a; \
	else \
		$(STRIP) $(SPFLAGS) $(LIB)/$(MODEL)/libmalloc.a; \
		$(STRIP) $(SPFLAGS) $(LIBP)/$(MODEL)/libmalloc.a; \
		$(AR) ts $(LIB)/$(MODEL)/libmalloc.a; \
		$(AR) ts $(LIBP)/$(MODEL)/libmalloc.a; \
	fi
	$(CP) llib-lmalloc.l $(LIB)/$(MODEL)/llib-lmalloc.l

clean: 
	$(RM) -fr pobjects objects

clobber: clean
	$(RM) -f $(POBJECTS) $(OBJECTS)
