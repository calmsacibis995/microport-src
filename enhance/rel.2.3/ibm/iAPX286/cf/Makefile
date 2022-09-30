#	@(#)cf Makefile	 2.3 - 1/7/88; sccsid = 
SYS = system5
NODE = system5
REL = 2
VER = 2.3.0-U
MACH = AT

SRCDIR = $(ROOT)/src/rel$(VER)

INS = $(ROOT)/etc/install
INSDIR = $(ROOT)

NAME = $(SYS)
TYPE = iAPX286
CONFIG = $(SYSDIR)/cmd/config/config
UNIXSYMS = $(SYSDIR)/cmd/unixsyms/unixsyms

SYSDIR = ../../iAPX286
SRC    = $(SYSDIR)/cf
INCRT  = $(ROOT)/usr/include
INCRT0 = .
INCRT1 = $(SYSDIR)
INCRT2 = $(SYSDIR)/sys

# DEFINES = -DATMERGE -DLCCFIX  -DATPROBE
DEFINES = -DLCCFIX $(ATMERGE)
CFLAGS = -I$(INCRT0) -I$(INCRT1) -I$(INCRT2) -I$(INCRT) -Ml $(DEFINES)
LDFLAGS = -i
FRC =

FILES = linesw.o   \
	conf.o     \
	handlers.o \
	buffers.o  \
	lomem.o    \
	gdt.o      \
	kernio.o   \
	name.o

all:	wini

current.wini: master dfile.wini 
	rm -f $(FILES) 
	echo $(NAME):
	$(CONFIG) -t -m master dfile.wini
	rm -f current.flop

current.flop: master.flop dfile.flop
	rm -f $(FILES) 
	echo $(NAME): 
	$(CONFIG) -t -m master.flop dfile.flop 
	rm -f current.wini 


buffers.o: $(SRC)/buffers.c
	$(CC) $(CFLAGS) -c $(SRC)/buffers.c

conf.o: $(SRC)/conf.c
	$(CC) $(CFLAGS) -c $(SRC)/conf.c

gdt.o: $(SRC)/gdt.s
	$(CC) $(CFLAGS) -c $(SRC)/gdt.s

handlers.o: $(SRC)/handlers.c
	$(CC) $(CFLAGS) -c $(SRC)/handlers.c

kernio.o:
	$(CC) $(CFLAGS) -c $(SRC)/kernio.c
	-@$(CC) -E -C $(CFLAGS) $(SRC)/kernio.c | grep -s '*!'

linesw.o: $(SRC)/linesw.c
	$(CC) $(CFLAGS) -c $(SRC)/linesw.c

lomem.o: $(SRC)/lomem.c
	$(CC) $(CFLAGS) -c $(SRC)/lomem.c

.s.o:
	$(CC) $(CFLAGS) -c $*.s

wini: current.wini name.o $(FILES)
	-$(LD) $(LDFLAGS) -o ../$(NAME) -e strt -K\
	     gdt.o conf.o lomem.o linesw.o buffers.o handlers.o kernio.o name.o\
		 ../ml.o $(DEBUGLIB) ../lib[0-9] ifile -la
		size ../$(NAME)
		if [ "$(DEBUGLIB)" ]; then $(UNIXSYMS) ../$(NAME); fi
		 -rm -f current* 
		 -touch current.wini
#		-/bin/nm -x ../$(NAME) > ../$(NAME).nm
#		-chgrp bin ../$(NAME)
#		-chmod 664 ../$(NAME)
#		-chown bin ../$(NAME)
#		-cd ..; dosyms

flop:   current.flop name.o $(FILES) lomem.o
	-$(LD) $(LDFLAGS) -o ../$(NAME) -e strt -K\
	     gdt.o conf.o lomem.o linesw.o buffers.o handlers.o kernio.o name.o\
		 ../ml.o $(DEBUGLIB) ../lib[0-9] ifile -la
		size ../$(NAME)
		if [ "$(DEBUGLIB)" ]; then $(UNIXSYMS) ../$(NAME); fi
		 -rm -f current*
		 -touch current.flop
#		-/bin/nm -x ../$(NAME) > ../$(NAME).nm
#		-chgrp bin ../$(NAME)
#		-chmod 664 ../$(NAME)
#		-chown bin ../$(NAME)

init: name.o
	echo System V/286 "'"$(SYS)"'", version "#"$(VER), release "#"$(REL), for the $(MACH)

name.o: $(SRC)/name.c $(SRC)/Makefile
	$(CC) $(CFLAGS) -c \
		-DSYS=\"`expr $(SYS) : '\(.\{1,8\}\)'`\" \
		-DNODE=\"`expr $(NODE) : '\(.\{1,8\}\)'`\" \
		-DREL=\"`expr $(REL) : '\(.\{1,8\}\)'`\" \
		-DVER=\"`expr $(VER) : '\(.\{1,8\}\)'`\" \
		-DMACH=\"`expr $(MACH) : '\(.\{1,8\}\)'`\" \
		$(SRC)/name.c

install:
	mv /system5 /system5.last
	rm -f /unix
	cp ../$(NAME) /system5
	ln /system5 /unix

clean:
	-rm -f $(FILES) current*

clobber:
	-rm -f *.o current*
	-rm -f config.h conf.c handlers.c
	-rm -f ../$(NAME)

dfile.wini:
	cp $(SRC)/dfile.wini .

dfile.flop:
	cp $(SRC)/dfile.flop .

ifile:
	cp $(SRC)/ifile .

FRC:
