/*      Copyright (c) 1985 AT&T */
/*        All Rights Reserved   */

/*      THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T     */
/*      The copyright notice above does not evidence any        */
/*      actual or intended publication of such source code.     */

/*
** @(#)wn.dump.c	1.4
** routines for crash dump
*/

#include <sys/param.h>
#include <sys/sysmacros.h>
#include <sys/types.h>
#include <sys/wn.h>

#define IOADR215 0x100		/* io port address */
#define NTRK 80			/* number of tracks on a floppy */
#define DUMPSIZE ( 18 * 512 )	/* how much to dump at a time */

extern struct i215wub wub;

struct i215drtab wnd_drtab[] = {
   0,	0,	0,	0,	0,	0,	0,	(struct i215slice *)0,
   0,	0,	0,	0,	0,	0,	0,	(struct i215slice *)0,
   0,	0,	0,	0,	0,	0,	0,	(struct i215slice *)0,
   0,	0,	0,	0,	0,	0,	0,	(struct i215slice *)0,
  40,	0,	2,	9,	512,   FLPY_MFM,9,	(struct i215slice *)0,
   0,	0,	0,	0,	0,	0,	0,	(struct i215slice *)0,
   0,	0,	0,	0,	0,	0,	0,	(struct i215slice *)0,
   0,	0,	0,	0,	0,	0,	0,	(struct i215slice *)0,
};

struct {
	struct i215ccb wnd_ccb;
	struct i215cib wnd_cib;
	struct i215iopb wnd_iopb;
} wnd_dev;

wnd_main()
{
	int trk;
	long addr, length, memsize;
	char c;
	extern int physmem;

	wnd_init();

	addr = 0;
	trk = 0;
	memsize = (long)physmem * ctob( 1 );

	for ( ; ; ) {
		if ( ! ( trk % NTRK ) ) {
			printf( "\nInsert floppy # %d and press return:",
			    ( trk / NTRK ) + 1 );
			while ( ( c = ( getchar() & 0x7f ) ) != '\r' )
				putchar( c );
			printf( "\n" );
		}
		wnd_format( trk % NTRK );
		wnd_format( ( trk + 1 ) % NTRK );
		length = memsize - addr;
		if ( length > DUMPSIZE )
			length = DUMPSIZE;
		wnd_write( addr, trk % NTRK, length );
		putchar( '.' );
		trk += 2;
		addr += length;
		if ( addr == memsize )
			break;
	}
}

wnd_init()
{
	int unit;
	struct i215ldrtab ldtab;
	extern long physaddr();

	/*
	** Set up wake-up block for the board.
	*/
	wub.w_sysop = 1;
	wub.w_rsvd = 0;
	wub.w_ccb = ADDR86( physaddr( &wnd_dev.wnd_ccb ) );

	/*
	** Reset the board
	*/
	out( IOADR215, WAKEUP_RESET );
	out( IOADR215, WAKEUP_CLEAR_INT );

	/*
	** fill in the CCB
	*/
	wnd_dev.wnd_ccb.c_ccw1 = 1;
	wnd_dev.wnd_ccb.c_busy1 = 0xFF;
	wnd_dev.wnd_ccb.c_cib = ADDR86( physaddr( &wnd_dev.wnd_cib.c_csa[0] ) );
	wnd_dev.wnd_ccb.c_rsvd0 = 0;
	wnd_dev.wnd_ccb.c_busy2 = 0;
	wnd_dev.wnd_ccb.c_ccw2 = 1;
	wnd_dev.wnd_ccb.c_cpp = ADDR86( physaddr( &wnd_dev.wnd_ccb.c_cp ) );
	wnd_dev.wnd_ccb.c_cp = 4;

	/*
	** fill in the CIB
	*/
	wnd_dev.wnd_cib.c_cmd = 0;
	wnd_dev.wnd_cib.c_stat = 0;
	wnd_dev.wnd_cib.c_cmdsem = 0;
	wnd_dev.wnd_cib.c_statsem = 0;
	wnd_dev.wnd_cib.c_csa[0] = 0;
	wnd_dev.wnd_cib.c_csa[1] = 0;
	wnd_dev.wnd_cib.c_iopb = ADDR86( physaddr( &wnd_dev.wnd_iopb ) );
	wnd_dev.wnd_cib.c_rsvd1[0] = 0;
	wnd_dev.wnd_cib.c_rsvd1[1] = 0;

	/*
	** Try to init board.
	*/
	out( IOADR215, WAKEUP_START );
	while ( wnd_dev.wnd_ccb.c_busy1 );	/* wait for controler */
	wnd_dev.wnd_cib.c_statsem = 0;

	/*
	** now initialize all the devices
	** wini first
	*/
	wnd_dev.wnd_iopb.i_funct = INIT_OP;
	wnd_dev.wnd_iopb.i_modifier = MOD_NO_INT;
	wnd_dev.wnd_iopb.i_device = DEVWINI;
	for ( unit = 0; unit < 4; unit++ ) {
		ldtab.ldr_ncyl = wnd_drtab[ unit ].dr_ncyl;
		ldtab.ldr_nfhead = wnd_drtab[ unit ].dr_nfhead;
		ldtab.ldr_nrhead = wnd_drtab[ unit ].dr_nrhead;
		ldtab.ldr_nsec = wnd_drtab[ unit ].dr_nsec;
		*(int *)&ldtab.ldr_secsiz_l = wnd_drtab[ unit ].dr_secsiz;
		ldtab.ldr_nalt = wnd_drtab[ unit ].dr_nalt;
		wnd_dev.wnd_iopb.i_unit = unit;
		wnd_dev.wnd_iopb.i_addr = ADDR86( physaddr( &ldtab ) );
		wnd_dev.wnd_ccb.c_busy1 = 0xFF;
		out( IOADR215, WAKEUP_START );
		while ( wnd_dev.wnd_ccb.c_busy1 )
			/* nothing */;
		wnd_dev.wnd_cib.c_statsem = 0;
	}

	/*
	** now floppy
	*/
	wnd_dev.wnd_iopb.i_device = DEV5FLPY;
	for ( unit = 0; unit < 4; unit++ ) {
		ldtab.ldr_ncyl = wnd_drtab[ unit + 4 ].dr_ncyl;
		ldtab.ldr_nfhead = wnd_drtab[ unit + 4 ].dr_nfhead;
		ldtab.ldr_nrhead = wnd_drtab[ unit + 4 ].dr_nrhead;
		ldtab.ldr_nsec = wnd_drtab[ unit + 4 ].dr_nsec;
		*(int *)&ldtab.ldr_secsiz_l = wnd_drtab[ unit + 4 ].dr_secsiz;
		ldtab.ldr_nalt = wnd_drtab[ unit + 4 ].dr_nalt;
		wnd_dev.wnd_iopb.i_unit = unit;
		wnd_dev.wnd_iopb.i_addr = ADDR86( physaddr( &ldtab ) );
		wnd_dev.wnd_ccb.c_busy1 = 0xFF;
		out( IOADR215, WAKEUP_START );
		while ( wnd_dev.wnd_ccb.c_busy1 )
			/* nothing */;
		wnd_dev.wnd_cib.c_statsem = 0;
	}
}

wnd_format( track )
{
	struct i215format format;

	wnd_dev.wnd_iopb.i_device = DEV5FLPY;
	wnd_dev.wnd_iopb.i_funct = FORMAT_OP;
	wnd_dev.wnd_iopb.i_unit = 0;
	wnd_dev.wnd_iopb.i_modifier = MOD_NO_INT;
	wnd_dev.wnd_iopb.i_cylinder = track / 2;
	wnd_dev.wnd_iopb.i_head = track & 1;
	wnd_dev.wnd_iopb.i_sector = 1;
	wnd_dev.wnd_iopb.i_addr = ADDR86( physaddr( &format ) );

	format.f_trtype = FORMAT_DATA;
	format.f_pattern0 = 'A';
	format.f_pattern1 = 'l';
	format.f_pattern2 = 'a';
	format.f_pattern3 = 'n';
	format.f_interleave = 3;

	wnd_dev.wnd_ccb.c_busy1 = 0xFF;
	out( IOADR215, WAKEUP_START );
	while ( wnd_dev.wnd_ccb.c_busy1 )
		/* nothing */;
	wnd_dev.wnd_cib.c_statsem = 0;
}

wnd_write( address, track, count )
long address;
{
	wnd_dev.wnd_iopb.i_device = DEV5FLPY;
	wnd_dev.wnd_iopb.i_funct = WRITE_OP;
	wnd_dev.wnd_iopb.i_unit = 0;
	wnd_dev.wnd_iopb.i_modifier = MOD_NO_INT;
	wnd_dev.wnd_iopb.i_cylinder = track / 2;
	wnd_dev.wnd_iopb.i_head = track & 1;
	wnd_dev.wnd_iopb.i_sector = 1;
	wnd_dev.wnd_iopb.i_addr = ADDR86( address );
	wnd_dev.wnd_iopb.i_xfrcnt = count;

	wnd_dev.wnd_ccb.c_busy1 = 0xFF;
	out( IOADR215, WAKEUP_START );
	while ( wnd_dev.wnd_ccb.c_busy1 )
		/* nothing */;
	wnd_dev.wnd_cib.c_statsem = 0;
}
