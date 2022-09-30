#       @(#)linkkit.mk	2.3 - 9/30/87
SYS = system5
NODE = system5
REL = 2
VER = 230.10-I
MACH = AT

SRCDIR = $(ROOT)/src/rel$(VER)

INS = $(ROOT)/etc/install
INSDIR = $(ROOT)

NAME = $(SYS)
TYPE = iAPX286
CONFIG = ../cmd/config/config
UNIXSYMS=../cmd/unixsyms/unixsyms

INCRT = $(ROOT)/usr/include
INCRT1 = ..
INCRT2 = ../sys
CFLAGS = -I$(INCRT1) -I$(INCRT2) -I$(INCRT) -Ml
CC = cc
AR = ar
LD = ld
LDFLAGS = -i
FRC =

FILES = linesw.o       \
	conf.o         \
	handlers.o     \
	gdt.o          \
	kernio.o       \
	name.o

all:	wini

current.wini: master dfile.wini 
	rm -f $(FILES) 
	echo $(NAME):
	$(CONFIG) -t -m master dfile.wini
	rm -f current.flop

current.flop: master dfile.flop 
	rm -f $(FILES) 
	echo $(NAME): 
	$(CONFIG) -t -m master dfile.flop 
	rm -f current.wini 


conf.o:
	$(CC) $(CFLAGS) -c conf.c

gdt.o:
	$(CC) $(CFLAGS) -c gdt.s

handlers.o:
	$(CC) $(CFLAGS) -c handlers.c

kernio.o:
	$(CC) $(CFLAGS) -c kernio.c
	-@$(CC) -E -C $(CFLAGS) kernio.c | grep -s '*!'

.s.o:
	$(CC) $(CFLAGS) -c $*.s

wini: current.wini name.o $(FILES)
	-$(LD) $(LDFLAGS) -o ../$(NAME) -e strt -K\
	    gdt.o conf.o lomem.o linesw.o buffers.o handlers.o kernio.o name.o\
		    ../ml.o $(DEBUGLIB) ../lib[1-9] ifile -la
		if [ "$(DEBUGLIB)" ]; then $(UNIXSYMS) ../$(NAME); fi
		-rm -f current* 
		-touch current.wini
		-/bin/nm -x ../$(NAME) > ../$(NAME).nm
		-chgrp bin ../$(NAME)
		-chmod 664 ../$(NAME)
		-chown bin ../$(NAME)


flop:   current.flop name.o $(FILES)
	-$(LD) $(LDFLAGS) -o ../$(NAME) -e strt -K\
	    gdt.o conf.o lomem.o linesw.o buffers.o handlers.o kernio.o name.o\
		    ../ml.o ../lib[1-9] ifile -la
		-rm -f current*
		-touch current.flop
		-/bin/nm -x ../$(NAME) > ../$(NAME).nm
		-chgrp bin ../$(NAME)
		-chmod 664 ../$(NAME)
		-chown bin ../$(NAME)

name.o: name.c
	$(CC) $(CFLAGS) -c \
		-DSYS=\"`expr $(SYS) : '\(.\{1,8\}\)'`\" \
		-DNODE=\"`expr $(NODE) : '\(.\{1,8\}\)'`\" \
		-DREL=\"`expr $(REL) : '\(.\{1,8\}\)'`\" \
		-DVER=\"`expr $(VER) : '\(.\{1,8\}\)'`\" \
		-DMACH=\"`expr $(MACH) : '\(.\{1,8\}\)'`\" \
		name.c

install:
	mv /system5 /system5.last
	rm -f /unix
	cp ../$(NAME) /system5
	ln /system5 /unix

clean:
	-rm -f $(FILES) current*

clobber:
	-rm -f $(FILES) current*
	-rm -f config.h conf.c handlers.c
	-rm -f ../$(NAME)

FRC:
