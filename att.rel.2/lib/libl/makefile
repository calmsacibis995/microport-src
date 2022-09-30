#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#	@(#)	1.4
ROOT=
ULIBDIR= $(ROOT)/usr/lib

CC=
CFLAGS= -M$(MSIZE)
LORDER=
LIBOBJS=allprint.o main.o reject.o yyless.o yywrap.o

all:     $(ULIBDIR)/$(MODEL)/libl.a

$(ULIBDIR)/$(MODEL)/libl.a: $(LIBOBJS)
	$(AR) rv tmplib.a `$(LORDER) *.o | tsort`;

install:  ncform nrform all
	cp tmplib.a $(ULIBDIR)/$(MODEL)/libl.a;

clean:
	-rm -f *.o

clobber:	clean
	-rm -f tmplib.a 

allprint.o:	lib/allprint.c
	$(CC) -c $(CFLAGS) lib/allprint.c
main.o:		lib/main.c
	$(CC) -c $(CFLAGS) lib/main.c
reject.o:		lib/reject.c
	$(CC) -c $(CFLAGS) lib/reject.c
yyless.o:		lib/yyless.c
	$(CC) -c $(CFLAGS) lib/yyless.c
yywrap.o:		lib/yywrap.c
	$(CC) -c $(CFLAGS) lib/yywrap.c

ncform:	lib/ncform
	if [ ! -d $(ULIBDIR)/lex ] ;\
	then \
		mkdir $(ULIBDIR)/lex ;\
	fi
	rm -f $(ULIBDIR)/lex/ncform;
	cp lib/ncform $(ULIBDIR)/lex/ncform;

nrform:	lib/nrform
	if [ ! -d $(ULIBDIR)/lex ] ;\
	then \
		mkdir $(ULIBDIR)/lex ;\
	fi
	rm -f $(ULIBDIR)/lex/nrform;
	cp lib/nrform $(ULIBDIR)/lex/nrform;
