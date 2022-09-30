static char *uportid = "@(#)wd.subs.c	Microport Rev Id 1.3.6  7/7/86";

/*
 * @(#)wd.subs.c        1.0
 * Copyright 1985 @ by Microport Systems. All Rights Reserved.
 *      This module contains subroutines that are called
 *      by the WD1010 disk driver.
 */
extern int instatus;	/* interrupt status for polled seeks */
extern  struct i1010cfg  i1010cfg[];      /* per-board configuration */
extern  struct i1010drtab i1010drtab[];   /* per-unit configuration */
extern  struct i1010drtab i1010drdmy[];   /* per-unit configuration */
extern  struct i1010drtab typer[];   /* drive type configuration table */
extern  struct i1010dev  i1010dev[];      /* per-board device-data-structures */
extern  struct  iobuf   i1010tab[];      /* buffer headers per board */
extern  struct  buf     i1010rbuf[];     /* raw buffer headers per board */
extern  int     i1010fmaj;               /* for board index computation */
extern  struct  i1010dev *i1010bdd[];     /* board-idx -> "dev" map */
extern  unsigned i1010minor[];          /* minor number bit map */
extern  unsigned char bblock[], partitab[];
extern  struct  i1010slice slice1[];
extern  struct  bad_track_map;
extern  struct  bad_track_map this_track[];    /* bad track map */
extern  struct  bad_tracks i1010bad_track[];    /* bad track map */
extern  short   i1010maxmin;             /* largest minor number posible */
static int initial = 1 ; /*set to zero at end of wdinit */

/*
 * wdinit
 *      Initialize a board, and the parameter blocks.
 */
wdinit(dev,board)
dev_t dev;
unsigned board;
{
}

/*
 * wdstart
 *      Start an I/O request.
 *   Return 1 if nothing to do
 */
wdstart(dd)
register struct i1010dev *dd;
{

        /*
         * if busy or nothing to do then quit
         */
        if( dd->d_state.s_active & IO_BUSY ||
            ( bp = dd->d_curbuf ) == (struct buf *) bufh ) {
                return 1;
        }

        /*
         * get dev and unit
         */
        dev = minor( bp->b_dev );
        unit = UNIT( dev );

        /*
         * check to see if re-init is needed
         */
	if (we need to reinitialize the board) {
		reinitialize the board;
		return 0;
        }

        /*
         * must be something to do
         */
        x = splbio();				/* mask out irq's */
	if (we're currently servicing a request)
                return 1;
	initialize state fields to indicate the beggining of a request;
        wdio(dd, bp, IO_OP(bp),UNIT(bp->b_dev));
        splx(x);
	return 0;
}

/*
 * wdnotify
 *      Unit went not-ready.
 *
 * If removable unit & was open, print diagnostic & make it not-ready.
 * If already reported "not ready", ignore.
 *
 * Note: does NOT flush requests for off-line spindle out of I/O
 *       queue.  Any request started before unit goes ready again will
 *       get error from controller.  Requests should flush in this manner.
 *       It is possible for queued requests to be still queued after the
 *       unit goes ready again (very fast "pop" & re-insert floppy).  If
 *       this is a problem, then this routine can flush these requests.
 */
wdnotify(dd, dev)
register struct i1010dev *dd;
register unsigned dev;
{
        register unit;

        unit = UNIT( dev );
}

/*
 * format and print soft error status
 */
wdsoft(dev, error)
struct i1010err *error;
{
        int unit;

        unit = UNIT( dev );
        printf( "soft error on WD1010 dev %d, hard status = 0x%x, soft status = 0x%x\n",
                minor(dev), error->e_hard, (uint)error->e_soft );

        printf( "%s unit %d\n", ( unit & 4 ? "floppy" : "winchester" ), unit & 3 );
}

/*
 * format and print hard error status
 */
wdhard(dev, error)
struct i1010err *error;
{
        int unit;

        unit = UNIT( dev );
        printf( "hard error on WD1010 dev %d, hard status = 0x%x, soft status = 0x%x\n",
                minor(dev), error->e_hard, (uint)error->e_soft );

        printf( "%s unit %d\n", ( unit & 4 ? "floppy" : "winchester" ), unit & 3 );
}

/*
 * wdsweep
 *      Preform an init-sweep.
 *
 * The WD1010 controller runs a diagnostic and clears its RAM whenever unit 0
 * is initialized.  
 * The simplest solution is to do a complete init-sweep on every re-init.
 *
 * Sweep uses current drtab entries for all units.
 */
wdsweep(dd, dev)
register struct i1010dev *dd;
dev_t   dev;
{
        unsigned        unit;

        /*
         * Figure unit & drtab entry.
         */

        /*
         * Do the init-sweep.
         */
        wdio( dd, (char *)0, INIT_OP, 0 );
}

/*
 * wdio
 *      Mechanics of starting an IO.  Parameters are already stored in "dd".
 *      Starts the WD1010 by setting up the iopb and sending
 *      a wakeup signal to the controller.
 */
wdio(dd, bp, op, unit)
register struct i1010dev *dd;
register struct buf     *bp;
int     op;
int unit;
{

        /*
         * protect ourself
         */
        x = splbio();

        /*
         * Set up IOPB.
         * if using a 215G, turn on 24bit mode
         * Note: the calls to physaddr are dangerous if
         *      thay are executed at interrupt level because
         *      the correct user area may not be maped in.
         */
	set up status fields the io operation;

	if (controller is busy)
          printf ("WN Controller Hung Busy. STATUS: %x \n",inb(STATUS)) ;
	 splx(x);
	  return;
        };

        switch(op) {
                case STATUS_OP:
			set up for issueing the get status command;
                        break;

                case INIT_OP:
			set up for issueing the initialization command;
                        break;
                        
                case FORMAT_OP:
			issue format command;
                        break;

                case WRITE_OP:
			operation = issue write command;
                        break;

                case READ_OP:
			operation = issue read command;
                        break;
        } /* end of switch (op) */

       
	if (this isn't a status operation or an initialization command) { 
        if (op != STATUS_OP && op != INIT_OP) {
		/* compute # of sectors to be transferred */
        }
        if ( dd->d_state.s_bufh->b_active == 0 ) {
                dd->d_state.s_bufh->io_start = lbolt;
        }
	wdcmd(dd);	/* start operation */

	if (this is a read or write operation) {
		set up status bits for a seek substate;
		issue seek command;
        } else {
		issue the actual operation command;
        }

	splx(x);
}
/* here is where we will fill in the command table */
wdcmd(dd)
register struct i1010dev *dd;

{
	unsigned int secno;
        register struct i1010drtab *dr;

	if (# of sectors to be transferred > # of sectors per track) {
	{
		note the difference, for later use;
		set the # to be transferred = # of sectors per track;
	}

	if (this is a set parameters operation) {
		map bad tracks out;
		issue appropriate parameters command;
        }
	issue the disk command, for right sector and cylinder;
}

/***********************************************************************/
/* do a polled read on the device */
do_wcommand(dd)
register struct i1010dev *dd;
{
	int i;

	set status bits;
	i = polled_io(dd);
	return (i);
}
/***********************************************************************/
/* do a polled read or write of one sector */


polled_io(dd)
register struct i1010dev *dd;
{
int i, x;
long timer;
struct i1010drtab *dr;

	set timeout value
        while(controller is still busy) {
          	if (timed out)
		{
			printf("BUSY TIMEDOUT\007\n");
			printf("STATUS: %x\n",inb(STATUS));
			reset(dd);
			return(1)      ;
		};
        }
polled_read:
	issue read/write command;

	set timeout value;
        while(the data request line hasn't changed) {
		x = spl0();			/* pick up irq's */
          	if (we've timed out awaiting data req)
		{
			printf("TIMEDOUT awaiting DRQ\007\n");
			printf("STATUS: %x\n",inb(STATUS));
			reset(dd);
			return(1)      ;
		};
		splx(x);				/* M000 */
            if (controller's not busy and there's been an error)
		{
			reset(dd);
			return (1)   ;
		};
        };
        
	return (0);
}

}
reset(dd)
register struct i1010dev *dd;
{
	issue the reset command;
}
