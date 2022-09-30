static char *uportid = "@(#)wd.c	Microport Rev Id 1.3.4  6/23/86";

/*
 * @(#)wd.c     1.0
 *      WD1010 Driver for System 5 UNIX on the PCAT
 * Copyright 1985 @ by Microport Systems. All Rights Reserved.
 * Modification History:
 *                      Revision #1 - Author Buried in Obscurity 12/15/85
 *			Revision #2 - Larry weaver 1/30/86
*/
extern  struct i1010cfg  i1010cfg[];      /* per-board configuration */
extern  struct i1010drtab i1010drtab[];   /* per-unit configuration */
extern  struct i1010drtab i1010drdmy[];   /* dummy per-unit configuration */
extern  struct i1010dev  i1010dev[];      /* per-board device-data-structures */
extern  struct  iobuf   i1010tab[];       /* buffer headers per board */
extern  struct  buf     i1010rbuf[];      /* raw buffer headers per board */
extern  int     i1010fmaj;                /* for board index computation */
extern  struct  i1010dev *i1010bdd[];     /* board-idx -> "dev" map */
extern  unsigned        i1010minor[];     /* minor number bit map */
extern  struct bad_tracks i1010bad_track[]; /* bad track map */
extern  short   i1010maxmin;              /* largest minor number posible */
extern unsigned char bblock[];
extern  struct  iotime  wdstat[];         /* status structure */
int instatus;

        static char opencount = 0,firsttime = 1;
/*
 * wdopen
 *      Open a unit.
 * The open routine is called whenever a process wants to access a
 * device. The very first time that open is called, one has to initialize
 * the correct unit. 
 */

wdopen(dev, flag)
dev_t   dev;
int     flag;
{
        static char openwait = 0;
        /*
         * see if someone else is opening a disk
         */
	opencount++;
        openwait++;
        while ( openwait != 1 ) {		/* mask out other opens */
                sleep( (caddr_t)&openwait, PRIBIO+1 );
        }

        /*
         * if this is the first open ever then initilize the world
         * check range and board present
	 * if an error occurs: {
	 *  set u.u_error 
	 *  mark the unit as uninitialized (for later retries)
	 *  decrement openwait (i.e. # of processes waiting for device)
	 *  do a wakeup on anything sleeping on openwait:
	 *		wakeup((caddr_t) &openwait);
	 *  return
	 * }
	 */

	/* If we've gotten this far, then the device has been
	 * initialized, and is ready for business. Set a flag indicating
	 * as such, decrement openwait, and wakeup any processes that
	 * are sleeping on openwait (i.e. are waiting to call flopen).
	 */

}

/*
 * wdclose
 * under system V, the close routine is only called on the *last close*!
 * that is, the reference count is handled in closef(). this differs from
 * various other flavors of unix.
 * 	since wdclose is only called once, when it is called, it is
 * time to shut everything down, and mark appropriate status bits as
 * uninitialized.
 */

wdclose(dev)
register dev_t  dev;
{
	opencount=0; /* only called on last close */
}

int mode = 0;
/*
 * wdstrategy
 * the strategy routine is the interface between high level io and the
 * actual device (this includes raw i/o). It's purpose is to check that the
 * data request (read/write) is valid; i.e., bp->b_blkno fits within
 * the region assigned to the unit. If the request is indeed legal, then
 * the buffer bp is added to the device's iobuf queue. No io is done
 * at this level; it's always done in wdstart(). If there aren't any
 * buffer's currently on the iobuf queue, then wdstart() isn't doing
 * anything, and can be called immediately.
 *
 *      Queue an I/O Request, and start it if not busy already.
 *      Check legality, and adjust for slice.
 *      Reject request if unit is not-ready.
 */
wdstrategy(bp)
register struct buf *bp;
{
	/* set up appropriate pointers to information about this device. */
         *  update stats: io_cnt and io_bcnt. bp->b_start = lbolt;
         */

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
                bp->b_secno = ( ( bp->b_blkno * LBLOCK ) / sectorsize );
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

	/* compute # of sectors required to satisfy byte count */
	bp->b_resid = 0;
 	sectorcount = (bp->b_bcount+(dr->dr_secsiz)-1)/(dr->dr_secsiz);
        while ((bp->b_secno + sectorcount) > p->p_nsec) {
		sectorcount --;
		bp->b_bcount -= dr->dr_secsiz;
		bp->b_resid += dr->dr_secsiz;
        };

	if (sectorcount < 1 )
		{
	printf("Weird EOF- bcount %d secno %d nsec %d\n",bp->b_bcount,bp->b_secno,p->p_nsec); /* shouldnt happen */
		 bp->b_resid = bp->b_bcount;
		 iodone(bp);
		 return;
		};

        bp->b_secno += p->p_fsec;
        bp->b_cylin = bp->b_secno / dr->dr_spc;

        /*
         * get physical address of buffer
         */
        a20bit = (int)(dd->d_state.s_devcode[0]) != DEVWINI;
        bp->b_physaddr = PHYSADDR( bp->b_un.b_addr );           
 
        /*
         * Add request to queue, and start it.
         */
        x = splbio();
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
        
        wdstart(dd);
        splx(x);
}

int	seconderr = 0;

/*
 * wdintr
 *      Handle interrupt.
 *
 * Interrupt procedure for 1010 driver.  Gets status of last operation
 * and performs service function according to the type of interrupt.
 * If it was an operation complete interrupt, switches on the current
 * driver state and either declares the operation done, or starts the
 * next operation and sets dd->d_state.s_state to the next state.
 */
wdintr(level)
int     level;                  /* interrupt level */
{
        /*
         * mask the interrupt to prevent re-entering, and
         * ack the interrupt so the next one won't be missed
         * read status port once before the eoi
         */
        x = splbio();
        
        /*
         * Pick up the controller status.
	 * read error reg, then stat reg, THEN ack int
         */

		eoi( level );

        /*
         * Make sure that the interrupt is real.
         */
	if (irq isn't real)
		return;

        /*
         * determine device # of hard disk 
         */
        device = ...;

        /*
         * Check for operation complete interrupt or seek-complete (restore).
         * Note: only interrupt possible on unopened unit is media-change.
         */
	if (operation is complete or seek has completed) {
                switch(current device state) {

                /*
                 * Normal operation complete interrupt.
                 * Check status for error; on error get
                 * status information from controller.
                 * Otherwise declare operation done.
                 */

                case operation_complete_irq:
			if (status is bad) {
				print disk I/O Error message;
				bp->b_error = EIO;
				bp->b_flags |= B_ERROR;
				reset board and try again;
			};
			if (we just finished a seek command) {
				issue a read/write command;
                        	splx( x );
                       		return;

                	} else {
		/*  A read or write operation generated an interrupt. 
		 * We may be done or in the middle of a multi sector operation.
		 * See if there's more to do, 
		 */
			if (no more data to be transferred) {
				finish io request;
                         } else {
				set up appropriate fields to drive next cmd;
				issue command;
			}

			splx(x);
			return;
        	 }
                 case INITIALIZING:
                /*
                 * step to the next unit and start initializing it.
                 * don't initialize invalid units.
                 * restart io after initializing last unit.
                 */
			if (an error has occured) {
				printf("Error in state initializing\n");
				try reinitializing the unit;
                                splx( x );
                                return;
                         } else {
                                 /*
                                  * all are inited, reset flags
                                  * and wake up init'er
                                  */
                                 wdstart( dd );
                                 splx( x );
                                 return;
                         }
                }
        }

        /*
         * see if there was an error
         */

        if ( bufh->b_errcnt )
                log the error;

        /*
         * I/O Done.  Complete it & start next.
         * We should read the buffer now
         * only in the case of read_op && no error status
         */
        
        wdstat[ board ].io_resp += lbolt - bp->b_start;
        wdstat[ board ].io_act += lbolt - bufh->io_start;

        /*
        ** get next buffer while checking direction
        */
	set up next buffer for i/o;

	/* mark current buffer as having completed io */
        iodone(bp);
        splx( x );
}

/*
 * wdread
 *      "Raw" read.  Use physio().
 */
wdread(dev)
dev_t   dev;
{ 
        if ( physck( ( ( #_of_sectors * sector_size) >> LBSHIFT ), B_READ ) )
                physio(wdstrategy, &i1010rbuf[BOARD(dev)], dev, B_READ);
}

/*
 * wdwrite
 *      "Raw" write.  Use physio().
 */
wdwrite(dev)
dev_t   dev;
{
        if ( physck( ( ( #_of_sectors * sector_size) >> LBSHIFT ), B_WRITE ) )
                physio(wdstrategy, &i1010rbuf[BOARD(dev)], dev, B_WRITE);
}

/*
 * wdioctl
 *
 * I1010_CHAR command:
 * Returns device characteristics as specified in the corresponding i1010drtab
 * structure into the structure pointed at by "cmdarg".  The dr_slice field is
 * not copied to user space.  (This avoids different model problems.)
 *
 * I1010_NTRACK command:
 * Returns # of tracks on device into the daddr_t pointed to by "cmdarg".
 *
 * I1010_IOC_FMT command:
 * Waits for and uses the "raw" buffer for the board, and the d_format field
 * in i1010dev.  Use of the "d_format" field is mutexed by use of the "raw"
 * buffer.
 *
 * The format ioctl allows the caller to specify all necessary data to assign
 * alternate tracks.  This code has only the intelligence to format the track;
 * no testing/etc of the track is done.
 *
 * I1010_RAWIO command:
 * ARG points to IOPB initialized with function(read or write),unit,
 * actual cylinder, actual head, sector and buffer. Reads or writes
 * entire sector. Buffer pointer must be 4-byte pointer.
 *
 * i100_SETDB sets the debug control variable to the value of *cmdarg
 * to enable or disable debug printouts.
 *
 * i1010_REINIT resets the firsttime switch so a subsequent wdopen
 * call will reload the minor device, slice, drive, and bad-track
 * tables from the disk. This is used in hard-disk installation
 * to avoid reboots.
 *
 * Assumes access to "raw" device is restricted via file-system
 * to knowledgeable programs/users.
 */
wdioctl(dev, cmd, cmdarg, flag)
dev_t   dev;
int     cmd;
struct  i1010ftk *cmdarg;
int     *flag;
{
	if (cmd == I1010_SETDB)			/* turn on debugging */
	{					/* can now be done via patch*/
		if (cmdarg)	/* zero is CTEST */
			lwdb = *((int *)cmdarg);
		return;
	};

	if (cmd == I1010_REINIT)
	{
		if (opencount <= 1)
			firsttime = 1;
		else
		{
	printf("opencount: %d \n,opencount");
			u.u_error = EBUSY;
		};
		return;
	};

       if (cmd == I1010_RAWIO)
       {
		set up various device status fields;
		if (we can't do polled io)
			return an error code;

		set up dma for read or write operation;
		if (bad winchester status) {
			u.u_error = EFAULT;
		}
		restore appropriate status fields;
		return;
       	}
  
       if (cmd == I1010_CHAR) {
                if (copyout(drtabp, cmdarg,
                        (char *)&(drtabp->dr_slice) - (char *)drtabp))
                        u.u_error = EFAULT;
                return;
        }

        if (cmd == I1010_NTRACK) {
                
                /* Calculate # of tracks on section. */
                return;
        }
        if (cmd != I1010_IOC_FMT) {
                u.u_error = ENXIO;
                return;
        }

        /* Format a track. */

         if(copyin(cmdarg, &ftk, sizeof(struct i1010ftk))) {
                 u.u_error = EFAULT;
                 return;
         }

        /*
         * Wait for buffer.
         */

        x = splbio();
        while(bp->b_flags&B_BUSY) {
                bp->b_flags |= B_WANTED;
                sleep((caddr_t)bp, PRIBIO+1);
        }
	set up appropriate flags and states bits;
	fill dma buffer with format data;
	issue command;
	return;
}

/*
	Dummy because its called from kernel
 */
wddump()
{

}

/*
 * generic print routine. called by the kernel.
 */
wdprint( dev, string )
int dev;
char *string;
{
}
