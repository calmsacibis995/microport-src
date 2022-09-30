#	iAPX286 @(#)pass1.mk	1.2 85/09/06
#	IAPX286 FORTRAN COMPILER PASS1 MAKEFILE

DEFS = $(CFLAGS) -DHERE=VAX -DTARGET=IAPX286 -DFAMILY=PCC \
	-I$(DIRMAC) -I$(DIRCOM)
PRX =
DIRCOM = ../../common/pass1
DIRMAC = .
MODEL = -Ml
CFLAGS = -O
LDFLAGS = -Ml
CC = $(PRX)cc
SIZE = $(PRX)size
STRIP = $(PRX)strip
INSTALL = install

OBJECTS = main.o init.o gram.o lex.o proc.o equiv.o data.o \
	  expr.o exec.o intr.o io.o misc.o error.o put.o \
	  putpcc.o iAPX286.o iAPX286l.o
INCLUDES = $(DIRCOM)/defs $(DIRCOM)/defines $(DIRMAC)/machdefs $(DIRCOM)/ftypes

GRAMMAR = $(DIRCOM)/gram.head $(DIRCOM)/gram.dcl $(DIRCOM)/gram.expr \
 	  $(DIRCOM)/gram.exec $(DIRCOM)/gram.io

all : f77 f77pass1

# driver in small model

f77:		driver.o iAPX286s.o
		$(CC) driver.o iAPX286s.o -o f77
		@$(SIZE) f77

driver.o:	$(DIRCOM)/driver.c $(DIRMAC)/drivedefs $(INCLUDES)
		$(CC) -c $(DEFS) $(DIRCOM)/driver.c

iAPX286s.o:	$(DIRMAC)/iAPX286x.c $(INCLUDES)
		$(CC) -c $(DEFS) $(DIRMAC)/iAPX286x.c
		mv iAPX286x.o iAPX286s.o

# f77pass1 in large model

f77pass1:	$(OBJECTS)
		$(CC) $(LDFLAGS) $(OBJECTS) -o f77pass1
		@$(SIZE) f77pass1

main.o:		$(DIRCOM)/main.c $(INCLUDES)
		$(CC) -c $(MODEL) $(DEFS) $(DIRCOM)/main.c

init.o:		$(DIRCOM)/init.c $(INCLUDES)
		$(CC) -c $(MODEL) $(DEFS) $(DIRCOM)/init.c

gram.o:		gram.c $(INCLUDES)
		$(CC) -c $(MODEL) $(DEFS) gram.c

lex.o:		$(DIRCOM)/lex.c $(INCLUDES) tokdefs
		$(CC) -c $(MODEL) $(DEFS) $(DIRCOM)/lex.c

proc.o:		$(DIRCOM)/proc.c $(INCLUDES)
		$(CC) -c $(MODEL) $(DEFS) $(DIRCOM)/proc.c

equiv.o:	$(DIRCOM)/equiv.c $(INCLUDES)
		$(CC) -c $(MODEL) $(DEFS) $(DIRCOM)/equiv.c

data.o:		$(DIRCOM)/data.c $(INCLUDES)
		$(CC) -c $(MODEL) $(DEFS) $(DIRCOM)/data.c

expr.o:		$(DIRCOM)/expr.c $(INCLUDES)
		$(CC) -c $(MODEL) $(DEFS) $(DIRCOM)/expr.c

exec.o:		$(DIRCOM)/exec.c $(INCLUDES)
		$(CC) -c $(MODEL) $(DEFS) $(DIRCOM)/exec.c

intr.o:		$(DIRCOM)/intr.c $(INCLUDES)
		$(CC) -c $(MODEL) $(DEFS) $(DIRCOM)/intr.c

io.o:		$(DIRCOM)/io.c $(INCLUDES)
		$(CC) -c $(MODEL) $(DEFS) $(DIRCOM)/io.c

misc.o:		$(DIRCOM)/misc.c $(INCLUDES)
		$(CC) -c $(MODEL) $(DEFS) $(DIRCOM)/misc.c

error.o:	$(DIRCOM)/error.c $(INCLUDES)
		$(CC) -c $(MODEL) $(DEFS) $(DIRCOM)/error.c

put.o:		$(DIRCOM)/put.c $(INCLUDES) $(DIRCOM)/pccdefs
		$(CC) -c $(MODEL) $(DEFS) $(DIRCOM)/put.c

putpcc.o:	$(DIRCOM)/putpcc.c $(INCLUDES) $(DIRCOM)/pccdefs
		$(CC) -c $(MODEL) $(DEFS) $(DIRCOM)/putpcc.c

iAPX286.o:	$(DIRMAC)/iAPX286.c $(INCLUDES) $(DIRCOM)/pccdefs
		$(CC) -c $(MODEL) $(DEFS) $(DIRMAC)/iAPX286.c

iAPX286l.o:	$(DIRMAC)/iAPX286x.c $(INCLUDES)
		$(CC) -c $(MODEL) $(DEFS) $(DIRMAC)/iAPX286x.c
		mv iAPX286x.o iAPX286l.o

gram.c:		$(GRAMMAR) tokdefs
		( sed < tokdefs "s/#define/%token/" ; \
 		cat $(GRAMMAR) ) > gram.in
		$(YACC) $(YFLAGS) gram.in
		@echo "expect 4 shift/reduce"
		mv y.tab.c gram.c
		rm gram.in
 
tokdefs:	$(DIRCOM)/tokens
		grep -n "^[^#]" < $(DIRCOM)/tokens | \
		sed "s/\([^:]*\):\(.*\)/#define \2 \1/" > tokdefs

install:
	$(STRIP) f77
	$(INSTALL) -f $(ROOT)/usr/bin f77
	$(STRIP) f77pass1
	$(INSTALL) -f $(ROOT)/usr/lib f77pass1

clean:
	-rm -f gram.c $(OBJECTS) driver.o iAPX286s.o

clobber:	clean
	-rm -f f77pass1 f77

