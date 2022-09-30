static char *uportid = "@(#)765.subs.c	Microport Rev Id 1.3.8  11/24/86";
/* Copyright 1985 by ATT, DRI and Microport. All Rights Reserved.
 *
 * Subroutines for the Intel 765 Floppy Disk driver, on the MAD AT.
 * References:
 *      Intel Microprocessor and Peripheral Handbook. Intel, 1983.
 *              Order #210844-001, pps 6-224 to 6-242 (8272A).
 *      IBM Technical Reference Manual for the AT, March 1984
 *              A: Fixed Disk and Diskette Adapter (Appendix)
 *
 * Initial Coding:
 *              uport!dwight    Fri Oct 18 22:19:51 PDT 1985
 * Modification History:
 *	M000:	uport!dwight	Thu Jun 19 16:47:58 PDT 1986
 *		Added 8 spt support
 *	M001:	uport!dwight	Fri Jul 11 10:48:36 PDT 1986
 *		Full track reads and writes.
 *	M002:	uport!dwight 	Sun Nov 23 16:35:46 PST 1986
 *		Added sharing the dma channels in initdma().
 *	M003:	uport!dean	Wed Jan  9 10:03:24 PST 1987
 *		Added 386 ifdefs (MP386)
 *	M004: 	uport!larry    Thu Mar  5 19:53:24 PST 1987
 *		Added fdupfix() to convert to uport minor device #s
 *	M005:	uport!dean	Thu Mar 26 13:00:23 PST 1987
 *		Changed fdinit to fdbinit to not clash with standard
 *		config fdinit (boot time)
 *	M006:	uport!fredo	Wed Apr  1 15:24:45 PST 1987
 *		Changed all references to "rdtrack" to fdrdcmd.
 *	M007:	uport!fredo    Tue Apr  7 21:07:54 PST 1987
 *		Added fsck fix. (see fd1.c M007 for description)
 *	M008:	uport!rex	Wed Jun 24 20:02:33 PDT 1987
 *		Changed the DMA interface to use the routines provided
 *		in "io/8237.c" for DMA channel sharing.
 *		The routine that was "setdma()" is now changed and called
 *		"fdrsetdma()" and uses the new DMA interface routines.
 *	M009:	uport!rex	Thu Jul 14 19:43:02 PDT 1987
 *		Modified fdbinit() for fd096ds15 and fd096ds9 minor
 *		number handling and parameters for the fd096ds9 device.
 *	M010:	uport!rex	Sun Aug 30 19:23:34 PDT 1987
 *		Added Fred's routine for write protect check.
 */

#include "sys/fd.h"
#include "sys/8237.h"
#include <sys/file.h>

extern uint dbglvl;
extern uint fdcurcyl[];

unsigned int dmainitted = 0;				/* M002 */

/*
 * fdquedrain:  drain all outstanding buffers.
 *              (see M007 comments)
 */
fdquedrain(dev)
dev_t dev;
{
	struct cmd cmd;

	cmd.cmd = QDCMD;
	fdcommand(&cmd, dev);
}

#ifdef	WPCHECK
/*
 * fdwrtchk:  check for write protect status -- M010.
 */
fdwrtchk(dev, flag)
dev_t dev;
int  flag;
{
	struct cmd cmd;

	cmd.cmd = SDS;			/* Sense Drive Status */
	cmd.np = 1;
	cmd.ns = 1;
	cmd.p[0] = UNIT(dev);		/* check head 0 | unit */
	DBG1(printf("fdwrtchk: cmd.cmd=%x UNIT=%x flag=%x\n", cmd.cmd, cmd.p[0], flag));
	fdcommand(&cmd, dev);
	DBG1(printf("fdwrtchk: cmd.s[0] = %x flag = %x\n", cmd.s[0], flag));
#ifdef	MP386
	if ((cmd.s[0] & WP) && (flag & FWRITE))
#else	/* !MP386 */
	if (cmd.s[0] & WP)
#endif	/* !MP386 */
	{
		return(FAIL);
	}
	return(OK);
}
#endif	/* WPCHECK */

/*
 * fdrecalib:   Recalibrate the device
 */
fdrecalib(dev)
dev_t dev;
{
        struct cmd cmd;
        unsigned char s0, s1, recalretries;

redo:
	fdcurcyl[UNIT(dev)] = 0;
	DBG14(printf("fdrecalib: unit=%x, fdcurcyl=%x\n",
		UNIT(dev), fdcurcyl[UNIT(dev)]));
        cmd.cmd = RECALIB;
        cmd.p[0] = UNIT(dev);
        cmd.np = 1;                     /* 1 parameter                  */
        cmd.ns = 2;                     /* Not really used. SIS in fdstart */
        fdcommand(&cmd, dev);
        if ((cmd.s[0] & IC) != 0) {
		if (recalretries++ == 3) {
			DBG14(printf("fdrecalib failed s0 =%x\n", cmd.s[0]));
                	return(FAIL);
		}
		goto redo;
        }
	DBG14(printf("fdrecalib succeeded. s0 =%x\n", cmd.s[0]));
	return(OK);
}

/*
 * fdbwait() -- Read main status reg., busy-wait polling until RQM is set, 
 *              and DIO is 0
 */
fdbwait(phase)
unsigned char phase;
{
        unsigned char status;
	unsigned long countb, countc;

	countb = 2;
	while (countb-- != 0 ) {
		countc = 0xFFFF;
		do {
			status = STATUS;
			if ((status & DIO) == (phase << 6)) {
				goto round2;
			}
		} while ( countc-- != 0 );
	}
	printf("fdbwait: first count timeout! sts=%x\n", status);
	return(FAIL);

round2:
	countb = 2;
	while (countb-- != 0 ) {
		countc = 0xFFFF;
		do {
			status = STATUS;
			if ((status & RQM) == RQM) {
				return(OK);
			}
		} while ( countc-- != 0);
	}

        /*
         * we get here only when timeout has run down... unfortunately,
         * there isn't any constructive action to take after complaining.
         */

	printf("fdbwait: second count timeout! sts=%x\n", status);
	return(FAIL);
}

/* fdcommand -- queue non-standard commands for execution via cfdbuf
 *	An incredibly useful tool.
 */
fdcommand(cp, dev)
struct cmd *cp;
dev_t dev;
{
	struct buf *bp, *ap;
	int s;
	extern struct fdfmt fmtbuf[];

	s = splbio();
	bp = &cfdbuf;
	DBG7(printf("fdcom: flags = %x\n", bp->b_flags));
	while (bp->b_flags & B_BUSY) {
		bp->b_flags |= B_WANTED;
		DBG7(printf("fdcom sleep\n")); 
		sleep(bp, PRIBIO);
	}
	bp->b_dev = dev;
	bp->b_flags = B_BUSY;

	/* driver's fdcmd structure mutexes using cfdbuf.
	 * copy caller's command over.
	 */
	fdcmd = *cp;

	/*  add cfdbuf to end of queue and be sure there's motion. */
	bp->av_forw = NULL;
	bp->av_back = NULL;
	bp->b_cylin = (unsigned int) 0xFFFF ;	/* for cyl sorting */
	bp->b_secno = (unsigned int) -1 ;	/* for sector sorting */
	if (fdtab.b_actf == NULL) {
		fdtab.b_actf = bp;
	} else {				/* stick on end of queue */
		for (ap = fdtab.b_actf; ap->av_forw != NULL; ap = ap->av_forw)
			;
		ap->av_forw = bp;
		bp->av_back = ap;
	}
	if (fdtab.b_active == IDLE) {
		DBG7(printf("fdcom: before start flags = %x\n", bp->b_flags));
		fdstart();
	}
	splx(s);

	/* otherwise, wait for command completion. when it occurs, the response
	 * bytes are already in the fdcmd structure. copy back the
	 * structure to our caller.
	 */
	iowait(bp);
	DBG8(printf("fdcmd: iowaited, "));
	*cp = fdcmd;

	/* if someone else is waiting, wake him/her. clear b_flags always */
	if (bp->b_flags & B_WANTED)
		wakeup(bp);
	bp->b_flags = 0;
	DBG8(printf("fdcmd: fini\n"));
}

/*
 * fdseek():       do a seek
 */
fdseek(bp)
struct buf *bp;
{
	extern struct fdparams *ftab[];
	struct fdparams *fp;

#ifdef NEVER
	if (fdcurcyl[UNIT(bp->b_dev)] != bp->b_cylin) {	/*same cyl => no seek*/
#endif
		fp = ftab[DEV];
        	fdbwait(CPHASE); CMD(SEEKCMD);
        	fdbwait(CPHASE); CMD((((bp->b_secno - 1) / fp->spt)<<2) | DEV);
        	DBG12(printf(" seek cmd wrd 2=%x, ",
			(((bp->b_secno - 1) / fp->spt)<<2) | DEV));
        	DBG12(printf(" seek cmd wrd 3=%x, b_cyl=%x step=%x\n", 
			(bp->b_cylin * fp->step), bp->b_cylin, fp->step));
        	fdbwait(CPHASE); CMD((unsigned char) (bp->b_cylin * fp->step));
		return(1);
#ifdef NEVER
	}
	return(0);
#endif
}

/*
 * fdrsetdma():    Setup the 8237 dma controller chip for a transfer.
 */
unsigned char
fdrsetdma(rw, addr, count)			/* M001 	*/
unsigned int rw, count;				/* M001 	*/
unsigned long addr;
{
        unsigned char mode, mask, labyte, habyte, lowcount, hicount, page; 
        unsigned int x;
	unsigned long	lcnt;

        mode = (rw & B_READ) ? FLDMARD : FLDMAWR;	/* M001 */
	lcnt = count;					/* M008: start */
	if ( _setdma((unsigned char) 2, mode, addr, &lcnt) )
		return(FAIL);
	_enabledma((unsigned char) 2);			/* M008: end */
	return(OK);
}


/*
 * initdma - Initialize all dma channels.
 *              DACK sense low, dreq sense high, late write, fixed pri,
 *              normal timing, controller enable, cho addr hold disable
 *              mem to mem disable.
 */
initdma()
{
        int x;

	if (!dmainitted) {		/* M002 */
		dmainitted = 1;		/* M002 */
        	x = splbio();

        	outb(MSTCLR1, 0xFF);    /* reset the slave      *
        	outb(MSTCLR2, 0xFF);    /* reset the master     */

        	outb(CMD2, 0x04);       /* controller disable   */
        	outb(CMD1, 0x04);

        	outb(MODE1, 0x40);      /* mode for ch. 0       */
        	outb(MODE2, 0xC0);      /* cascade mode for ch4 */

        	outb(MODE1, 0x41);      /* mode for ch. 1       */
        	outb(MODE2, 0x41);      /* mode for ch. 5       */

        	outb(MODE1, 0x42);      /* mode for ch. 2       */
        	outb(MODE2, 0x42);      /* mode for ch. 6       */

        	outb(MODE1, 0x43);      /* mode for ch. 3       */
        	outb(MODE2, 0x43);      /* mode for ch. 7       */

        	outb(CMD1, 0x00);
	        outb(CMD2, 0x00);       /* controller enable    */
        	splx(x);
	}				/* M002 */
}


/* 
 * fdresults():	read everything the 765 has to say.
 * input:	address of where to store the results.
 * output:	# of results read.
 */
fdresults(r)
struct results *r;
{
	unsigned int i=0, retries;

	while (waitsr() != FAIL) {
		if ((STATUS & DIO) != DIO) {	/* no longer results phase */
			return(i);
		}
		if ((STATUS & FDCBUSY) != FDCBUSY) {
			return(i);
		}
		r->r[i++] = inb(DATAREG);
	}
	return(i);				/* partial results */
}


/*
 * waitsr():	wait until the status register posts RQM
 */
waitsr()
{
	int retries = 0xFFFF;

	while (((STATUS & RQM) != RQM) && retries--)
		;

	if ((STATUS & RQM)) 
		return(OK);

	return(FAIL);
}

unsigned char fdrdcmd = RDCMD;				/* patchable	*/
unsigned char fdwrcmd = WRCMD;				/* patchable	*/
rw765(bp)
struct buf *bp;
{
	extern struct fdparams *ftab[];
	struct fdparams *f;
	unsigned char r, cmd, head, cyl;	/* M001			*/
	extern unsigned char docache();

	f = ftab[DEV];
	cmd = fdwrcmd;				/* Start M001:		*/
	head = (bp->b_secno - 1) / f->spt;
	cyl = bp->b_cylin;
	if (!docache(f, bp)) {			/* single sector	*/
		r = ((bp->b_secno - 1) % f->spt) + 1;	/* record #	*/
		if ((bp->b_flags & B_READ) == B_READ)
			cmd = fdrdcmd;
	} else {  /* multitrack */
		r = 1;   /* from r=1 to eot */
		if (cache.sts & FLCWR) {   /* need to sync: */
			head = cache.h;   /* cache to disk	*/
			cyl = cache.c;
		} else {   /* read */
			cmd = fdrdcmd;   /* M006: changed from RDTRACK */
		}
	}					/* End M001		*/

	DBG9(printf("rw765: r = %x, spt=%x, ", r, f->spt));
	DBG9(printf("head = %x, cyl=%x\n", head, cyl));
	fdbwait(CPHASE); CMD(cmd);
	fdbwait(CPHASE); CMD((head << 2) | DEV);	/* M001		*/
	fdbwait(CPHASE); CMD(cyl);			/* M001		*/
	fdbwait(CPHASE); CMD(head);			/* M001		*/
	fdbwait(CPHASE); CMD(r);
	fdbwait(CPHASE); CMD(f->nsec);
	fdbwait(CPHASE); CMD(f->eot);
	fdbwait(CPHASE); CMD(f->gpl);
	fdbwait(CPHASE); CMD(f->dtl);
}

readid(bp)
struct buf *bp;
{
	extern struct fdparams *ftab[];
	struct fdparams *fp;

	fp = ftab[DEV];
	fdbwait(CPHASE); CMD(READID);
	fdbwait(CPHASE); CMD((((bp->b_secno - 1)/fp->spt) << 2) | DEV);
}

#ifdef MP386
/* map the ISC minor devices into the microport kludges sort-of
	described below
*/
fdupfix(iscdev)
dev_t iscdev;
{
	dev_t dev,updev;
	dev = minor(iscdev);
	switch (dev & 0x30)
	{
	case 0x00:	updev = 0x46;
			break;
	case 0x20:	updev = 0x16;
			break;
	case 0x10:	updev = 0x17;
			break;
	default:
		printf("fd: unknown minor dev. no. %d\n",dev);
			updev = 0x46; 	/*default to normal */
	};
	if (dev & 0x4) updev |= 0x80;
	if (dev & 0x1) updev |= 0x08; /* determine DrvA or DrvB */
	DBG10(printf("ISC dev: %x uP dev: %x \n",iscdev,updev));
	return makedev(major(iscdev),updev);
}
#endif /* MP386 */

/*
 * Initialize the parameters that describe the floppy device, based upon
 * the minor dev. The minor bits are decrypted as follows:
 *	bit 0:	sectors per track:	0 = 8 spt,	1 = 9 spt
 *	bit 1:	# of sides:		0 = single,	1 = double sided
 *	bit 2:	if bit 4 not set, then:	0 = 48 tpi,	1 = 96 tpi
 *	bit 3:	drive:			0 = drive A,	1 = drive B
 *	bit 4:	sectors per track:	0 = 15 spt,	1 = use bit 0
 *	bit 5:	controller:		0 = card A,	1 = card B
 *	bit 6:	stepping:		0 = double,	1 = single
 *	bit 7:	file system offset	0 = all disk,	1 = 1 cylinder
 *
 * The above scheme is compatible with xenix 3.0 bit definitions,
 * with the following qualification: if bits 0, 1, and 2 are all
 * equal to 1, then a 15 spt, double-sided floppy is being accessed.
 * Also note that xenix 3.0 only uses bits 0-3. Thus, the above definitions
 * provide a super-set to xenix.
 * fdld.24: minor=23, fdld.25: minor = 151. fdhd.24: minor=70, fdhd.25, m=198
 * fd048ds8 - minor = 18
 */
 
fdbinit(dev, fp)
dev_t dev;
struct fdparams *fp;
{

	fp->nbps = 512;
	fp->nsec = 0x02;

	if ((dev & DS15SPT) == (SIDES & TPI96) && (!(dev & SPT9_15))) {	
						/* double-sided, 15 spt	*/
		fp->trrate = 0;			/* high density		*/
		fp->spt = 15;
		fp->tpc = 2;
		fp->ncyl = 80;
		fp->eot = 15;
		fp->gpl = 0x1B;
		fp->dtl = 0xFF;
		fp->spc = 30;
		fp->step = 1;
		fp->fgpl = 0x54;
		if (dev & FSOFF) {		/* start on cyl 1	*/
			fp->fsoff = fp->spt * fp->tpc;
		} else {			/* start on cyl 0	*/
			fp->fsoff = 0;
		}
		fp->limit = 2400 - fp->fsoff;
		return;
	}

	if ((dev & SPT9_15) == 0) {		/* just high density	*/
		fp->spt = fp->eot = 15;
	} else if (dev & SPT9) {		/* 9 sectors per track	*/
		fp->spt = fp->eot = 9;
	} else {				/* 8 sectors per track	*/
		fp->spt = fp->eot = 8;
	}

	if (dev & SIDES) {			/* double sided		*/
		fp->tpc = 2;
	} else {				/* single sided		*/
		fp->tpc = 1;
	}

	if (dev & FLUNIT) {			/* lower drive		*/
	} else {				/* upper drive (A)	*/
	}

	if (dev & CNTRLR1) {			/* card 1		*/
	} else {				/* card 0		*/
	}

	if (dev & SSTEP) {			/* single stepping	*/
		fp->step = 1;
	} else {				/* double stepping	*/
		fp->step = 2;
	}

	if (dev & FSOFF) {			/* start on cyl 1	*/
		fp->fsoff = fp->spt * fp->tpc;
	} else {				/* start on cyl 0	*/
		fp->fsoff = 0;
	}

	fp->spc = fp->tpc * fp->spt;

	if (dev & TPI96) {			/* 96 tracks per inch	*/
		switch(fp->spt) {
							/* start M009	*/
			case 9:			/* 720K in 1.2M drive	*/
				fp->trrate = 1;	/* 300 kbs		*/
				fp->ncyl = 80 - fp->fsoff / fp->spc;
				fp->gpl = 0x2A;
				fp->dtl = 0xFF;
				fp->fgpl = 0x50;
				fp->limit = 1440 - fp->fsoff;
							/* end M009	*/
				break;
			case 15:		/* 1.2M in 1.2M drive	*/
				fp->trrate = 0;	/* 500 kbs		*/
				fp->ncyl = 80 - fp->fsoff / fp->spc;
				fp->gpl = 0x1B;
				fp->dtl = 0xFF;
				fp->fgpl = 0x54;
				fp->limit = 2400 - fp->fsoff;
				break;
			case 8:			/* 320K in 1.2M drive	*/
				fp->trrate = 1;	/* 300  kbs	*/
				fp->ncyl = 40 - fp->fsoff / fp->spc;
				fp->gpl = 0x2A;
				fp->fgpl = 0x50;
				fp->dtl = 0xFF;
				fp->limit = 640 - fp->fsoff;
				break;
	    }
	} else { switch(fp->spt) {		/* 360K drive	*/
		case 9:				/* 360K floppy	*/
			fp->trrate = 2;		/* 250 kbs	*/
			fp->ncyl = 40 - fp->fsoff / fp->spc;
			fp->gpl = 0x1B;
			fp->dtl = 0xFF;
			fp->fgpl = 0x54;
			fp->limit = 720 - fp->fsoff;
			break;
		case 8:
			fp->trrate = 2;		/* 250 kbs	*/
			fp->ncyl = 40 - fp->fsoff / fp->spc;
			fp->gpl = 0x1B;
			fp->dtl = 0xFF;
			fp->fgpl = 0x54;
			fp->limit = 640 - fp->fsoff;
			break;
		}
	}
}
