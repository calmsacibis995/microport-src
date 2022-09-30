static char *uportid = "@(#)hd.dump.c	Microport Rev Id 2.2.  4/20/87";
/*
** @(#)hd.dump.c	1.2
** routines for crash dump
 *
 * Modification history:
 *
 *	Mon Apr 20 1987
 *		Changed names from wn to hd
 */

#include <sys/param.h>
#include <sys/sysmacros.h>
#include <sys/types.h>
#include <sys/hd.h>

#define IOADR1010 0x100		/* io port address */
#define NTRK 80			/* number of tracks on a floppy */
#define DUMPSIZE ( 18 * 512 )	/* how much to dump at a time */

extern struct i1010wub wub;

struct i1010drtab hdd_drtab[] = {
   0,	0,	0,	0,	0,	0,	0,	0,
   0,	0,	0,	0,	0,	0,	0,	(struct i1010slice *)0,
   0,	0,	0,	0,	0,	0,	0,	0,
   0,	0,	0,	0,	0,	0,	0,	(struct i1010slice *)0,
   0,	0,	0,	0,	0,	0,	0,	0,
   0,	0,	0,	0,	0,	0,	0,	(struct i1010slice *)0,
   0,	0,	0,	0,	0,	0,	0,	0,
   0,	0,	0,	0,	0,	0,	0,	(struct i1010slice *)0,
  40,	0,	2,	9,	512,   FLPY_MFM,9,	0,
   0,	0,	0,	0,	0,	0,	0,	(struct i1010slice *)0,
   0,	0,	0,	0,	0,	0,	0,	0,
   0,	0,	0,	0,	0,	0,	0,	(struct i1010slice *)0,
   0,	0,	0,	0,	0,	0,	0,	0,
   0,	0,	0,	0,	0,	0,	0,	(struct i1010slice *)0,
   0,	0,	0,	0,	0,	0,	0,	0,
   0,	0,	0,	0,	0,	0,	0,	(struct i1010slice *)0,
};

struct {
	struct i1010iopb hdd_iopb;
} hdd_dev;

hdd_main()
{
	int trk;
	long addr, length, memsize;
	char c;
	extern int physmem;

	hdd_init();

	addr = 0;
	trk = 0;
	memsize = (long)physmem * ctob( 1 );

	hdd_write( addr, memsize );
}
hdd_init() {
	int unit;
	struct i1010drtab ldtab;
	extern long physaddr();

	/*
	** Reset the board
	*/
	out(WD0_REG_PORT,WD_HDW_RESET);
	out(WD0_REG_PORT,WD_HDW_INIT);

	/*
	** Try to init board.
	*/
	/* try to run controller diagnostic
	 * report any errors
	 * then go on and try to use the darn thing anyway
	 *
	 *		out( IOADR1010, WAKEUP_START );
	 *		while ( hdd_dev.hdd_ccb.c_busy1 );wait for controler 
 	 *		hdd_dev.hdd_cib.c_statsem = 0;
	 *
	 */

	/*
	* now initialize all the devices
	* wini first
	* mask the interrupt in the slave pic
	* do a set parms on both drives
	* then do a recal_op on both drives
	* while polling for (not (busy) & drive_ready & seek_complete)
	* unmask the interrupt in the slave pic
	*/
	hdd_dev.hdd_iopb.i_device = DEVWINI;
	for ( unit = 0; unit < 4; unit++ ) {
		hdd_dev.hdd_iopb.i_unit = unit;
		hdd_dev.hdd_iopb.i_addr = ADDR86( physaddr( &ldtab ) );
	}
	/*
	 *
	 * do a nec 765 specify, recal, and sense ready status on both
	 * floppy drives on the primary fdc/wd1010 board
	 *
	 */
}

hdd_format( track )
/*
 * previously used to format tracks on a floppy so that
 * dump data could be written on floppies
 * when the floppy controller stuff is done
 * the code will be reinstated in its new (nec765) form
 * the winchester and the floppies are no longer a shared controller
 *
 */
{
}

hdd_write( address, count )
/*
 * see format above
 * these will be sequential track writes to the 765
 *
 */
long address,count;
{
	unsigned char	sector_number=0x01;
	unsigned char	dump_track=0x00;
	unsigned int	dump_cylinder=481;
	while(count > 0) {
		outb((WD0_BASE)+2,0x01);
		outb((WD0_BASE)+3,sector_number);
		outb((WD0_BASE)+4,lobyte(dump_cylinder));
		outb((WD0_BASE)+5,hibyte(dump_cylinder));
		outb((WD0_BASE)+6,dump_track);
		outb((WD0_BASE)+7,WD_READ_OP);
	
		while(!(inb(WD0_STAT) & WD_ST_DRQ)) {
			;
		}
		hdwdma(address);
		while(inb(WD0_STAT) & WD_ST_BSY) {
			;
		}
		if(!(inb(WD0_STAT)) & WD_ST_ERROR) {
			++sector_number;
			if(sector_number > 17) {
				sector_number = 0x01;
				++dump_track;
				if(dump_track > 0x07) {
					dump_track = 0x00;
					++dump_cylinder;
				}
			}
			count-=0x0200;
			address+=0x0200;
		}
	}
	printf("End of memory dump\n");
}
