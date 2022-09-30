/*      Copyright (c) 1985 AT&T */
/*        All Rights Reserved   */

/*      THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T     */
/*      The copyright notice above does not evidence any        */
/*      actual or intended publication of such source code.     */

/*
 * @(#)wn.subs.c	1.10
 *	This module contains subroutines that are called
 *	by the 215 disk driver.
 */

#include <sys/signal.h>
#include <sys/types.h>
#include <sys/sysmacros.h>
#include <sys/param.h>
#include <sys/systm.h>
#include <sys/buf.h>
#include <sys/iobuf.h>
#include <sys/conf.h>
#include <sys/dir.h>
#include <sys/user.h>
#include <sys/wn.h>
#include <sys/errno.h>

long	physaddr();
extern	struct i215cfg	i215cfg[];	/* per-board configuration */
extern	struct i215drtab i215drtab[];	/* per-unit configuration */
extern	struct i215drtab i215drdmy[];	/* dummy per-unit configuration */
extern	struct i215dev	i215dev[];	/* per-board device-data-structures */
extern	struct	iobuf	i215tab[];	/* buffer headers per board */
extern	struct	buf	i215rbuf[];	/* raw buffer headers per board */
extern	int	i215fmaj;		/* for board index computation */
extern	struct	i215dev	*i215bdd[];	/* board-idx -> "dev" map */
extern	unsigned	i215minor[];	/* minor number bit map */
extern	short	i215maxmin;		/* largest minor number posible */

/*
 * wninit
 *	Initilize a board, and the parameter blocks.
 */
wninit(board)
unsigned board;
{
	register struct i215cfg *cf;
	register struct i215dev *dd;
	extern struct i215wub	wub;
	struct iobuf *iobuf;
	int		i;

	cf = &i215cfg[board];	/* part of configuration */
	dd = &i215dev[board];	/* i215dev was zapped during load */

	/*
	 * Set up wake-up block for the board.
	 */
	wub.w_sysop = 1;
	wub.w_rsvd = 0;
	wub.w_ccb = ADDR86( physaddr( &dd->d_ccb ) );

	/*
	 * Reset the board
	 */

	out((int)(cf->c_wua)>>4, WAKEUP_RESET);
	out((int)(cf->c_wua)>>4, WAKEUP_CLEAR_INT);

	/*
	 * Set up various constant fields.
	 */
	dd->d_state.s_wua = cf->c_wua;		/* local copy */
	dd->d_state.s_level = cf->c_level;	/* local copy */
	dd->d_state.s_bufh = &i215tab[ board ];	/* back-pointer to hdr */
	dd->d_state.s_active = IO_IDLE;		/* active flag */

	/*
	 * initialize buffer header
	 */
	iobuf = &i215tab[ board ];
	iobuf->b_forw = (struct buf *)iobuf;
	iobuf->b_back = (struct buf *)iobuf;
	iobuf->b_actf = (struct buf *)iobuf;
	iobuf->b_actl = (struct buf *)iobuf;
	iobuf->io_nreg = sizeof ( struct i215eregs );
	iobuf->io_addr = (caddr_t)i215cfg[ board ].c_wua;

	/*
	 * set board to 'dev' map
	 */
	i215bdd[board] = dd;

	/*
	 * fill in the CCB
	 */
	dd->d_ccb.c_ccw1 = 1;
	dd->d_ccb.c_busy1 = 0xFF;
	dd->d_ccb.c_cib = ADDR86( physaddr( &dd->d_cib.c_csa[0] ) );
	dd->d_ccb.c_rsvd0 = 0;
	dd->d_ccb.c_busy2 = 0;
	dd->d_ccb.c_ccw2 = 1;
	dd->d_ccb.c_cpp = ADDR86( physaddr( &dd->d_ccb.c_cp ) );
	dd->d_ccb.c_cp = 4;

	/*
	 * fill in the CIB
	 */
	dd->d_cib.c_cmd = 0;
	dd->d_cib.c_stat = 0;
	dd->d_cib.c_cmdsem = 0;
	dd->d_cib.c_statsem = 0;
	dd->d_cib.c_csa[0] = 0;
	dd->d_cib.c_csa[1] = 0;
	dd->d_cib.c_iopb = ADDR86( physaddr( &dd->d_iopb ) );
	dd->d_cib.c_rsvd1[0] = 0;
	dd->d_cib.c_rsvd1[1] = 0;

	/*
	 * Try to init board.  If there, set "exists" in state.
	 */

	out((int)(dd->d_state.s_wua)>>4, WAKEUP_START);
	delay(100);			/* give it time to happen */
	dd->d_state.s_exists = ( ( dd->d_ccb.c_busy1 == 0 ) ? 1 : 0 );

	/*
	 * init to not open state
	 */
	dd->d_state.s_active = IO_IDLE;
	for( i = 0; i < NUMSPINDLE; i++ ) {
		dd->d_state.s_flags[ i ] = 0;
		dd->d_drtab[ i ] = i215drdmy;
	}

	/*
	 * Set up "unit" and "device-code" value (per unit), for iopb
	 * programming.  Set up here to avoid calculating on EACH I/O.
	 */
	for(i = 0; i < FIRSTREMOV; i++) {
		dd->d_state.s_devcode[i] = cf->c_devcod[0];
		dd->d_state.s_unit[i] = i;
	}
	for(; i < NUMSPINDLE; i++) {
		dd->d_state.s_devcode[i] = cf->c_devcod[i/FIRSTREMOV];
		dd->d_state.s_unit[i] = (i & FIXEDMASK) | UNIT_REMOVABLE;
	}
}

/*
 * wnstart
 *	Start an I/O request.
 */
wnstart(dd)
register struct i215dev *dd;
{
	register struct iobuf *bufh;
	register struct buf *bp;
	register unit;
	register dev;
	register unsigned x;

	bufh = dd->d_state.s_bufh;

	/*
	 * if busy or nothing to do then quit
	 */
	if( dd->d_state.s_active & IO_BUSY ||
	    ( bp = dd->d_curbuf ) == (struct buf *) bufh ) {
		return;
	}

	/*
	 * get dev and unit
	 */
	dev = minor( bp->b_dev );
	unit = UNIT( dev );

	/*
	 * check to see if re-init is needed
	 */
	if( dd->d_drtab[ unit ] != &i215drtab[ DRTAB( dev ) ] ) {
		wnsweep( dd, dev );
		return;
	}

	/*
	 * must be something to do
	 */
	x = SPL();
	dd->d_state.s_active |= IO_BUSY;
	bufh->b_errcnt = 0;
	wnio(dd, bp, IO_OP(bp),UNIT(bp->b_dev));
	splx(x);
}

/*
 * wnnotify
 *	Unit went not-ready.
 *
 * If removable unit & was open, print diagnostic & make it not-ready.
 * If already reported "not ready", ignore.
 *
 * Note: does NOT flush requests for off-line spindle out of I/O
 *	 queue.  Any request started before unit goes ready again will
 *	 get error from controller.  Requests should flush in this manner.
 *	 It is possible for queued requests to be still queued after the
 *	 unit goes ready again (very fast "pop" & re-insert floppy).  If
 *	 this is a problem, then this routine can flush these requests.
 */
wnnotify(dd, dev)
register struct i215dev *dd;
register unsigned dev;
{
	register unit;

	unit = UNIT( dev );
	if ((unit >= FIRSTREMOV) &&
	    ((dd->d_state.s_flags[unit] & (SF_OPEN|SF_READY)) == (SF_OPEN|SF_READY))) {
		wnprint( dev, "not ready" );
		dd->d_state.s_flags[unit] &= ~SF_READY;
	}
}

/*
 * format and print soft error status
 */
wnsoft(dev, error)
struct i215err *error;
{
	int unit;

	unit = UNIT( dev );
	printf( "soft error on 215 dev %d, hard status = 0x%x, soft status = 0x%x\n",
		minor(dev), error->e_hard, (uint)error->e_soft );

	printf( "%s unit %d\n", ( unit & 4 ? "floppy" : "winchester" ), unit & 3 );
}

/*
 * format and print hard error status
 */
wnhard(dev, error)
struct i215err *error;
{
	int unit;

	unit = UNIT( dev );
	printf( "hard error on 215 dev %d, hard status = 0x%x, soft status = 0x%x\n",
		minor(dev), error->e_hard, (uint)error->e_soft );

	printf( "%s unit %d\n", ( unit & 4 ? "floppy" : "winchester" ), unit & 3 );
}

/*
 * wnsweep
 *	Preform an init-sweep.
 *
 * The 215 controller runs a diagnostic and clears its RAM whenever unit 0
 * is initialized.  When floppy units are initialized, some of RAM is cleared.
 * Thus, the simplest solution is to do a complete init-sweep on every re-init.
 *
 * Sweep uses current drtab entries for all units.
 */
wnsweep(dd, dev)
register struct i215dev *dd;
dev_t	dev;
{
	unsigned	unit;

	/*
	 * Figure unit & drtab entry.
	 */

	unit = UNIT(dev);
	dd->d_drtab[unit] = &i215drtab[DRTAB(dev)];

	/*
	 * Do the init-sweep.
	 */
	dd->d_state.s_state = INITIALIZING;
	dd->d_state.s_active |= IO_BUSY;
	wnio( dd, (char *)0, INIT_OP, 0 );
}

/*
 * wnio
 *	Mechanics of starting an IO.  Parameters are already stored in "dd".
 *	Starts the 215 by setting up the iopb and sending
 *	a wakeup signal to the controller.
 */
wnio(dd, bp, op, unit)
register struct i215dev *dd;
register struct buf	*bp;
int	op;
int unit;
{
	register int	a20bit;
	register struct i215slice *p;
	register struct i215drtab *dr;
	register struct i215ldrtab *ldr;
	unsigned x;
	int *secsiz;

	/*
	 * protect ourself
	 */
	x = SPL();

	/*
	 * Set up IOPB.
	 * if using a 215G, turn on 24bit mode
	 * Note: the calls to physaddr are dangerous if
	 *	thay are executed at interrupt level because
	 *	the correct user area may not be maped in.
	 */
	a20bit = (int)(dd->d_state.s_devcode[0]) != DEVWINIG;
	dd->d_iopb.i_modifier = ( a20bit ? 0 : MOD_24_BIT_ADDR );

	switch(op) {
		case STATUS_OP:
			dd->d_iopb.i_addr = PHYSADDR( &dd->d_eregs.e_error );
			break;

		case INIT_OP:
			dr = dd->d_drtab[unit];
			ldr = &dd->d_ldrtab;
			ldr->ldr_ncyl = dr->dr_ncyl;
			ldr->ldr_nfhead = dr->dr_nfhead;
			ldr->ldr_nrhead = dr->dr_nrhead;
			ldr->ldr_nsec = dr->dr_nsec;
			secsiz = (int *) &ldr->ldr_secsiz_l;
			*secsiz = dr->dr_secsiz;
			ldr->ldr_nalt = dr->dr_nalt;
			dd->d_iopb.i_addr = PHYSADDR( ldr );
			break;
			
		case FORMAT_OP:
			dd->d_iopb.i_addr = PHYSADDR( &dd->d_format );
			break;
		default:
			dd->d_iopb.i_xfrcnt = bp->b_bcount;
			dd->d_iopb.i_addr = bp->b_physaddr;
			break;
	}

	dd->d_iopb.i_funct = op;
	dd->d_iopb.i_device = dd->d_state.s_devcode[unit] & DEVMASK;
	dd->d_iopb.i_unit = dd->d_state.s_unit[unit];
	
	if (op != STATUS_OP && op != INIT_OP) {
		uint secno;	/* temporary for sector number */

		dr = dd->d_drtab[unit];
		p = &dr->dr_slice[SLICE(bp->b_dev)];

		dd->d_iopb.i_cylinder = bp->b_cylin;
		secno = bp->b_secno % dr->dr_spc;
		dd->d_iopb.i_head = secno / dr->dr_nsec;
		dd->d_iopb.i_sector = secno % dr->dr_nsec;
		if (dd->d_iopb.i_device == DEV8FLPY || dd->d_iopb.i_device == DEV5FLPY)
			++dd->d_iopb.i_sector;		/* floppy starts at 1 */
	}
	dd->d_state.s_opunit = unit;
	if ( dd->d_state.s_bufh->b_active == 0 ) {
		dd->d_state.s_bufh->io_start = lbolt;
	}
	dd->d_state.s_bufh->b_active++;
	out((int)(dd->d_state.s_wua)>>4, WAKEUP_START);
	splx( x );
}
