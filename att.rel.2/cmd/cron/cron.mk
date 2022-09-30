#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#	@(#)	1.2

INS= :

ROOT=
ETC=$(ROOT)/etc
USR=$(ROOT)/usr
INSDIR=$(ROOT)/usr/bin
SPOOL=$(USR)/spool/cron
LIB=$(USR)/lib

CRONLIB=$(LIB)/cron
CRONSPOOL=$(SPOOL)/crontabs
ATSPOOL=$(SPOOL)/atjobs

XDIRS= $(ROOT) $(ETC) $(USR) $(INSDIR) $(LIB) $(SPOOL)\
      $(CRONLIB) $(CRONSPOOL) $(ATSPOOL)

DIRS= $(SPOOL)\
      $(CRONLIB) $(CRONSPOOL) $(ATSPOOL)

CMDS= cron at crontab batch

CFLAGS= -O
LDFLAGS= -s
DEFS=

.c.o:
	$(CC) $(CFLAGS) $(DEFS) -c $<

all:	$(CMDS)

install:	dirs
	make -f cron.mk INS="install -f" $(ARGS)

libelm.a: elm.o
	$(AR) cr libelm.a elm.o

cron:	cron.o funcs.o libelm.a
	$(CC) $(LDFLAGS) cron.o funcs.o libelm.a -o cron
	rm -f $(ETC)/OLDcron $(ETC)/cron
	$(INS) $(ETC) -o cron

crontab:	crontab.o permit.o funcs.o
	$(CC) $(LDFLAGS) crontab.o permit.o funcs.o -o crontab
	rm -f $(INSDIR)/crontab
	$(INS) $(INSDIR) crontab

at:	at.o att1.o att2.o funcs.o permit.o
	$(CC) $(LDFLAGS) at.o att1.o att2.o funcs.o permit.o -o at
	rm -f $(INSDIR)/at
	$(INS) $(INSDIR)  at

batch:	batch.sh
	cp batch.sh batch
	rm -f $(INSDIR)/batch
	$(INS) $(INSDIR) batch

att1.c att1.h:	att1.y
	yacc -d att1.y
	mv y.tab.c att1.c
	mv y.tab.h att1.h

att2.c:	att2.l
	lex att2.l
	ed - lex.yy.c < att2.ed
	mv lex.yy.c att2.c

att2.o:	att1.h

cron.o:	cron.c cron.h
crontab.o:	crontab.c cron.h
at.o:	at.c cron.h

dirs:	$(DIRS)

$(DIRS):
	-mkdir $@
	$(CH) -chmod 755 $@
	$(CH) -chgrp sys $@
	$(CH) -chown root $@

clean:
	rm -f *.o libelm.a

clobber:	clean
	rm -f $(CMDS)
