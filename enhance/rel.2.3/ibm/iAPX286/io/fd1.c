static char *uportid = "@(#)fd1.c	Microport Rev Id  2.3.0 6/24/87";
/* Copyright 1985 by Microport. All Rights Reserved.
 *
 * NEC 765 (Intel 8272A) Floppy Disk Controller for the IBM AT.
 * References:
 *	Intel Microprocessor and Peripheral Handbook. Intel, 1983.
 *		Order #210844-001, pps 6-224 to 6-242 (8272A).
 *	IBM Technical Reference Manual for the AT, March 1984
 *		A: Fixed Disk and Diskette Adapter (Appendix)
 *
 * Initial Coding:
 *		uport!dwight	Fri Oct 18 22:19:51 PDT 1985
 *
 * Modification History:
 *
 *   M011:	uport!rex	Sun Aug 30 19:14:35 PDT 1987
 *   M010:	uport!rex	Wed Jun 24 19:56:04 PDT 1987
 *	In modifying FD to use the new DMA interface, it was necessary
 *	to change the name of the routine "setdma" to "fdrsetdma" since
 *	there was a name conflict.  This "fdrsetdma" routine has also
 *	been modified.
 *	Also added checks to see if "fdtimeout" is non-zero before calling
 *	timeout().  If "fdtimeout" is patchable and if zero, the floppy
 *	will stay on and selected until closed.
 *   M009:	uport!dwight	Sat Apr 25 14:12:18 PST 1987
 *	Support write-protected disks if this is the boot floppy.
 *	All ifdef'd on M009
 *   M008: uport!fredo   Tue Apr  7 20:55:22 PST 1987
 *		Made fix so that fsck would work.  Basically,
 *        we add a stub command, QDCMD ("queue drain"),
 *        to the end of the command buffer to guarantee
 *        that the cache buffers are flushed before
 *        fdclose() is called.
 *   M007: uport!fredo   Thu Apr  1 09:48:47 PST 1987
 *        Removed all references to rdtrack and changed
 *        it to read data to enable reading of ISC
 *        formatted floppies when floppy caching is on.
 *	M006: uport!dean	Thu Mar 26 13:02:04 PST 1987
 *		changed fdinit to fdbinit to not clash with
 *		standard boot time init. Added stub of fdinit for
 *		boot time
 *   M005: uport!fredo   Thu Mar  5 18:58:07 PST 1987
 *        Incorporated ISC ioctls: V_FORMAT and V_GETPARM
 *  		and test for write protect condition
 *	M004: uport!fredo   Thu Feb 19 18:52:26 PST 1987
 *		Incorporated 138 revisions
 *	M003: uport!dean	Fri Jan  2 16:25:00 PST 1987
 *		Modified for 386 kernel
 *	M002: uport!rex	Tue Dec 23 13:11:20 PST 1986
 *		I215_FLOPPY ioctl() command:
 *		Returns the # of 512 byte blocks on the device.
 * To do:
 *	1) single side support
 */

#include "sys/fd.h"
#include "sys/8237.h"
#include "sys/ioctl.h"
#include "sys/format.h"
#include <sys/file.h>
#ifdef MP386
#include "sys/vtoc.h"
#endif
#ifdef ATMERGE
#include "sys/realmode.h"
#endif /* ATMERGE */

#define SELTAR 0x2c00000

extern unsigned char initdma(), fdreset(), fdspecify(), fdrecalib();
unsigned char (*fdinit_tbl[])() = { fdreset, fdspecify, fdrecalib, 0 };
uint devstat[2] = {0,0};		/* current # of opens on dev	*/

#ifdef LCCFIX

#define MNT 1 	
#define OPN 2

/* devotyp[unit] is a bit mask keeping track of all opens.
* if bit 1 is set, the unit is open on a mount(), if bit 2 is
* set it's a regular open. The final close will occur only if
* both bits are cleared.
*/
uint devotyp[2] = {0,0};
#endif LCCFIX

static char openwait=0;
static char retries = RWRETRY, skretries = SKRETRY, riretries = RIRETRY;
static unsigned long io_s1, io_s2;
static char firsttime[2]= {0, 0};
static unsigned int altbufflg = 0;
static unsigned char altbuf[512];	/* alt. for dma rollovers	*/
uint fdcurcyl[2] = {0, 0};		/* assume recalib here 		*/
uint dbglvl = 0;			/* determines printf's		*/
int specwrd1 = SPECWRD1;		/* tunable, via patch		*/
int specwrd2 = SPECWRD2;		/* tunable, via patch		*/
char drvlock[2] = {0, 0};		/* drive is being turned off	*/
unsigned short fdtimeout = 3;		/* timeout in secs		*/
unsigned short cachetimeout = 1;	/* cache sync window in secs	*/
uint fdunit = 3;			/* current active unit		*/
int fdcache = 0;			/* patchable 1 = don't cache	*/
struct cmd tcmd;			/* for temporary commands	*/
dev_t lastdev;
char cachewait = 0;
time_t lastaccess;			/* lbolt time of last activity	*/
uint fddebug = 0;			/* patchable debug parameter	*/

#ifdef MP386
#define	KVaddr(x)	phystokv((kvtophys(x)))
extern caddr_t svirtophys();
int  hz = HZ;
#endif /* MP386 */

extern unsigned char cbuf0[];

/* Start M009 */
/* Write-protected boot disks are supported under special circumstances.
 * First, the root device has to be the floppy. Second, the swap device
 * has to be the ramdisk. If both of these are true, then *all* write
 * commands to the floppy driver do not get written out to the disk. The
 * driver state machine only acts as if it does.
 */
#define M009
static int writeprot = -1;		/*  1 = disk is write protected	*/
					/*  0 = not write protected	*/
					/* -1 = not initialized (M011)	*/
/* end M009 */

struct fdparams drivea, driveb;
struct fdparams *ftab[] = { &drivea, &driveb };

#define	NSEC	15		/* max # of sectors per track 		*/
struct fdfmt fmtbuf[NSEC*2];	/* contain nec. format info 		*/
#ifndef MP386
int	fddmad;
#endif

/*
 * fdinit()
 * To satisfy reference in standard config	M006
 */
fdinit()
{
	initdma();
}

#ifdef LCCFIX
fdopen(dev, flag, omnt) 
	dev_t dev;
	int flag;
	unsigned omnt;
#else /* -LCCFIX */
fdopen(dev, flag) 
	dev_t dev;
	int flag;
#endif /* LCCFIX */
{
	unsigned char (*s)(), (**initptr)(), ft;
	extern unsigned char (*fdinit_tbl[])();
	dev_t unit;
	struct buf *ap;

	DBG1(printf("fdopen"));
#ifdef MP386
	dev = fdupfix(dev);   /* convert to uport minors */
#endif
	unit = UNIT(dev);
#ifdef ATMERGE
	/* try to get control of the device */
	if (devclaim(DEVN_FLOPPY)) {
		u.u_error = EBUSY;	/* DOS has the device */
		return;
	}
#endif /* ATMERGE */

	DBG1( printf("fdopen: devstat[%x]=%x \n", unit, devstat[unit]));
	openwait++;
	if (openwait != 1) {			/* mask out other opens() */
		sleep((caddr_t) &openwait, PRIBIO+1);
	}

	ft = firsttime[unit];
	if ((ft == 0) || (ft != (dev & INITMASK))) {
#ifndef	MP386						/* start M010 */
		if ((fddmad = opendma(2)) == -1) {
			u.u_error = ECHRNG;
			devstat[unit] = 0;	/* uninitialized  */
			firsttime[unit] = 0;
			if (--openwait)
				wakeup((caddr_t) &openwait);
			return;
		}
#endif	/* !MP386 */					/* end M010 */
		firsttime[unit] = (dev & INITMASK);
		fdbinit(dev, ftab[unit]);		/* M006 */

		/* mark buffer caches as uninitialized */
		cache.c = (uint) -1;
		cache.h = (uint) -1;
		cache.sts = (unsigned char) 0;
		cache.cp = &cbuf0[0];
		cfdbuf.b_flags = 0;	/* M011: command buf not in use */

#ifndef	MP386						/* start M011 */
#ifdef M009
							/* Start M009 */
		/* Test for this being a write-protected boot floppy */
		if (writeprot == -1) {
			if ((rootdev == makedev(1,198)) &&
			    (swapdev == makedev(2,0)))
				writeprot = 1;
			else
				writeprot = 0;
	DBG1(printf("fdopen: swapdev=%x writeprot=%x \n", swapdev, writeprot));
		}					/* End M009 */
#endif M009
#endif	/* !MP386 */					/* end M011 */


		initptr = &fdinit_tbl[0];

		while (s = *initptr++) {		/* Initialize FDC */
			if ((*s)(dev) == FAIL) {
				u.u_error = EIO;
				devstat[unit] = 0;	/* uninitialized  */
				firsttime[unit] = 0;
				fdoff(dev);
				closedma(fddmad);	/* M010 */
				if (--openwait) {
					wakeup((caddr_t) &openwait);
				}
				return;
			}
		}
	}

#ifdef	WPCHECK
	/*						Begin M011
	 * check if floppy is write protected and
	 *  if 80286 and FWRITE access and not floppy rooted,
	 *     fail the open and return.
	 *  if 80386, fail the open and return.
	 */
	if (fdwrtchk(dev, flag) == FAIL)
#ifndef	MP386
	{
		if ((flag & FWRITE) && (rootdev != makedev(1,198)))
#endif	/* !MP386 */
		{
			u.u_error = EIO;
			devstat[unit] = 0;		/* uninitialized  */
			firsttime[unit] = 0;
			fdoff(dev);
			closedma(fddmad);
			if (--openwait) {
				wakeup((caddr_t) &openwait);
			}
			return;
		}
#ifndef	MP386
	} else if (writeprot == 1) {
	    /*
	     *	writeprot == 1 only if root device is the floppy
	     *			   and swap device is the RAM disk
	     *	Un-write-protected boot floppy will swap on the diskette
	     *	  equivalent to dfile.flop entry:
	     *	 	swap	flop	198	2120	250
	     */
		swapdev = makedev(1,198);
		nswap = 250;
		swplo = 2120;
		writeprot = 0;
	    DBG1(printf("fdopen: reset swapdev to %x at %ld\n", swapdev,swplo));
	}						/* End M011 */
#endif	/* !MP386 */
#endif	/* WPCHECK */

	devstat[unit]++;				/* Open for business */

#ifdef LCCFIX
	if (omnt)
	    devotyp[unit] |= MNT;
	else
	    devotyp[unit] |= OPN;
#endif LCCFIX
	if (--openwait) {
		wakeup((caddr_t) &openwait);
	}

}

/* called only on the last close. reference count is handled in closef() */
#ifdef LCCFIX
fdclose(dev, flag, omnt)
    dev_t dev;
    int flag;
    unsigned omnt;
#else /* -LCCFIX */
fdclose(dev)
    dev_t dev;
#endif /* LCCFIX */
{
	struct cmd cmd;
	uint unit, x;
	unsigned char d, newdor;

	DBG1(printf("fdclose"));
#ifdef MP386
	dev = fdupfix(dev);   /* convert to uport minors */
#endif
	unit = UNIT(dev);
	DBG2(printf("fdclose, devstat[%x] was %x, dorstat = %x\n", 
		unit, devstat[unit], dorstat));

	fdquedrain(dev);   /* M008: drain buffer queues before closing */

#ifdef LCCFIX
	DBG2(printf("---- devotyp[%x] was %x\n", unit, devotyp[unit]));
	if (omnt)
	    devotyp[unit] &= ~MNT;
	else
	    devotyp[unit] &= ~OPN;
	if (devotyp[unit])		/* if device is open, don't close */
	    return;
#endif LCCFIX

	x = splbio();
	dorstat &= 0xFE;			/* make sure correct drive*/
     dorstat |= fdsel[unit];			/* is enabled		  */
	outb(DOR, dorstat);
	outb(DCR, ftab[UNIT(dev)]->trrate);
	splx(x);

	cacheflush(dev);			/* sync cache if neccessary */
	fdrecalib(dev);				/* required		  */

	x = splbio();
	devstat[unit] = 0;
	firsttime[unit] = 0;
	
	cache.c = (uint) -1;			/* mark cache buffers as*/
	cache.h = (uint) -1;			/* uninitialized	*/
	cache.sts = 0;
	fdoff(dev);				/* turn off device	*/
	splx(x);
	DBG2(printf("fdclose, new dorstat=%x \n", dorstat));
	DBG2(printf("fdclose, devstat = [%x, %x]\n", devstat[0], devstat[1]));
#ifdef ATMERGE
	devrelse(DEVN_FLOPPY);
#endif /* ATMERGE */
#ifndef	MP386
	closedma(fddmad);			/* M010 */
#endif	/* !MP386 */
}

/* turn off the appropriate floppy device */
fdoff(dev)
dev_t dev;
{
	uint unit;
	unsigned char d;

	unit = UNIT(dev);

	if (devstat[0] == 0 && devstat[1] == 0)  {   /* no more floppy action */
       	dorstat &= 0x0F;		/* don't generate irq's	 */
	} else {				/* other unit is active  */
		d = ((unit == 0) ? DRIVEAE : DRIVEBE);
		dorstat &= ~d;			/* disable motor	 */
	}
	outb(DOR, dorstat);
}

fdstrategy(bp)
struct buf *bp;
{
	daddr_t track, bn, limit;
	unsigned int maxcnt;
	int x;
	int debug=0;
	struct cmd cmd;
	long physaddr();
	unsigned char c, *lp;
	unsigned long tmp;
	struct fdparams *fp;
	unsigned char fsoff;
	struct buf *ap, *np;
	extern struct fdparams *ftab[];
	extern struct buf *queswap();
	uint scyl;

	DBG1(printf("fdstrategy"));
	bp->b_start = lbolt;
	bp->b_resid = bp->b_bcount;
	fp = ftab[DEV];

#ifdef MP386
	if (fdupfix(bp->b_dev) & FSOFF) {			/* off 1 cyl.	*/
#else
	if (bp->b_dev & FSOFF) {			/* off 1 cyl.	*/
#endif
		fsoff = fp->spt * fp->tpc;
	} else {
		fsoff = 0;				/* entire disk	*/
	}

	switch (fp->spt) {
		case 8:
			limit = 640 - fsoff;
			break;
		case 9:
			limit = 720 - fsoff;
			break;
		case 15:
			limit = 2400 - fsoff;
			break;
		default:
			printf("fdstrat: unknown spt=%d, dev=%d\n", fp->spt,
#ifdef MP386
					fdupfix(bp->b_dev));
#else
					bp->b_dev);
#endif
			bp->b_flags |= B_ERROR;
			bp->b_error = ENXIO;
			iodone(bp);
			return;
	}

	/* Determine sector and cylinder from block #			*/
	/* The head number will be determined in fdstart().		*/
	bn = bp->b_blkno + fsoff;
	bp->b_secno = (daddr_t) (bn % fp->spc) + 1; /* 1<=b_s<=SPC*/
	bp->b_cylin = (uint)( bn / fp->spc);

	DBG13(printf("\n---fdstrat: [bn=%lx, b_n=%lx, b_sec=%x, track=%x",
		bn, bp->b_blkno, bp->b_secno, track));
	DBG13(printf(", cyl=%x, count=%x,", bp->b_cylin, bp->b_bcount));
	DBG13(printf("spc = %x", fp->spc));
	DBG13(printf(" fsoff=%d, limit=%ld\n", fsoff, limit));

	/*
	 * Validate io according to these rules:
	 * 	Any request that starts inside the floppy (i.e.
	 * within NCYL) is ok. if it's going to run over the
	 * end, trim back b_bcount. after i/o, b_resid will show
	 * how many bytes were not transferred.
	 *	Any request beyond the end of the floppy disk will not
	 * be performed, and will be bounced back with ENXIO.
	 */

	/* If device isn't ready, or off the end of dev: */
#ifdef MP386
	if ((bp->b_blkno > limit) || (devstat[UNIT(fdupfix(bp->b_dev))] == 0)) {
#else
	if ((bp->b_blkno > limit) || (devstat[UNIT(bp->b_dev)] == 0)) {
#endif
		bp->b_flags |= B_ERROR;
		bp->b_error = ENXIO;
		iodone(bp);
		return;
	}

	/* Within the floppy's limits. Will request go over? */
	if ((daddr_t) bp->b_blkno ==  limit) {
		if (bp->b_flags & B_READ) 
			bp->b_resid = bp->b_bcount;
		else {
			bp->b_error = ENXIO;
			bp->b_flags |= B_ERROR;
		}
		iodone(bp);
		return;
	}

	/* check if request over */
	if ((bp->b_blkno + ((bp->b_bcount + 511)/FLBSIZE)) > ((daddr_t) (limit))) {
		bp->b_resid = bp->b_bcount;
		iodone(bp);
		return;
	}

	/* get physical address of buffer */
	DBG5(printf("fdstrat: b_addr=%lx, ", bp->b_un.b_addr));
#ifdef MP386
	DBG5(printf(" paddr=%lx\n", paddr(bp)));
#else
	bp->b_physaddr = physaddr(bp->b_un.b_addr);
	DBG5(printf(" paddr=%lx\n", bp->b_physaddr));
#endif /* MP386 */


	x = splbio();

	/* 
	 * Insert buffer into queue. Clear B_ERROR bit and b_error field, 
	 * to be safe.
	 */

	bp->b_flags &= ~B_ERROR;
	bp->b_error = 0;

	/* Insert buffer into list */
	bp->av_forw = NULL;
#define FLSORT					/* M001 */
#ifndef FLSORT					/* 1.3.6 style sorting */
	if (fdtab.b_actf == NULL) {
		fdtab.b_actf = bp;
	} 
	else {
		ap = fdtab.b_actf;
		while (ap->av_forw != NULL) {
#define LINEAR
#ifdef LINEAR 
			if (bp->b_blkno > ap->b_blkno) { /* M001 */
				ap = ap->av_forw;
			}
			else
				break;
#else
		ap = ap->av_forw;
#endif /* LINEAR */
		}
		bp->av_forw = ap->av_forw;
		ap->av_forw = bp;
	}
/* Begin M000 */
#else
	bp->av_back = NULL;
	if (fdtab.b_actf == NULL) {
		fdtab.b_actf = bp;
	} else {				/* sort via cyl & sector */
		ap = fdtab.b_actf; 

		/* search to end of queue, looking for ap: bp < ap. */	
		for ( ; ap->av_forw != NULL; ap = ap->av_forw) {
			if (bp->b_cylin > ap->b_cylin) { 
				continue;
			} else if (bp->b_cylin == ap->b_cylin) {
				if (bp->b_secno > ap->b_secno) {
					continue;
				}
			}
			break;
		} 

		/* if ap is at the head of the queue, but bp < ap,
		 * then we must be careful to avoid a cache sync 
		 * (i.e. moving to another cyl/head). to do this, 
		 * we scan until the next cylinder, or end of queue,
		 * and attach bp after ap.
		 */
		if (ap->av_back == NULL ) {		/* queue head */
			scyl = ap->b_cylin;
			while((ap->av_forw != NULL) && (scyl == ap->b_cylin)){
				ap = ap->av_forw;
			}
		}

		/* if bp < ap,and ap isn't head of queue, insert bp before ap.*/
		if ((ap->av_back != NULL) && (bp->b_cylin < ap->b_cylin)
			   && (bp->b_secno < ap->b_secno)) { /* bp before ap */
			ap->av_back->av_forw = bp;
			bp->av_back = ap->av_back;
			ap->av_back = bp;
			bp->av_forw = ap;

		} else {				/* ap before bp */
			if (ap->av_forw != NULL) {
				ap->av_forw->av_back = bp;
			} 
			bp->av_forw = ap->av_forw;
			ap->av_forw = bp;
			bp->av_back = ap;
		}
	}
/* End M000 */
#endif /* FLSORT */

	/* If controller isn't busy, then start io request */
	if (fdtab.b_active == IDLE) {
		fdstart();
	}

	splx(x);

}

fdread(dev)
dev_t dev;
{
	struct fdparams *rp;

#ifdef MP386
	int fdbreakup();
#endif

	rp = ftab[UNIT(dev)];

#ifdef MP386
	physio(fdbreakup, &rfdbuf, dev, B_READ);
#else
	physio(fdstrategy, &rfdbuf, dev, B_READ);
#endif
}

fdwrite(dev)
dev_t dev;
{
	struct fdparams *rp;

#ifdef MP386
	int fdbreakup();
#endif

	rp = ftab[UNIT(dev)];

#ifdef MP386
	physio(fdbreakup, &rfdbuf, dev, B_WRITE);
#else
	physio(fdstrategy, &rfdbuf, dev, B_WRITE);
#endif
}

#ifdef MP386
/*
 * Stolen from i214.c.
 * This breaks up 64K-hunks and does something magic to the page tables.
 * It was necessary for the winnie driver.  Maybe it will make caching work. 
 */
fdbreakup(bp)
register struct buf *bp;	/* buffer header */
{
	dma_breakup(fdstrategy, bp);
}
#endif /* MP386 */

/*
 * I215_FLOPPY command:
 * Returns the # of 512 byte blocks on the floppy device if called for a floppy
 * device, or zero if called for a winchester device.
 *
 * The following are applicable only on an iAPX386 based machine:
 *   V_FORMAT:  Based on the starting track number and the number
 *			 of tracks, this call is used to format tracks on
 *			 a diskette.  The argument must contain the starting
 *			 track, number of tracks, and interleave factor.
 *   V_GETPARM: Returns information about the current drive configuration.
 *			 The argument to the ioctl is the address of a disk_parms
 *			 structure defined in <sys/vtoc.h>.
 */
fdioctl(dev, com, arg, flag)
dev_t dev;
uint com, flag;
union ioctl_arg	arg;
{
	unsigned long t;
	struct fdparams *fp;
 	struct i215ftk control;
#ifdef MP386
	union io_arg *ia;   /* used in V_FORMAT */
	struct disk_parms *vdp;   /* used in V_GETPARM */
#endif

	DBG1(printf("fdioctl"));
#ifdef MP386
	dev = fdupfix(dev);   /* convert to uport minors */
#endif
	fp = ftab[UNIT(dev)];

	switch (com) {
#ifdef MP386
		case V_FORMAT:   /* format one track at a time */
#ifdef DEBUG9
			printf("ioctl: V_FORMAT on dev %d\n",dev);
#endif
			/* Get parameters from user segment */
			ia = (union io_arg *)arg.sparg;

			/* validate number of tracks */
			if (((ia->ia_fmt.start_trk < 0) || (ia->ia_fmt.start_trk + ia->ia_fmt.num_trks)) > (fp->ncyl*fp->tpc))  {
				u.u_error = EINVAL;
				return;
    			}

			fdformat(dev, ia->ia_fmt.start_trk);   /* format the track */

#ifdef DEBUG9
			printf("ioctl: format (%d) (%d) (%d)\n", ia->ia_fmt.start_trk,
						ia->ia_fmt.num_trks, ia->ia_fmt.intlv);
#endif
			break;
		case V_GETPARMS:   /* caller wants device parameters */
#ifdef DEBUG9
			printf("ioctl: V_GETPARMS on dev %d\n",dev);
#endif
			vdp = (struct disk_parms *)arg.sparg;   /* get user args */

			vdp->dp_type = DPT_FLOPPY;   /* disk type               */
			vdp->dp_heads = fp->tpc;     /* number of heads (1 or 2)*/
			vdp->dp_cyls = fp->ncyl + fp->fsoff/fp->spc;  /* # cyls */
			vdp->dp_sectors = fp->spt;   /* number of sectors/track */
			vdp->dp_secsiz = fp->nbps;   /* number of bytes/sector  */
			vdp->dp_pstartsec = fp->fsoff;   /* absolute starting sector */
			vdp->dp_pnumsec = fp->spt*fp->tpc*fp->ncyl;   /* tot #of sec's */
#ifdef DEBUG9
			printf("About to return type %d  ncyl %d  heads %d\n",
				 vdp->dp_type, vdp->dp_cyls, vdp->dp_heads);
			printf("             sec/trk %d  secsiz %d\n",
				 vdp->dp_sectors, vdp->dp_secsiz);
#endif
			break;
#endif /* MP386 */
		case I215_NTRACK:		/* # tracks this device	*/
			t = (unsigned long) fp->ncyl;
			if (copyout(&t, arg.sparg, sizeof(t)))
				u.u_error = EFAULT;
			break;

		case I215_IOC_FMT:		/* format command	*/
			if (copyin(arg.sparg, &control, sizeof(control))){ 
 				u.u_error = EFAULT;
 				break;
 			}

			if (fdformat(dev, control.f_cyl) == FAIL) {
				u.u_error = ENXIO;
			}
			break;

		/* M002 */
		case I215_FLOPPY:
			u.u_rval1 = fp->spt * fp->tpc * fp->ncyl;
			break;

		/* Dummy ioctl() to get around stupid ENOTTY message */
		case I215_CTEST:
			break;

		default:
			u.u_error = ENXIO;
			break;
	}
	return;
}


fdformat(dev, cyl)
dev_t dev;
int cyl;
{
	struct fdparams *fp;
	unsigned char h, r, fretry=10, i, head, head_unit;
	struct cmd cmd;
	struct buf *bf;
	extern struct fdfmt fmtbuf[];

	fp = ftab[UNIT(dev)];

/* seek to track/cylinder. format one track at a time.			*/
	for (head=0; head < 2; head++) {
		head_unit = (head<<2 | 0);		/* hardwire unit*/
		cmd.cmd = SEEKCMD;
		cmd.p[0] = head_unit;
		cmd.p[1] = (unsigned char) (fp->step * cyl);
		cmd.np = 2;
		cmd.ns = 2;				/* results from SIS */
		fdcommand(&cmd, dev);

		for (r = 0; r < fp->spt; r++) {		/* for each sector*/
			fmtbuf[r].c = (unsigned char) cyl;
			fmtbuf[r].h = head;
			fmtbuf[r].r = r+1;
			fmtbuf[r].n = fp->nsec;		/* N 		*/
		}

		cmd.cmd = FLFORMAT;
		cmd.p[0] = head_unit;
		cmd.p[1] = 2;				/*fp->nsec;	*/
		cmd.p[2] = fp->spc/2;			/*fp->spc;	*/
		cmd.p[3] = fp->fgpl;			/*fp->fgpl;	*/
		cmd.p[4] = (unsigned char) cyl;		/*data stamp	*/
		cmd.np = 5;
		cmd.ns = 7;
		fdcommand(&cmd, dev);
#ifdef DEBUG9
		printf("fdformat end: s=[%x, %x, %x, %x, %x, %x, %x], ",
			cmd.s[0], cmd.s[1], cmd.s[2], cmd.s[3], cmd.s[4],
			cmd.s[5], cmd.s[6], cmd.s[7]);
#endif /* DEBUG9 */
	}
	return(OK);
}

/*
 * generic kernel print routine
 */
fdprint(dev, s)
int dev;
char *s;
{
#ifdef MP386
	dev = fdupfix(dev);   /* convert to uport minors */
#endif
	printf("%s on floppy device %d, unit %d\n", s, dev, UNIT(dev));
}


/* fdintr: Handle 765 events. The real work is done in fdstart().	*/

fdintr(level)
int level;
{
	int x;
#ifdef LCCFIX
	struct results r;
#endif

	DBG3(printf("fdintr: - state = %x, level=%x\n", fdtab.b_active, level));
	x = splbio();
#ifndef MP386
#ifndef	PICFIX1
	eoi(level);					/* ack irq */
#endif /* ! PICFIX1 */ 
#endif
	switch (fdtab.b_active & (XFER|FLCMD)) {
		case XFER:
		case FLCMD:
			fdtab.b_active |= IRQ;
			fdstart();
			splx(x);
			return;
	}
	splx(x);
#ifdef LCCFIX
	/* If we get an unexpected interrupt, try to see if it seems
	 * safe to ignore, and do so if possible.  We tend to get
	 * unexpected interrupts when calling fdoff().
	 */
	CMD (SIS);
	x = fdresults (&r);
	if (r.r[0] & ~HD0)	/* any bits on besides head address? */
	    printf ("fdintr: unexpected irq - state = %x, sis[0] = %x\n",
		fdtab.b_active, r.r[0]);

	/* Note that the code below that is advertised to "try to clear"
	   in fact does nothing useful. */
#else /* -LCCFIX */
	/*
	 * Here we've picked up an unexpected irq for the present state.
	 * Complain bitterly, and try to clear. 
	 */
	printf("fdintr: unexpected irq - state = %x\n", fdtab.b_active);
#endif /* -LCCFIX */
}

/* fdstart - main state machine for request processing. Driven by
 * the b_active field in fdtab. Called *ONLY* with irq's prevented.
 * Should fit on one page (<~65 lines).
 */
fdstart()
{
	struct buf *bp;
	unsigned char sts, xfer(), xferirq(), spcmd();
	extern fdresults(), rw765(), readid();
	extern int driveaoff(), driveboff();
	extern hz, cachesync();
	unsigned short ccbug;

top:
	bp = fdtab.b_actf;
	if (bp == NULL) {
		DBG6(printf("fdstart: req end \n"));
		if (fdtab.b_active != IDLE) {
			printf("fdstart -- unexp null reqptr\n");
			return;
		}
		if ((cache.sts & FLCDIRTY) && (cachewait == 0)) {
			if ((cache.sts & FLSYNC) != FLSYNC) {
			cachewait = 1;
			DBG6(printf("fdstart: queue cachesync check\n"));
			ccbug = (unsigned short) cachetimeout*hz;
			timeout(cachesync, (caddr_t)0, (ushort) ccbug); 
			}
		}

		if ((fdunit != 3) && (drvlock[fdunit] == 0) && (devstat[fdunit] != 0)) {
			drvlock[fdunit]++;
			if (fdtimeout) {			/* M010 */
			    if (fdunit)
				timeout(driveboff, (caddr_t)0, fdtimeout*hz);
			    else
				timeout(driveaoff, (caddr_t)0, fdtimeout*hz);
			}
		}
		fdunit = 3;			/* invalid indicator */
		return;
	}
newstate:
	switch(fdtab.b_active & (FLCMD|XFER|IRQ)) {
	case IDLE:				/* bp references a new req */
		fdtab.b_active |= ((bp == &cfdbuf) ? FLCMD : XFER);
		lastaccess = lbolt;
		goto newstate;

	case XFER:				/* a read/write req.	*/
		if (xfer(bp) == OK) 		/* issue proper command	*/
			return;			/* Wait for irq		*/
		break;				/* Finished with req.	*/

	case XFER+IRQ:	/* a transfer irq: get status, decide what to do*/
		if (xferirq(bp) == OK) 
			goto newstate;
		break;

	case FLCMD:			/* non-std cmd in fdcmd struct. */
		if (spcmd(bp) == OK)		/* issue special command*/
			return;			/* wait for irq		*/
		break;				/* finished with req	*/
		
	case FLCMD+IRQ:				/* irq after non-std cmd*/
		spcmdirq(bp); 			/* log response 	*/
		break;

	default:
		printf("fdstart: unexpected state %x\n", fdtab.b_active);
		return;
	}
	/* break from the main switch only to finish a request. If there was 
	 * an error, b_error has been set to some nonzero error code, and we 
	 * should set B_ERROR. iodone() is always called, and the queue advanced
	 */
	if (bp->b_error != 0) {
		bp->b_flags |= B_ERROR;
	}
#ifdef FLSORT						/* M000 */
	if (bp->av_forw != NULL)
		bp->av_forw->av_back = NULL;
#endif
#ifdef MP386
	lastdev = fdupfix(bp->b_dev);   /* convert to uport minors */
#else
	lastdev = bp->b_dev;
#endif
	fdtab.b_actf = bp->av_forw;
	iodone(bp);
	fdtab.b_active = IDLE;

	goto top;
}

/* xfer:	Issue the appropriate sequence of commands to transfer data.
 *	Returns OK if the proper sequence is being carried out.
 *	Returns FAIL if finished with driving the 765, and the req should end.
 *
 *	This is where all data requests start. Before any data can be
 *	transferred, the 765 demands that a seek and readid be performed 
 *	first; i.e. each data request consists of a seek()/readid()/rw765().
 *	So seek and readid are treated as substates to xfer request. Only
 *	when both actions have been performed is the r/w command issued.
 *
 *	The irq generated by each substate indicates that the substate
 *	has finished; this status handling is done in xferirq(). If 
 *	everything is going according to plan, it will handle advancing
 *	the substate.
 */

unsigned char
xfer(bp)
struct buf *bp;
{
	extern unsigned char fdsetdma(), cachecheck(), docache();
	struct fdparams *f;

#ifdef MP386
	fdunit = UNIT(fdupfix(bp->b_dev));
#else
	fdunit = UNIT(bp->b_dev);
#endif
	f = ftab[DEV];
	switch (SUBSTATE) {
		default:			/* First time for XFER	*/
			/* First check if the block is accessible via cache. 
			 * If it is, we can skip all of the driving.
		 	 */
			if (cachecheck(bp) == OK) {	
				/* 
				 * reads: data was in cache, so end. 
				 * writes: update cache *only*. disk will
				 * be updated when track changes, or end
				 * of queue.				 
				 */

				return(FAIL);	/* no failure; just end */

			}	
			/* Here the request isn't in the cache. Activate
			 * the appropriate unit in preparation for a xfer.
			 * If the cache is dirty, it will have to be
			 * written out to disk before the head changes
			 * to a different cylinder.
			 */
			dorstat &= 0xFE;	/* turn on correct unit	*/
#ifdef MP386
       		dorstat |= fdsel[UNIT(fdupfix(bp->b_dev))];
#else
       		dorstat |= fdsel[UNIT(bp->b_dev)];
#endif
			outb(DOR, dorstat);
#ifdef MP386
			outb(DCR, ftab[UNIT(fdupfix(bp->b_dev))]->trrate);
#else
			outb(DCR, ftab[UNIT(bp->b_dev)]->trrate);
#endif

			if (docache(f,bp)) {	/* multitrack		*/
				/* if cache is dirty, sync */
				if (cache.sts & FLCDIRTY) {
					cache.sts |= FLCWR; /*update happening*/
					cache.sts &= ~FLCRD;
					break;	
				}
				else {
					cache.sts |= FLCRD;/* update cache */
					cache.sts &= ~FLCWR;
				}
			}
			fdtab.b_active |= LIVE_SEEK;
			fdseek(bp);
			return(OK);		/* await irq		*/

		case LIVE_SEEK:
			fdtab.b_active &= ~(LIVE_SEEK);
			fdtab.b_active |= LIVE_READID;
			readid(bp);		/* issue read id cmd	*/
			return(OK);		/* await irq		*/

		case LIVE_READID:
			fdtab.b_active &= ~(SUBSTATE);
			break;
	}

	/* We're now at correct cyl. Do the read/write:	*/
	if ((fdtab.b_active & FLMORE) != FLMORE) { /* req start */
		io_s1 = bp->b_bcount;		/* running bytes to go  */
#ifdef MP386
		io_s2 = KVaddr(paddr(bp));	/* running i/o address  */
#else
		io_s2 = bp->b_physaddr;		/* running i/o address  */
#endif
	}

	if (fdsetdma(bp) == FAIL) {
		printf("X: fdsetdma failed!\n");
		bp->b_error = EIO;
		return(FAIL);
	}
	rw765(bp);			/* issue the r/w command	*/
	return(OK);
}

/* xferirq:	Handle irq generated from xfer() state-machine.
 *	Returns OK if substate has been handled correctly (newstate required).
 *	Returns FAIL when i/o needs to finish (not neccessarily a failure).
 */
unsigned char
xferirq(bp)
struct buf *bp;
{
	int i, j, sts;
	struct fdparams *f;
	unsigned char *cp, head;
	struct results r;

	fdtab.b_active &= ~IRQ;
	switch(SUBSTATE) {
		case LIVE_SEEK:		/* Finished with seek	*/
			fdbwait(CPHASE); CMD(SIS);
			i = fdresults(&r);
			sts = r.r[0];
#ifdef MP386
			fdcurcyl[UNIT(fdupfix(bp->b_dev))] = r.r[1];
#else
			fdcurcyl[UNIT(bp->b_dev)] = r.r[1];
#endif

			DBG3(printf("seek results: ["));
			for(j=0; j < i; j++) {
				DBG3(printf("%x, ", r.r[j]));
			}
			DBG3(printf("]\n"));

			if ((r.r[0] & IC) != 0) {/* Bad seek	*/
				bp->b_error = EIO;

			DBG3(printf("seek err: skretries=%x, ba=%x\n",
				skretries, fdtab.b_active));

				return(FAIL);

			}
#ifdef Mp386
			fdcurcyl[UNIT(fdupfix(bp->b_dev))] = bp->b_cylin;
#else
			fdcurcyl[UNIT(bp->b_dev)] = bp->b_cylin;
#endif
			return(OK);		/* Finished w/seek substate */
			break;

		case LIVE_READID:		/* Finished with READID	*/
			i = fdresults(&r);

			DBG3(printf("readid results: ["));
			for(j=0; j < i; j++) {
				DBG3(printf("%x, ", r.r[j]));
			}
			DBG3(printf("]\n"));

			if ((r.r[0] & IC) != 0) {	/* Bad readid	*/
				bp->b_error = EIO;
				return(FAIL);
			}
			return(OK);
			break;

		default:	/* Not a substate; A normal r/w has occured.*/
				/* Take a gander at the results.	*/

			i = fdresults(&r);	/* get the results*/
 
			DBG3(printf(" r/w results: ["));
		DBG3(printf("st0=%x, st1=%x, st2=%x, C=%x, H=%x, R=%x, N=%x]\n",
		   r.r[0], r.r[1], r.r[2], r.r[3], r.r[4], r.r[5], r.r[6]));

#ifdef M009
/* M009 */	     /* Allow write protect failures if this's the bootdisk */
			if (((r.r[0] & IC) != 0) || ((r.r[1] & BADSR1) != 0)) {
/* M009 */			if (((r.r[1] & NW) && (writeprot == 1)) == 0) {
#else M009
			/* Check for bad xfer */
			if (((r.r[0] & IC) != 0) || ((r.r[1] & BADSR1) != 0)) {
#endif M009
				DBG3(printf("bad xfer, retries=%x\n", retries));
	
				/* test for write protected floppy */
				if (NW & r.r[1])
					break;   /* do not attempt retry on write protect */

				if (--retries < 0 ) {
					bp->b_error = EIO;
					break;
				} else { /* Attempt a retry	*/
					fdtab.b_active &= 0x8E;
					return(OK);
				}
#ifdef M009
			    }
#endif M009
			}

			retries = RWRETRY;	/* re-init next pass*/
			DBG3(printf("X+I: b_e=%x, b_f=%x\n",
				 bp->b_error, bp->b_flags));

			/* 
			 * one xfer is done. update state variables.
			 */

			if (cache.sts & FLCWR) { /* Finish disk update */
				/* here the cache has been successfully
				 * written to the disk. So go back and
				 * do the real request.
				 */
				fdtab.b_active &= 0x0e;
				cache.sts &= ~(FLCDIRTY|FLCWR); /* now clean */
				DBG5(printf("x+i: cachetodisk fini\n"));
				return(OK);	/* another round	*/
			}

			f = ftab[DEV];
			cp = cache.cp;
			head = (bp->b_secno - 1) / f->spt;

			/* finish cache update: */
			if (cache.sts & FLCRD) {
				/* Here we've just finished a multi-track read 
				 * command. Mark the cache as valid, and copy
				 * the appropriate sector over to the buffer.
				 */
				fdtab.b_active &= 0x0e;
				cache.sts &= ~(FLCDIRTY|FLCRD); /* now clean */
				DBG5(printf("x+i: disktocache fini\n"));
				cache.c = bp->b_cylin;
				cache.h = head;
				return(OK);	/* another round	*/
			}

			/* single sector xfer: */
			if ((bp->b_flags & B_READ) == B_READ) {
			    if (altbufflg) {/* alternate buffer*/
#ifdef MP386
				bcopy(altbuf, io_s2, FLBSIZE);
#else
				bcopy(altbuf, mapin(io_s2, WNDMASEL), FLBSIZE);
#endif
				altbufflg = 0;
				DBG15("\nfinished with alt. buffer\n");
			    }
			}

			if (bp->b_resid < FLBSIZE) {
				bp->b_resid = 0;
				io_s1 = 0;
			} else {
				bp->b_resid -= FLBSIZE;
				io_s1 -= FLBSIZE;
			}
			io_s2 += FLBSIZE;
			DBG3(printf("s1=%lx, s2=%lx\n", io_s1, io_s2));
			if (io_s1 == 0) {		/* xfer complete */
			DBG3(printf("fdstart:XFER fini,res=%x, s1=%x\n",
				bp->b_resid, io_s1));
				break;
			} else if (io_s1 < FLBSIZE) {	/*leftovers: complain*/
				bp->b_error = EIO;
				printf(" fdstart: %s err. n=%x\n",
				    (bp->b_flags & B_READ) ? "read" : "write",
				    io_s1);
				break;
			} else {			/* another round */
				fdtab.b_active |= FLMORE;
				if (bp->b_secno == ftab[DEV]->spc) { 
#ifdef M001
				    if (((bp->b_cylin % f->spc) == 0) && (bp->b_cylin != 0))  {
					printf("trackcheck cyl overrun 0!\n");
				    }
#endif /* M001: */
				    bp->b_secno = 0;
				    bp->b_cylin++;
				}
				fdtab.b_active &= 0x8E;
				bp->b_secno++;

				return(OK);
			}

	}
	return(FAIL);		/* not nec. failure; just end */
}

unsigned char
spcmd(bp)
struct buf *bp;
{
	int i, rw;
	struct fdparams *f;
	unsigned long dest;
	unsigned char head, d;
	uint unit;

/*	DBG7(printf("spcmd:")); */
	fdunit = 0;			/* tmp for top unit caching */
	if (fdcmd.cmd == 0) {		/* Special case - await irq */
		fdtab.b_active |= GETSTATUS;
		DBG7(printf("spcmd: GETSTATUS\n"));
		return(OK);
	}
	DBG8(printf("spcmd= %x, dor=%x, flags=%x\n",
			fdcmd.cmd, dorstat, bp->b_flags));

		/*
		 *  M008: return from this state machine to
		 *        begin draining the buffer queue.
		 */
	if (fdcmd.cmd == QDCMD) {
		return(FAIL);
	}

	if (fdcmd.cmd == WRCMD) {			/* w cache to disk? */
		if ((cache.sts & FLCDIRTY) == 0) {	/* cache not dirty */
			DBG8(printf("spcmd: cache not dirty\n"));

			/* here we have a special request to sync the cache
			 * to disk. This only happens on a close, or
			 * when there hasn't been any recent floppy 
			 * activity. If the cache is already in sync,
			 * (i.e. not dirty) then turn off the motor.
			 * Needed for cpio reads, over multivolumes; results
			 * in the floppy light being turned off at the
			 * proper time.
			 */

#ifdef MP386
			unit = UNIT(fdupfix(bp->b_dev));
#else
			unit = UNIT(bp->b_dev);
#endif

#ifdef NOTYET
#ifdef MP386
			fdoff(fdupfix(bp->b_dev));
#else
			fdoff(bp->b_dev);
#endif
#endif /* NOTYET */
			bp->b_flags &= ~B_BUSY;
			return(FAIL);
		}
	}

	if (fdcmd.cmd == FLFORMAT) {	/* format cmd */
		bp->b_flags &= ~B_READ;
		rw = B_WRITE;
#ifdef MP386
		fdrsetdma(rw, kvtophys(fmtbuf),
				(unsigned int) (ftab[UNIT(fdupfix(bp->b_dev))]->spt)*4);
#else
		fdrsetdma(rw, physaddr(fmtbuf),
				(unsigned int) (ftab[UNIT(bp->b_dev)]->spt)*4);
#endif /* MP386 */
	}

	/* if cmd == WRCMD, then we are trying to sync the cache to
	 * the disk. The command parameters are assigned here, rather 
	 * than at a higher level, to insure that the cache doesn't
	 * switch tracks after the command has been issued.
	 */
	if (fdcmd.cmd == WRCMD) {		/* w cache to disk */
		DBG9(printf("spcmd: cache sync, "));

		dorstat &= 0xFE; 		/* make sure unit is active */
#ifdef MP386
      	dorstat |= fdsel[UNIT(fdupfix(bp->b_dev))];
#else
      	dorstat |= fdsel[UNIT(bp->b_dev)];
#endif
		outb(DOR, dorstat);
#ifdef MP386
		outb(DCR, ftab[UNIT(fdupfix(bp->b_dev))]->trrate);
#else
		outb(DCR, ftab[UNIT(bp->b_dev)]->trrate);
#endif

		f = ftab[DEV];
		rw = B_WRITE;
		head = cache.h;
		DBG9(printf("fdrsetdma spt=%x, ", f->spt));
#ifdef MP386
		dest = KVaddr(cache.cp);
		if (fdrsetdma(rw, kvtophys(dest), f->spt*FLBSIZE) == FAIL)
#else
		dest = physaddr(cache.cp);
		if (fdrsetdma(rw, dest, f->spt*FLBSIZE) == FAIL)
#endif /* MP386 */
		{
			printf("floppy sync: set dma failure!\n");
		}
		DBG9(printf("fdrsetdma fini, "));

		fdcmd.p[0] = (head << 2) | DEV;
		fdcmd.p[1] = cache.c;
		fdcmd.p[2] = head;
		fdcmd.p[3] = 1;			/* start a r=1 to EOT	*/
		fdcmd.p[4] = f->nsec;
		fdcmd.p[5] = f->eot;
		fdcmd.p[6] = f->gpl;
		fdcmd.p[7] = f->dtl;
		fdcmd.np = 8;
		fdcmd.ns = 7;
		DBG9(printf("issuing command\n"));
	}

	fdbwait(CPHASE); CMD(fdcmd.cmd); 	/* issue command	*/
	for (i=0; i < fdcmd.np; i++) {
		fdbwait(CPHASE); CMD(fdcmd.p[i]); 
	}

	if (fdcmd.cmd == SDS)  {		/* start M011 */
		i = fdresults(fdcmd.s);
		bp->b_flags &= ~B_BUSY;
		return(FAIL);
	}					/* end M011 */

	/* For cmds that don't return status upon irq:	*/
	if ((fdcmd.cmd == RECALIB) || (fdcmd.cmd == SEEKCMD)) {
		fdtab.b_active |= GETSTATUS;
	}
	DBG9(printf("end FLCMD = %x, flags = %x\n", fdcmd.cmd, bp->b_flags));
	return(OK);
}

spcmdirq(bp)
struct buf *bp;
{
	int i, j;
	struct results r;

	DBG6(printf("spcmdirq: = %x,", fdcmd.cmd));
	if (fdtab.b_active & GETSTATUS) {/* Sense irq status if neccessary */
		fdbwait(CPHASE); CMD(SIS);
	}
	i = fdresults(&r);				/* Get results	*/
	for (j=0; j < i; j++) {
		fdcmd.s[j] = r.r[j];			/* save results */
	}

	if (fdcmd.cmd == WRCMD) {	/* currently servicing a sync */
		DBG6(printf("Cache synced:"));
		cache.sts = 0;
#ifdef NOTYET
		dorstat &= ~DRIVEAE;
		outb(DOR, dorstat);
#ifdef MP386
		fdoff(fdupfix(bp->b_dev));
#else
		fdoff(bp->b_dev);
#endif
#endif /* NOTYET */
	}				/* need to handle retries */
	DBG6(printf("flags =%x\n", bp->b_flags));
	bp->b_flags &= ~B_BUSY;

}

/*
 * fdreset:     Reset the floppy controller.
 * Input:       dev -> which drive is to be reset.
 * Output:      Returns OK if successful, FAIL otherwise.
 */
unsigned char
fdreset(dev)
dev_t dev;
{
     unsigned x, retries;
     unsigned char sts0;
     struct cmd cmd;
	extern struct fdparams *ftab[];
	extern hz, initdor();

	if ((devstat[0] != 0) || (devstat[1] != 0)) {
		DBG14(printf("fdreset: unit already active\n"));
        	return(OK);
	}
	retries = 3;
retry:
	DBG14(printf("fdreset: setting dorstat= %x,\n", dorstat));
        cmd.cmd = 0;                    /* special case - reset FDC     */
        cmd.np = 0;                     /* no initial cmds issued       */
        cmd.ns = 2;                     /* results from SIS             */
        dorstat = EDIDMA|fdsel[UNIT(dev)];
        x = splbio();
        outb(DOR, dorstat);             /* reset the DOR                */
        delay(30);
	timeout(initdor, (caddr_t)0, (unsigned short) 30);

        splx(x);
        fdcommand(&cmd, dev);        	/* await the irq                */
        delay(60);                      /* wait for motor to spin up    */

	DBG14(printf("fdreset: dorstat = %x, s0=%x\n", dorstat, cmd.s[0]));
	sts0 = cmd.s[0] & IC;
        if (sts0 != 0xC0) {  /* check for valid reset*/
	printf("fdreset: dorstat = %x, s0=%x\n", dorstat, cmd.s[0]);
		if (retries-- == 0)
             		return(FAIL);
		else
			goto retry;
        }

	initdma();
        return(OK);
}

/* Initialize the Digital Output Register. This will cause an irq.
 * Must be called only from the timeout queue, so that state machine
 * is ready to handle it.
 */
initdor()
{
       	dorstat |= FLRESET;		/* Reset the drive		*/
       	outb(DOR, dorstat);     	/* init the DOR                 */
}

/*
 * fdspecify:   Specify device parameters.
 */
unsigned char
fdspecify(dev)
dev_t dev;
{
	DBG11(printf("entering fdspecify - sw2=%d", specwrd2));
	if ((devstat[0] != 0) || (devstat[1] != 0)) {
		DBG14(printf("fdreset: unit already specified\n"));
        	return(OK);
	}
	fdbwait(CPHASE); CMD(SPECIFY);
       	fdbwait(CPHASE); CMD(specwrd1);
        fdbwait(CPHASE); CMD(specwrd2);
        return(OK);
}

int
driveaoff()
{
	int x;
	struct buf *bp;

	x = splbio();
	bp = fdtab.b_actf;
#ifdef MP386
	if ((bp == NULL) || (UNIT(fdupfix(bp->b_dev)) != 0))  {
#else
	if ((bp == NULL) || (UNIT(bp->b_dev) != 0))  {
#endif
		if (driveoff(0) == FAIL)  {	/* do it again */
			drvlock[0]++;
			retimeout(0);
			splx(x);
			return;
		}
	}
	drvlock[0] = 0;
	splx(x);
}

int
driveboff()
{
	struct buf *bp;
	int x;

	x = splbio();
	bp = fdtab.b_actf;
#ifdef MP386
	if ((bp == NULL) || (UNIT(fdupfix(bp->b_dev)) != 1))  {
#else
	if ((bp == NULL) || (UNIT(bp->b_dev) != 1))  {
#endif
		if (driveoff(1) == FAIL)  {	/* do it again */
			drvlock[1]++;
			retimeout(1);
			splx(x);
			return;
		}
	}
	drvlock[1] = 0;
	splx(x);
}

int
driveoff(u)
unsigned char u;
{
	unsigned short ccbug;

	DBG8(printf("driveoff: drvlock[%d] = %x,", u, drvlock[u]));

	if (cache.sts & FLCDIRTY)  {    /* turn motor off only when */
		DBG8(printf(" cache dirty\n"));   /* cache is clean */
		return OK;
	}
	
	ccbug = (unsigned short) (fdtimeout*hz);
	if ((lastaccess + ccbug) > lbolt)  {
		DBG8(printf(" not enough time on drive %d\n", u));
		return FAIL;
	}
	u = ((u == 0) ? DRIVEAE : DRIVEBE);
	dorstat &= ~u;			/* disable motor	   */
	outb(DOR, dorstat);
	DBG8(printf(" dorstat = %x\n", dorstat));
	return OK;
}

retimeout(u)
unsigned char u;
{
	extern int driveaoff(), driveboff(), kludgeoff();

	DBG8(printf(" retimeout: drive %d\n", u));
#ifdef	REMOVE
	if (u)  {
		timeout(driveboff, (caddr_t)0, fdtimeout*hz);
	} else  {
		timeout(driveaoff, (caddr_t)0, fdtimeout*hz);
	}
#endif	/* REMOVE */
	if (fdtimeout)					/* M010 */
		timeout(kludgeoff, (caddr_t)0, fdtimeout*2*hz);
}

int
kludgeoff()
{
	driveaoff();
}

/* cachecheck:	searches the current active caches for buffer data.
 *	returns FAIL if the data isn't in memory.
 *	returns OK  if the data is in the cache, and has been xfered.
 */

unsigned char
cachecheck(bp)
struct buf *bp;
{
	unsigned char c, head;
	struct fdparams *f;
	uint i, j;
	unsigned char *cp;
	extern unsigned char docache(), trackcheck();	

	if (fdcache)
		return(FAIL);

	f = ftab[DEV];

	DBG5(printf("cachecheck: "));
	/* we now only handle cached r/w's on 15 spt floppies. */

	if (!docache(f, bp)) {	/* : multitrack check */
		DBG5(printf("FAILed\n"));
		return(FAIL);
	}
	/* now check and see if the buffer is in the active caches:	*/
cachesearch:
	head = (bp->b_secno - 1) / f->spt;
	/* cyl/head in cache? */                      
	if ((cache.c == bp->b_cylin) && (cache.h == head)) {
		DBG5(printf("req in queue: c=%x, h=%x\n", cache.c, cache.h));
			/* req. is in cache. xfer and update state variables.*/

		if ((fdtab.b_active & FLMORE) != FLMORE) { /*req start*/
			io_s1 = bp->b_bcount;	/* bytes to go	*/
#ifdef MP386
			io_s2 = KVaddr(paddr(bp)); /* i/o addr	*/
#else
			io_s2 = bp->b_physaddr;	   /* i/o addr	*/
#endif
			DBG5(printf("cchk: start xfer: io_s2=%lx, ", io_s2));
			DBG5(printf("b_a = %x \n",
                                        (unsigned char) fdtab.b_active));
		}

			/* xfer: */
		cp = &cbuf0[0];
		if ((bp->b_flags & B_READ) == 0 ) { /* write */
			DBG5(printf("cchk: w xfer: s=%x, io_s2=%lx\n",
					bp->b_secno, io_s2));
#ifdef MP386
			bcopy( io_s2, (cp + (((bp->b_secno - 1) %
					f->spt)*FLBSIZE)), FLBSIZE);
#else
			mapin(io_s2, WNDMASEL);
			bcopy( SELTAR, (cp + (((bp->b_secno - 1) %
					f->spt)*FLBSIZE)), FLBSIZE);
#endif /* MP386 */
			cache.sts |= FLCDIRTY;	/* mark as dirty*/
		} else {			/* read	*/
			DBG5(printf("cchk: r xfer: s=%x, io_s2=%lx\n",
					bp->b_secno, io_s2));
#ifdef MP386
			bcopy((cp+(((bp->b_secno-1)%f->spt)*FLBSIZE)),
					io_s2, FLBSIZE);
#else
			mapin(io_s2, WNDMASEL);
			bcopy((cp+(((bp->b_secno-1)%f->spt)*FLBSIZE)),
					SELTAR, FLBSIZE);
#endif /* MP386 */
		}

			/* must handle req's that cross a track boundary */

		if (trackcheck(bp) == OK)	/* M001 */
			return(OK);

		goto cachesearch;
	}

	/* req. not in cache */
	DBG5(printf("req not queue: c=%x, h=%x, ", bp->b_cylin, head));
	DBG5(printf("c.c=%x, c.h=%x\n", cache.c, cache.h));
	return(FAIL);
}

/* Start M001 */
unsigned char
trackcheck(bp)
struct buf *bp;
{
	struct fdparams *f;

	f = ftab[DEV];

	if (bp->b_resid < FLBSIZE) {
		bp->b_resid = 0;
		io_s1 = 0;
	} else {
		bp->b_resid -= FLBSIZE;
		io_s1 -= FLBSIZE;
	}
	io_s2 += FLBSIZE;
	if (io_s1 == 0) {	/* xfer complete	*/
		return(OK);
	} else if (io_s1 < FLBSIZE) {/* leftovers, complain!*/
		bp->b_error = EIO;
		return(OK);	/* end i/o		*/
	} else {		/* more to do		*/
		fdtab.b_active |= FLMORE;
		if (bp->b_secno == f->spc) {
#ifdef M001
			if (((bp->b_cylin % f->spc) == 0)&&(bp->b_cylin != 0)){
				printf("trackcheck cyl overrun!\n");
			}
#endif /* M001: */
			bp->b_secno = 0;
			bp->b_cylin++;
		}
	}
	fdtab.b_active &= 0x8E;
	bp->b_secno++;
	return(FAIL);
}
/* End M001 */

/* fdsetdma:	sets up the 8237 for single-sector & multitrack reads/writes.
 *	Returns OK if the hardware is properly initialized.
 *	Returns FAIL if the address region crosses a 64K boundary.
 */
unsigned char
fdsetdma(bp)
struct buf *bp;
{
	unsigned long dest;
	unsigned int size, rw;
	unsigned char head;
	struct fdparams *f;
	extern unsigned char docache(); 

	f = ftab[DEV];
	rw = bp->b_flags & B_READ;
	if (!docache(f, bp)) {			/* single sector xfer */
		dest = io_s2;
		size = FLBSIZE;
	} else {				/* full track */
#ifdef MP386
		dest = KVaddr(cache.cp);
#else
		dest = physaddr(cache.cp);
#endif
		size = f->spt * FLBSIZE;
		rw = B_READ;
		if (cache.sts & FLCWR) { 
			rw = B_WRITE;		/* cache update in progress */
		}
	}

	DBG4(printf("fdsetdma: dest = %lx, size = %x\n", dest, size));
#ifdef MP386
	if (fdrsetdma(rw, kvtophys(dest), size) == FAIL)
#else
	if (fdrsetdma(rw, dest, size) == FAIL)
#endif
	{
		DBG4(printf("fdsetdma: fdrsetdma failed on write\n"));
		if (!docache(f,bp)) {
		    if ((bp->b_flags & B_READ) == 0) { /* writes - altbuf */
#ifdef MP386
			bcopy(io_s2, altbuf, FLBSIZE);
			if (fdrsetdma(rw, kvtophys(altbuf), FLBSIZE) == FAIL)
#else
			bcopy(mapin(io_s2, WNDMASEL), altbuf, FLBSIZE);
			if (fdrsetdma(rw, physaddr(altbuf), FLBSIZE) == FAIL)
#endif /* MP386 */
			{
				printf("floppy dbl edge dma error\n");
				bp->b_error = EIO;
				return(FAIL);
			} else {
				altbufflg = 1;
			}
		    } else {
			printf("fdsetdma: address error\n");
			return(FAIL);
		    }
		} else {			/* multitrack: quit */
		    printf("fdsetdma: address error\n");
		    return(FAIL);
		}
	}

	return(OK);
}

unsigned char
docache(f, bp)
struct fdparams *f;
struct buf *bp;
{
	if (fdcache)
		return(0);
#ifdef MP386
	return((unsigned char)((f->spt == 15) && (UNIT(fdupfix(bp->b_dev)) == 0)));
#else
	return((unsigned char)((f->spt == 15) && (UNIT(bp->b_dev) == 0)));
#endif
}

/*
 * cacheflush:	put a request to sync the cache with disk on fdtab queue,
 *	if neccessary.
 */

cacheflush(dev)
dev_t dev;
{
	struct cmd cmd;
	int x;
	extern unsigned char nosync();

	x = splbio();
	if (nosync() == OK) {			/* sync not needed */
		splx(x);
		return;
	}

	cache.sts |= FLSYNC;			/* warn others away */
	cmd.cmd = WRCMD;			/* other params set in spcmd*/
	DBG6(printf("cacheflush: calling fdcommand\n"));
	splx(x);
	fdcommand(&cmd, dev);
	DBG6(printf("cacheflush: fini\n"));
}

/*
 * cachesync:	put a request to sync the cache with disk on fdtab queue,
 *	Called from timeout queue
 */

cachesync()
{
	unsigned short ccbug;
	int  x;
	extern unsigned char nosync();

	x = splbio();
	cachewait = 0;			/* allow future que's on failure */

	DBG8(printf("cachesync:"));
	if (nosync() == OK) {		/* sync not needed */
		DBG8(printf(" sync not needed\n"));
		retimeout(0);		/* cache top unit only */
		drvlock[0]++;
		splx(x);
		return;
	}

	/* still activity? */
	if ((fdtab.b_actf != NULL) || (fdtab.b_active != IDLE)) {
		DBG8(printf("\n"));
		splx(x);
		return;
	}

	/* if we've gotten here, but we haven't allotted enough time,
	 * then timeout, and check again later.
	 */
	ccbug = (unsigned short) (cachetimeout*hz);
	if ((lastaccess + ccbug) > lbolt) { 
		cachewait = 1;		/* mask out other timeouts */
		DBG8(printf(" not enough time\n"));
		timeout(cachesync, (caddr_t)0, ccbug);
		splx(x);
		return;
	}

	if (cfdbuf.b_flags & B_BUSY) {
		cachewait = 1;		/* mask out other timeouts */
		DBG8(printf(" cfdbuf busy\n"));
		ccbug = (unsigned short) cachetimeout*hz;
		timeout(cachesync, (caddr_t)0, ccbug);
		splx(x);
		return;
	}

	/* finally, the act_f queue has been quiescent enough to
	 * warrent syncing the cache.
	 */
	DBG6(printf(" calling fdsync\n"));

	tcmd.cmd = WRCMD;			/* other params set in spcmd*/
	tcmd.np = 8;				/* other params set in spcmd*/
	fdsync(&tcmd, lastdev);
	fdstart();
	splx(x);
}

/* fdsync -- queue non-standard commands for execution via cfdbuf
 *	Like fdcommand, but to be run when servicing an interrupt.
 *	shouldn't get here otherwise, or when cfdbuf is in use.
 */
fdsync(cp, dev)
struct cmd *cp;
dev_t dev;
{
	struct buf *bp;

	bp = &cfdbuf;
	bp->b_dev = dev;
	bp->b_flags = B_BUSY;   /* assume exclusive use */

	/* driver's fdcmd structure mutexes using cfdbuf.
	* copy caller's command over.
	*/
	fdcmd = *cp;

	/*  add cfdbuf to end of queue and be sure there's motion. */
	bp->b_cylin = (unsigned int) 0xFFFF;	/* for cyl sorting */
	bp->b_secno = (unsigned int) -1;	/* for sector sorting */
	bp->av_forw = NULL;
	bp->av_back = NULL;
	fdtab.b_actf= bp;
}

/* nosync: returns OK if the cache doesn't need to be sync'd. FAIL otherwise */

unsigned char
nosync()
{
	/* if the cache isn't dirty, don't bother */
	if ((cache.sts & FLCDIRTY) != FLCDIRTY) {
		return(OK);
	}

	/* a sync is currently in progress */
	if (cache.sts & FLSYNC) {
		return(OK);
	}

	/* track caching only on top drive, for now */
	if (UNIT(lastdev) != 0) {
		return(OK);
	}
	return(FAIL);
}

#ifdef ATMERGE
int
flopreset()
{
	fdoff(0);		/* driveaoff(); */
	fdoff(FLUNIT);		/* driveboff(); */
}
#endif /* ATMERGE */
