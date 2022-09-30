#       @(#)cf.mk	1.9 - 85/09/13
SYS = unix
NODE = unix
REL = 2
VER = 1
MACH = iAPX286

SRCDIR = $(ROOT)/usr/src/uts

INS = $(ROOT)/etc/install
INSDIR = $(ROOT)

NAME = $(SYS)$(VER)
TYPE = iAPX286

INCRT = $(ROOT)/usr/include
CFLAGS = -I$(INCRT) -Ml
CC = cc
AR = ar
LD = ld
LDFLAGS = -i
STRIP = $(PFX)strip
FRC =

FILES = linesw.o       \
	conf.o         \
	handlers.o     \
	buffers.o      \
	gdt.o          \
	name.o

.IGNORE:
all:
	@echo 'make what? (flop or wini20 or wini40)'

current.wini20:
	-@if [ ! -r current.wini20 ] ;\
	then\
		rm -f $(FILES) ;\
		echo $(NAME): ;\
		$(PFX)config -t -m $(ROOT)/etc/master dfile.20w ;\
	fi

current.wini40:
	-@if [ ! -r current.wini40 ] ;\
	then\
		rm -f $(FILES) ;\
		echo $(NAME): ;\
		$(PFX)config -t -m $(ROOT)/etc/master dfile.40w ;\
	fi

current.flop:
	-@if [ ! -r current.flop ] ;\
	then\
		rm -f $(FILES) ;\
		echo $(NAME): ;\
		$(PFX)config -t -m $(ROOT)/etc/master dfile.f ;\
	fi

buffers.o:
	$(CC) $(CFLAGS) -S buffers.c;\
	ed buffers.s < commtobss;\
	$(CC) $(CFLAGS) -c buffers.s

conf.o:
	$(CC) $(CFLAGS) -S conf.c;\
	ed conf.s < commtobss;\
	$(CC) $(CFLAGS) -c conf.s

.s.o:
	$(CC) $(CFLAGS) -c $*.s

wini20: init current.wini20 $(FILES)
	-$(LD) $(LDFLAGS) -o ../$(NAME) -e strt -K\
		 gdt.o ../ml.o conf.o linesw.o buffers.o handlers.o name.o\
		 ../lib[0-9] ifile -la
		 -rm -f current*
		 -touch current.wini20
		 -$(STRIP) -x ../$(NAME)
		-chgrp bin ../$(NAME)
		-chmod 664 ../$(NAME)
		-chown bin ../$(NAME)

wini40: init current.wini40 $(FILES)
	-$(LD) $(LDFLAGS) -o ../$(NAME) -e strt -K\
		 gdt.o ../ml.o conf.o linesw.o buffers.o handlers.o name.o\
		 ../lib[0-9] ifile -la
		 -rm -f current*
		 -touch current.wini40
		 -$(STRIP) -x ../$(NAME)
		-chgrp bin ../$(NAME)
		-chmod 664 ../$(NAME)
		-chown bin ../$(NAME)

flop:   init current.flop $(FILES)
	-$(LD) $(LDFLAGS) -o ../$(NAME) -e strt -K\
		 gdt.o ../ml.o conf.o linesw.o buffers.o handlers.o name.o\
		 ../lib[0-9] ifile -la
		 -rm -f current*
		 -touch current.flop
		 -$(STRIP) ../$(NAME)
		-chgrp bin ../$(NAME)
		-chmod 664 ../$(NAME)
		-chown bin ../$(NAME)

init:
	-rm -f name.o
	cd $(SRCDIR); make -f uts.mk "INS=$(INS)" "INSDIR=$(INSDIR)" \
		"INCRT=$(INCRT)" "FRC=$(FRC)" "SYS=$(SYS)" \
		"NODE=$(NODE)" "REL=$(REL)" "VER=$(VER)" "MACH=$(MACH)"\
		"TYPE=$(TYPE)"

name.o: name.c
	$(CC) $(CFLAGS) -c \
		-DSYS=\"`expr $(SYS) : '\(.\{1,8\}\)'`\" \
		-DNODE=\"`expr $(NODE) : '\(.\{1,8\}\)'`\" \
		-DREL=\"`expr $(REL) : '\(.\{1,8\}\)'`\" \
		-DVER=\"`expr $(VER) : '\(.\{1,8\}\)'`\" \
		-DMACH=\"`expr $(MACH) : '\(.\{1,8\}\)'`\" \
		name.c

install:
	$(INS) -f $(INSDIR) "../$(SYS)$(VER)"

clean:
	cd $(SRCDIR); make -f uts.mk "INS=$(INS)" "INSDIR=$(INSDIR)" \
		"INCRT=$(INCRT)" "FRC=$(FRC)" "SYS=$(SYS)" \
		"NODE=$(NODE)" "REL=$(REL)" "VER=$(VER)" "MACH=$(MACH)"\
		"TYPE=$(TYPE)" clean
	-rm -f *.o current*

clobber:
	cd $(SRCDIR); make -f uts.mk "INS=$(INS)" "INSDIR=$(INSDIR)" \
		"INCRT=$(INCRT)" "FRC=$(FRC)" "SYS=$(SYS)" \
		"NODE=$(NODE)" "REL=$(REL)" "VER=$(VER)" "MACH=$(MACH)"\
		"TYPE=$(TYPE)" clobber
	-rm -f *.o current*
	-rm -f config.h conf.c handlers.c
	-rm -f ../$(NAME)
FRC:
