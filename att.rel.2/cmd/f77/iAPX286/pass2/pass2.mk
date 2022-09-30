# iAPX286 @(#)pass2.mk	1.3 85/09/06
#	@(#)pass2.mk	1.3
#	iAPX286 FORTRAN COMPILER PASS2 MAKEFILE


FCOM = ../../common/pass2
FMAC = .
PCOM = $(ROOT)/usr/src/cmd/sgs/comp/mip
PMAC = $(ROOT)/usr/src/cmd/sgs/comp/i286
CFLAGS =  -O -c -DFLT -DSPFORT -DFORT \
	 -I$(PCOM) -I$(PMAC) -I$(FCOM) -I$(FMAC) -DiAPX286
LDFLAGS = -s 
CC = cc
SIZE = size
STRIP = strip
INSTALL = install
INCLUDES = $(PCOM)/mfile2 $(PCOM)/manifest $(PMAC)/macdefs $(PMAC)/mac2defs

all:		f77pass2

f77pass2:	seg.o fort.o reader.o local2.o order.o match.o allo.o table.o comm2.o
		$(CC) $(LDFLAGS) fort.o reader.o local2.o order.o match.o \
			seg.o allo.o table.o comm2.o -o f77pass2
		@$(SIZE) f77pass2

fort.o:		$(FCOM)/fort.c $(FMAC)/fort.h $(INCLUDES)
		$(CC) $(CFLAGS) $(FCOM)/fort.c

reader.o:	$(PCOM)/reader.c $(PCOM)/messages.h $(INCLUDES)
		$(CC) $(CFLAGS) -DNOMAIN $(PCOM)/reader.c

local2.o:	$(PMAC)/local2.c $(INCLUDES) $(ROOT)/usr/include/ctype.h
		$(CC) $(CFLAGS) $(PMAC)/local2.c

order.o:	$(PMAC)/order.c $(INCLUDES)
		$(CC) $(CFLAGS) $(PMAC)/order.c

seg.o:	$(PMAC)/seg.c $(INCLUDES)
		$(CC) $(CFLAGS) $(PMAC)/seg.c

match.o:	$(PCOM)/match.c $(INCLUDES)
		$(CC) $(CFLAGS) $(PCOM)/match.c

allo.o:		$(PCOM)/allo.c $(INCLUDES)
		$(CC) $(CFLAGS) $(PCOM)/allo.c

table.o:	$(PMAC)/table.c $(INCLUDES)
		$(CC) $(CFLAGS) $(PMAC)/table.c

comm2.o:	$(PCOM)/comm2.c $(PCOM)/common $(INCLUDES)
		$(CC) $(CFLAGS) $(PCOM)/comm2.c

install:
		$(STRIP) f77pass2
		$(INSTALL) -f $(ROOT)/usr/lib f77pass2

clean:
	-rm -f *.o

clobber:
	-rm -f *.o f77pass2
