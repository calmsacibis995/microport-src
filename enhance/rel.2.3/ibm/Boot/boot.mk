#	Copyright (c) 1985 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#   @(#)boot.mk	1.7 - 85/08/31
# Modification History
# M000:	uport!dwight Tue Feb 11 15:54:52 PST 1986
#	Modified to create an IBM AT big boot. All files must
#	be compiled with IBMAT defined, along with one or more of the
#	following flags:
#		HARDDISK:	For a big boot that lives on the harddisk.
#		LOWDENSITY:	A low density (48 tpi, 9 spt) floppy.
#		HIDENSITY:	A hi density (96 tpi, 15 spt) floppy.
#
#	make boot.flld:		For a low density floppy
#	make boot.flhd:		For a high density floppy
#	make boot.hd:		Hard disk
#
#	make install_flld:	installs low density big boot on floppy
#	make install:		installs hard disk big boot on $(TARGET)
#
# M001:	uport!dwight Sun Mar  9 10:09:35 PST 1986
#	Now supports all types of hard disks. Assumes that the
#	disk info is passed via little boot.

AS=$(PFX)as
CC=$(PFX)cc
DD=$(PFX)dd
LD=$(PFX)ld
MV=$(PFX)mv
RM=$(PFX)rm

# where `make install` will put your big boot
TARGET=/dev/rdsk/0s255

CFLAGS= -O
AFLAGS=
LFLAGS=

OBJ=	bboot.o bload.o butil1.o
OBJ_HD  =	bboot.hd.o bload.hd.o butil1.hd.o
OBJ_FLLD=	bboot.flld.o bload.flld.o butil1.flld.o
OBJ_FLHD=	bboot.flhd.o bload.flhd.o butil1.flhd.o
SRC=	bboot.s bload.c butil1.c
INS=    install
INSDIR= $(ROOT)/etc

# default make is for an ibm at
all: boot.hd

# previous all definition. from the generic release.
all.generic:	boot

# standard ibm at (e.g. ncr box):
boot.hd ncr mad:	$(OBJ_HD)
	$(LD) $(LFLAGS) -R -s  -o boot.hd $(OBJ_HD) ifile

# low density floppy
boot.flld:	$(OBJ_FLLD)
	$(LD) $(LFLAGS) -R -s  -o boot.flld $(OBJ_FLLD) ifile

# hi density floppy
boot.flhd:	$(OBJ_FLHD)
	$(LD) $(LFLAGS) -R -s  -o boot.flhd $(OBJ_FLHD) ifile

boot: $(OBJ)
	$(LD) $(LFLAGS) -R -s  -o boot $(OBJ) ifile

bboot.o: bboot.s
	$(CC)  -c  $(CFLAGS) bboot.s

butil1.o: butil1.c
	$(CC)  -c  $(CFLAGS) butil1.c

bload.o: bload.c
	$(CC)  -c  $(CFLAGS) bload.c

clean:
	rm -f $(OBJ) $(OBJ_HD) $(OBJ_FLLD) $(OBJ_FLHD)

clobber: clean
	rm -f boot boot.hd boot.flld boot.flhd

install: all
	$(DD) if=boot.hd of=$(TARGET) bs=1b count=8

# Previous generic installation:
install_generic: all.generic
	$(INS) -n $(INSDIR) boot

install_flld:	boot.flld
	$(DD) if=boot.flld of=/dev/dsk/0s24 bs=1b seek=9 count=8

bboot.hd.o:	bboot.s
	$(CC)  -c  $(CFLAGS) -DIBMAT -DHARDDISK bboot.s
	$(MV) bboot.o bboot.hd.o

bboot.flld.o:	bboot.s
	$(CC)  -c  $(CFLAGS) -DIBMAT -DLOWDENSITY bboot.s
	$(MV) bboot.o bboot.flld.o

bboot.flhd.o:	bboot.s
	$(CC)  -c  $(CFLAGS) -DIBMAT -DHIDENSITY bboot.s 
	$(MV) bboot.o bboot.flhd.o

butil1.hd.o:	butil1.c
	$(CC)  -c  $(CFLAGS) -DIBMAT -DHARDDISK butil1.c
	$(MV) butil1.o butil1.hd.o

butil1.flld.o:	butil1.c
	$(CC)  -c  $(CFLAGS) -DIBMAT -DLOWDENSITY butil1.c
	$(MV) butil1.o butil1.flld.o

butil1.flhd.o:	butil1.c
	$(CC)  -c  $(CFLAGS) -DIBMAT -DHIDENSITY butil1.c 
	$(MV) butil1.o butil1.flhd.o

bload.hd.o:	bload.c
	$(CC)  -c  $(CFLAGS) -DIBMAT -DHARDDISK bload.c
	$(MV) bload.o bload.hd.o

bload.flld.o:	bload.c
	$(CC)  -c  $(CFLAGS) -DIBMAT -DLOWDENSITY bload.c
	$(MV) bload.o bload.flld.o

bload.flhd.o:	bload.c
	$(CC)  -c  $(CFLAGS) -DIBMAT -DHIDENSITY bload.c
	$(MV) bload.o bload.flhd.o
