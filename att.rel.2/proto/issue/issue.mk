ROOT =
CC = cc
ISSDIR = $(ROOT)/issue
ADMDIR = $(ROOT)/etc
TMPDIR = $(ROOT)/tmp

CFLAGS = -O -s

all: CPIO cpio_out filelist init_disk psplit kmount basecheck update

CPIO: CPIO.c
	$(CC) $(CFLAGS) CPIO.c -o CPIO

cpio_out: cpio_out.c
	$(CC) $(CFLAGS) cpio_out.c -o cpio_out

filelist: filelist.c
	$(CC) $(CFLAGS) filelist.c -o filelist

init_disk: init_disk.c
	$(CC) $(CFLAGS) init_disk.c -o init_disk

psplit: psplit.c
	$(CC) $(CFLAGS) psplit.c -o psplit

kmount: kmount.c
	$(CC) -s kmount.c -o kmount

basecheck: basecheck.c
	$(CC) $(CFLAGS) basecheck.c -o basecheck

update: update.c
	$(CC) $(CFLAGS) update.c -o update

clean:
	rm -f CPIO cpio_out filelist init_disk psplit kmount kumount \
	basecheck update

install: all 
	install -f $(ISSDIR) CPIO
	install -f $(ISSDIR) cpio_out
	install -f $(ISSDIR) filelist
	install -f $(ISSDIR) init_disk
	install -f $(ISSDIR) psplit
	install -f $(ISSDIR) basecheck
	install -f $(ISSDIR) update
	install -f $(ADMDIR) kmount
	rm $(ADMDIR)/kumount
	ln $(ADMDIR)/kmount $(ADMDIR)/kumount
	cp BASE COMM COMMawk1 awkscript issue mkboot1 mkboot1.40 \
	   mkboot2 mkboot2.40 makeflops validBASE $(ISSDIR)
	cp inittab.s mkroom mklost+found $(ADMDIR)
	cp baseexec $(TMPDIR)

new: dir install

dir:
	mkdir $(ISSDIR)
