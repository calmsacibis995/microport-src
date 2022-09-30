/*      Copyright (c) 1985 AT&T */
/*        All Rights Reserved   */

/*      THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T     */
/*      The copyright notice above does not evidence any        */
/*      actual or intended publication of such source code.     */

/*
 * @(#)wn.c	1.25
 *	iSBC 215 Driver for System 5 UNIX. Supports 215/A/B/G.
 *	This code is based on an Intel supplied source.
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
#include <sys/utsname.h>
#include <sys/elog.h>
#include <sys/erec.h>
#include <sys/errno.h>
#include <sys/trap.h>

long	physaddr();
extern	int	N215;
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
extern	struct	iotime	wnstat[];	/* status structure */

/*
 * wnopen
 *	Open a unit.
 */
wnopen(dev, flag)
dev_t	dev;
int	flag;
{
	register struct i215dev *dd;
	register unsigned	board;
	unsigned unit;
	static char firsttime = 1;
	static char openwait = 0;

	/*
	 * see if someone else is opening a disk
	 */
	openwait++;
	while ( openwait != 1 ) {
		sleep( (caddr_t)&openwait, PRIBIO+1 );
	}

	/*
	 * if this is the first open ever then initilize the world
	 */
	if( firsttime ) {
		firsttime--;
		for( board = 0; board < N215; board++ ) {
			wninit( board );
		}
	}

	board = BOARD(dev);
	dd = i215bdd[board];

	/*
	 * check range and board present
	 */
	if( board >= N215 || dev > i215maxmin || !dd->d_state.s_exists ) {
		u.u_error = ENXIO;
		return;
	}

	dd->d_state.s_flags[ UNIT( dev ) ] = SF_OPEN|SF_READY;

	/*
	 * set pointer to io stats
	 */
	dd->d_state.s_bufh->io_stp = &wnstat[ board ].ios;

	/*
	 * if anyone is waiting to open wake them up
	 */
	openwait--;
	if ( openwait ) {
		wakeup( (caddr_t)&openwait );
	}
}

/*
 * wnclose
 *	Close a unit, does nothing.
 */
wnclose(dev)
register dev_t	dev;
{
}

int mode = 0;
/*
 * wnstrategy
 *	Queue an I/O Request, and start it if not busy already.
 *	Check legality, and adjust for slice.
 *	Reject request if unit is not-ready.
 */
wnstrategy(bp)
register struct buf *bp;
{
	register struct i215dev	 *dd;
	register struct i215drtab *dr;
	register struct i215slice *p;
	register int	a20bit;
	struct	buf	*ap;
	unsigned	x;
	unsigned	unit;

	dd = i215bdd[ BOARD( bp->b_dev ) ];
	unit = UNIT( bp->b_dev );
	dr = &i215drtab[ DRTAB( bp->b_dev ) ];
	p = &dr->dr_slice[ SLICE( bp->b_dev ) ];

	/*
	 * update stats
	 */
	wnstat[ BOARD( bp->b_dev ) ].io_cnt++;
	wnstat[ BOARD( bp->b_dev ) ].io_bcnt += btoc( bp->b_bcount );
	bp->b_start = lbolt;

	/*
	 * Figure sector number from block number. 
	 * Check if ready, and see if fits in slice.
	 * Adjust sector # for slice.
	 *
	 * Note: if format, b_blkno is already the correct sector number.
	 */
	if( bp->b_flags & B_FORMAT ) {
		bp->b_secno = bp->b_blkno;
	} else {
		bp->b_secno = ( ( bp->b_blkno * LBLOCK ) / dr->dr_secsiz );
	}

	/*
	 * check for not ready, or off the end
	 */
	if (((dd->d_state.s_flags[unit] & SF_READY) == 0) ||
	    (bp->b_secno > p->p_nsec)) {
		bp->b_flags |= B_ERROR;
		bp->b_error = ENXIO;
		iodone(bp);
		return;
	}
	if (bp->b_secno == p->p_nsec) {
		if (bp->b_flags & B_READ)
			bp->b_resid = bp->b_bcount;
		else {
			bp->b_error = ENXIO;
			bp->b_flags |= B_ERROR;
		}
		iodone(bp);
		return;
	}
	if ((bp->b_secno + (bp->b_bcount+dr->dr_secsiz-1)/dr->dr_secsiz)
	    > p->p_nsec) {
		/* just asked to read last one.  Tell him EOF */
		bp->b_resid = bp->b_bcount;
		iodone(bp);
		return;
	}
	bp->b_secno += p->p_fsec;
	bp->b_cylin = bp->b_secno / dr->dr_spc;

	/*
	 * get physical address of buffer
	 */
	a20bit = (int)(dd->d_state.s_devcode[0]) != DEVWINIG;
	bp->b_physaddr = PHYSADDR( bp->b_un.b_addr );

	/*
	 * Add request to queue, and start it.
	 */

	x = SPL();
	ap = dd->d_state.s_bufh->b_actf;

	/*
	 * find the right place to put this buffer into the list
	 * by cylinder number
	 */
	while( ap != (struct buf *)dd->d_state.s_bufh ) {
		if( bp->b_cylin < ap->b_cylin )
			ap = ap->av_forw;
		else
			break;
	}

	/*
	 * now insert buffer into the list
	 */
	bp->av_back = ap->av_back;
	ap->av_back->av_forw = bp;
	bp->av_forw = ap;
	ap->av_back = bp;

	/*
	 * if the controler is not busy then start it
	 */
	if( ! ( dd->d_state.s_active & IO_BUSY ) )
		dd->d_curbuf = bp;
		wnstart(dd);

	splx(x);
}

/*
 * wnintr
 *	Handle interrupt.
 *
 * Interrupt procedure for 215 driver.  Gets status of last operation
 * and performs service function according to the type of interrupt.
 * If it was an operation complete interrupt, switches on the current
 * driver state and either declares the operation done, or starts the
 * next operation and sets dd->d_state.s_state to the next state.
 */
wnintr(level)
int	level;			/* interrupt level */
{
	register struct i215dev *dd;
	register struct buf	*bp, *ap;
	struct iobuf	*bufh;
	unsigned	spindle;
	unsigned	board;
	unsigned	status;
	unsigned	hard;
	unsigned	x;

	/*
	 * mask the interrupt to prevent re-entering, and
	 * ack the interrupt so the next one wont be missed
	 */
	x = SPL();
	eoi( level );

	/*
	 * find the board for this interrupt level
	 */
	for( board = 0; i215cfg[ board ].c_level != level; board++);
	dd = &i215dev[ board ];
	bufh = dd->d_state.s_bufh;
	bp = dd->d_curbuf;

	/*
	 * Clear the interrupt and make sure that the interrupt is real.
	 */
	out((int)(dd->d_state.s_wua)>>4, WAKEUP_CLEAR_INT);
	if ((!dd->d_state.s_exists) || (dd->d_cib.c_statsem == 0)) {
		splx( x );
		return;
	}

	/*
	 * Pick up the controller status.
	 */

	status = dd->d_cib.c_stat;
	dd->d_cib.c_statsem = 0;

	/*
	 * Set spindle to the spindle number of the
	 * interrupting device: 0-3 hard disk/4-7 floppies
	 */

	spindle = (status & ST_UNIT) >> 4;

	/*
	 * set up the spindle number for floppy
	 */

	if (status & ST_FLOPPY) {
		spindle += FIRSTREMOV;			/* normal floppy mode */
	}

	/*
	 * Check for operation complete interrupt or seek-complete (restore).
	 * Note: only interrupt possible on unopened unit is media-change.
	 */

	if (status & (ST_OP_COMPL|ST_SEEK_COMPL)) {
		switch(dd->d_state.s_state) {

		/*
		 * Normal operation complete interrupt.
		 * Check status for error; on error get
		 * status information from controller.
		 * Otherwise declare operation done.
		 */

		case NOTHING:
			if (status & ST_ERROR) {
				if ( status & ST_HARD_ERR )
					dd->d_state.s_active |= IO_HARD_ERR;
				else
					dd->d_state.s_active &= ~IO_HARD_ERR;

				/*
				 * there is an error, save the iopb
				 * for error loging and get error status
				 */
				dd->d_eregs.e_iopb = dd->d_iopb;
				dd->d_state.s_state = GET_BAD_STATUS;
				wnstat[ board ].ios.io_misc++;
				wnio(dd, bp, STATUS_OP, spindle);
				splx( x );
				return;
			}

			bp->b_resid = 0;		/* xfr'd all */
			break;

		/*
		 * Have read status information from controller.
		 * If was hard error, print about it & done.
		 * If there was a soft error, start a seek to track zero.
		 */

		case GET_BAD_STATUS:
			hard = dd->d_eregs.e_error.e_hard;
			bufh->io_addr = (caddr_t)&dd->d_eregs;
			fmtberr( bufh, dd->d_drtab[ UNIT( bp->b_dev ) ]->dr_slice->p_fsec );

			if (hard & HARD_NOT_READY) {
				dd->d_state.s_state = NOTHING;
				wnnotify( dd, minor( bp->b_dev ) );
			} else {
				if ( dd->d_state.s_active & IO_HARD_ERR ) {
					wnhard( bp->b_dev, &dd->d_eregs.e_error );
					dd->d_state.s_state = RESTORING;
					dd->d_state.s_hcyl = bp->b_cylin;
					bp->b_cylin = 0;
					wnstat[ board ].ios.io_misc++;
					wnio(dd, bp, SEEK_OP, spindle);
					splx( x );
					return;
				} else {
					wnsoft( bp->b_dev, &dd->d_eregs.e_error );
					dd->d_state.s_state = NOTHING;
				}
			}
			break;

		/*
		 * Have just completed a seek to track 0.
		 * dd->d_error.e_soft contains soft status word from before.
		 * If haven't exhausted retry-count, then retry;
		 * Else, print diagnostic & terminate.
		 * Note: 2 interrupts come in: 1st ==> seek started, 2nd ==>
		 *	 seek-complete.
		 */

		 case RESTORING:
			if ((status & ST_SEEK_COMPL) == 0) {
				splx( x );
				return;			/* wait for real one */
			}
			dd->d_state.s_state = NOTHING;
			if (++bufh->b_errcnt < I215RETRY) {
				bp->b_cylin = dd->d_state.s_hcyl;
				wnio(dd, bp, IO_OP(bp), spindle);
				splx( x );
				return;
			}

			/*
			 * tried to recover, now give up
			 * and report the error
			 */
			bp->b_error = EIO;
			bp->b_flags |= B_ERROR;
			break;

		/*
		 * step to the next unit and start initializing it.
		 * dont initialize invalid units.
		 * restart io after initializing last unit.
		 */
		 case INITIALIZING:
			dd->d_state.s_init[dd->d_state.s_opunit++] = status;
			if(dd->d_state.s_devcode[dd->d_state.s_opunit]==INVALID)
				dd->d_state.s_opunit += NEXT_REMOVE_UNIT;

			if( dd->d_state.s_opunit < NUMSPINDLE ) {
				wnio(dd, (char *)0, INIT_OP,dd->d_state.s_opunit);
				splx( x );
				return;
			} else {
				/*
				 * all are inited, reset flags
				 * and wake up init'er
				 */
				dd->d_state.s_state = NOTHING;
				dd->d_state.s_active &= ~IO_BUSY;
				wnstart( dd );
				splx( x );
				return;
			}

		}
	}

	/*
	 * Check for media change interrupt and set not ready if so.
	 */
	else if( status & ST_MEDIA_CHANGE ) {
		wnnotify(dd, spindle);
		splx( x );
		return;
	}

	/*
	 * see if there was an error
	 */
	if ( bufh->b_errcnt )
		logberr( bufh, dd->d_state.s_active & IO_HARD_ERR );

	/*
	 * I/O Done.  Complete it & start next.
	 */
	wnstat[ board ].io_resp += lbolt - bp->b_start;
	wnstat[ board ].io_act += lbolt - bufh->io_start;
	bufh->b_active = 0;

	/*
	** get next buffer while checking direction
	*/
	if ( dd->d_state.s_active & IO_BACK ) {
		ap = bp->av_back;
		if ( ap == (struct buf *)dd->d_state.s_bufh ) {
			ap = bp->av_forw;
			dd->d_state.s_active &= ~IO_BACK;
		}
	} else {
		ap = bp->av_forw;
		if ( ap == (struct buf *)dd->d_state.s_bufh ) {
			ap = bp->av_back;
			dd->d_state.s_active |= IO_BACK;
		}
	}
	dd->d_curbuf = ap;

	bp->av_back->av_forw = bp->av_forw;
	bp->av_forw->av_back = bp->av_back;
	dd->d_state.s_active &= ~IO_BUSY;
	iodone(bp);
	wnstart(dd);
	splx( x );
}

/*
 * wnread
 *	"Raw" read.  Use physio().
 */
wnread(dev)
dev_t	dev;
{ 
	register struct i215drtab *dr;
	register struct i215slice *p;

	dr = &i215drtab[ DRTAB( dev ) ];
	p = &dr->dr_slice[SLICE(dev)];
	if ( physck( ( ( p->p_nsec * dr->dr_secsiz ) >> LBSHIFT ), B_READ ) )
		physio(wnstrategy, &i215rbuf[BOARD(dev)], dev, B_READ);
}

/*
 * wnwrite
 *	"Raw" write.  Use physio().
 */
wnwrite(dev)
dev_t	dev;
{
	register struct i215drtab *dr;
	register struct i215slice *p;

	dr = &i215drtab[ DRTAB( dev ) ];
	p = &dr->dr_slice[SLICE(dev)];
	if ( physck( ( ( p->p_nsec * dr->dr_secsiz ) >> LBSHIFT ), B_WRITE ) )
		physio(wnstrategy, &i215rbuf[BOARD(dev)], dev, B_WRITE);
}

/*
 * wnioctl
 *
 * I215_FLOPPY command:
 * Returns the # of 512 byte blocks on the floppy device if called for a floppy
 * device, or zero if called for a winchester device.
 *
 * I215_CHAR command:
 * Returns device characteristics as specified in the corresponding i215drtab
 * structure into the structure pointed at by "cmdarg".  The dr_slice field is
 * not copied to user space.  (This avoids different model problems.)
 *
 * I215_NTRACK command:
 * Returns # of tracks on device into the daddr_t pointed to by "cmdarg".
 *
 * I215_IOC_FMT command:
 * Waits for and uses the "raw" buffer for the board, and the d_format field
 * in i215dev.  Use of the "d_format" field is mutexed by use of the "raw"
 * buffer.
 *
 * The format ioctl allows the caller to specify all necessary data to assign
 * alternate tracks.  This code has only the intelligence to format the track;
 * no testing/etc of the track is done.
 *
 * Assumes access to "raw" device is restricted via file-system to knowledgeable
 * programs/users.
 */
wnioctl(dev, cmd, cmdarg, flag)
dev_t	dev;
int	cmd;
struct	i215ftk *cmdarg;
int	flag;
{
	register struct buf *bp;
	register struct iobuf *bufh;
	register struct i215dev *dd;
	int	 i;
	unsigned board;
	unsigned x,op;
	struct i215drtab *drtabp;
	char *p;
	struct i215ftk	ftk;

	board = BOARD(dev);
	dd = i215bdd[board];
	bp = &i215rbuf[board];
	drtabp = &i215drtab[DRTAB(dev)];

	if (cmd == I215_CHAR) {
		if (copyout(drtabp, cmdarg,
			(char *)&(drtabp->dr_slice) - (char *)drtabp))
			u.u_error = EFAULT;
		return;
	}
	if (cmd == I215_FLOPPY) {
		if(drtabp->dr_nrhead) {
			
			/* We have a floppy.  Calculate the # of 512
				byte blocks. */
			long	t;
			struct i215slice	*slicep;

			slicep = drtabp->dr_slice + SLICE(dev);
			t = slicep->p_nsec * drtabp->dr_secsiz;
			u.u_rval1 = t / 512;
		} else

			/* We don't have a floppy. */
			u.u_rval1 = 0;
		return;
	}
	if (cmd == I215_NTRACK) {
		
		/* Calculate # of tracks on section. */
		struct i215slice	*slicep;	/* ptr to slice info */
		daddr_t			t;		/* # of tracks */

		slicep = drtabp->dr_slice + SLICE(dev);
		t = slicep->p_nsec / drtabp->dr_nsec;
		if (copyout(&t, (daddr_t *)cmdarg, sizeof(t)))
			u.u_error = EFAULT;
		return;
	}
	if (cmd != I215_IOC_FMT) {
		u.u_error = ENXIO;
		return;
	}

	/* Format a track. */
	if(copyin(cmdarg, &ftk, sizeof(struct i215ftk))) {
		u.u_error = EFAULT;
		return;
	}

	/*
	 * Wait for buffer.
	 */

	x = SPL();
	while(bp->b_flags&B_BUSY) {
		bp->b_flags |= B_WANTED;
		sleep((caddr_t)bp, PRIBIO+1);
	}
	bp->b_flags = B_BUSY | B_FORMAT;
	bp->b_dev = dev;
	splx(x);

	/*
	 * Set up blkno so that wnio computes the correct cylinder & head.
	 * Set up "d_format" for the format.  B_FORMAT in the buff-header will
	 * tell everyone this is a format request.
	 */
	bp->b_blkno = ftk.f_track * drtabp->dr_nsec;
	bp->b_bcount = 0;	/* wnstrategy() needs this */

	dd->d_format.f_trtype = ftk.f_type;
	dd->d_format.f_interleave = ftk.f_intl;

	/*
	 * f_pattern is an array, but it is not aligned on a word.
	 */
	p = &dd->d_format.f_pattern0;
	for(i = 0; i < sizeof(cmdarg->f_pat); i++)
		*(p+i) = ftk.f_pat[i];

	/*
	 * Start it, then wait for completion.  iowait() clears
	 * all of b_flags except B_BUSY, and B_FORMAT.
	 */
	wnstrategy(bp);
	iowait(bp);
	bp->b_flags &= ~(B_BUSY|B_FORMAT);
}

wndump()
{
	for ( ; ; ) {
		asm( "	cli" );
		printf( "\nDo you really want to dump memory? " );
		if ( ( getchar() & 0x7f ) == 'y' ) {
			printf( "yes\n" );
			wnd_main();
			printf( "\ndump finished, re-boot now:" );
			asm( "	hlt" );
		}
		printf( "no\n" );
		eoi( INTERRUPT );
		asm( "	iret" );
	}
}

/*
 * generic print routine. called by the kernel.
 */
wnprint( dev, string )
int dev;
char *string;
{
	int unit;

	unit = UNIT( dev );
	printf( "%s on 215 dev %d\n", string, dev );
	printf( "%s unit %d\n", ( unit & 4 ? "floppy" : "winchester" ), unit & 3 );
}
