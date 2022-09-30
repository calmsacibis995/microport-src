# @(#)Makefile	1.7
# makefile for various console support utilities

ALL	= $(KEYRTNS) $(CONRTNS) mkcons
CC	= /bin/cc
CFLAGS	= $O -I$(INCRT) -I/usr/include 
CONLIB	= libcons.a
CONLIBO	= ccon.o dcon.o dgraf.o gcontty.o
CONRTNS	= delecons chgcons delegraf
INCRT	= ../..
INSTALL	= /etc/install -f $(ROOT)/etc -u bin -g sys -m 510
KEYRTNS	= prtkey rawkey setkey
O	= -s -O
ROOT	= ../../../dist

all:	$(ALL)

$(CONRTNS):	$(CONLIB) $@
	$(CC) $(CFLAGS) $@.c $(CONLIB) -o $@

$(CONLIB):	$(CONLIBO)
	rm -f $(CONLIB)
	ar q $(CONLIB) $(CONLIBO)

clean:
	rm -f *.o *.a

clobber: clean
	rm -f $(ALL)

install: $(ALL)
	for i in $(ALL); do $(INSTALL) ./$$i; done
	-ln $(ROOT)/etc/setkey $(ROOT)/etc/keyset	# historical
