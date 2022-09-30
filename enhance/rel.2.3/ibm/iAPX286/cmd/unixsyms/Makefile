# %W%
# makefile for unixsyms
CC	= /bin/cc
LD	= /bin/ld
O	=
CFLAGS	= -Ml $O -I$(INCRT) -I$(INCRT1)
INCRT	= ../..
INCRT1	= /usr/include
INSDIR	= /usr/bin
FILES	=  unixsyms.c unixsyms2.c unixsyms3.c
LIBS	= -lld

all:	unixsyms

unixsyms: \
	$(FILES)\
	$(INCRT1)/stdio.h\
	$(INCRT1)/fcntl.h\
	$(INCRT1)/filehdr.h\
	$(INCRT1)/syms.h\
	$(INCRT1)/storclass.h\
	$(INCRT1)/scnhdr.h\
	$(INCRT1)/ldfcn.h\
	unixsyms.h
	$(CC) $(CFLAGS) $(LDFLAGS) -o unixsyms $(FILES) $(LIBS)

clean:
	-rm -f *.o

clobber: clean
	-rm -f unixsyms

install:        all
	rm -f $(INSDIR)/unixsyms
	mv unixsyms $(INSDIR)/unixsyms
	chmod 755 $(INSDIR)/unixsyms
	chgrp bin $(INSDIR)/unixsyms
	chown bin $(INSDIR)/unixsyms
