static char *uportid = "@(#)ctconf.c	Microport Rev Id  1.3.8 1/12/87";
/* Copyright 1987 by Microport. All Rights Reserved.
 *
 * TVI TeleCAT-286 Streaming Tape Device Driver 
 * References:
 *	TVI Streaming Tape Interface Hardware Specifications
 *
 * Initial Coding:
 *		unix-net!doug
 *		unix-net!mark Mon Nov 10 22:19:51 PST 1985
 * Modification History:
 *	M001: uport!doug Thu Jan  8 19:43:00 PST 1987
 *		Write only one file mark at close.
 *	M002: uport!doug Fri Jan  9 10:19:15 PST 1987
 *		Add IOCTL for streaming support.
 *	M003: uport!doug Thu Jan 15 12:23:48 PST 1987
 *		Add IOCTL for driver malloc of large buffer.
 *	M004: uport!doug Thu Jan 15 20:10:41 PST 1987
 *		Do not set bp->b_resid if not an error.
 *	M005: uport!doug Fri Jan 16 09:55:19 PST 1987
 *		Add kernel buffering for fast device.
 *	M006: uport!doug Sun Jan 18 14:46:12 PST 1987
 *		Disallow reads after write or write after read.
 *	M007: uport!doug Fri Feb 27 19:09:22 PST 1987
 *		Zero tcb.tc_stat[0] on open to clear rfm. 
 * To do:
 *	1) incorporate sharing the 8237 dma channels with other drivers
 */

/*
 *
 * Televideo Cartridge Tape Driver
 * 
 */
#include "stand.h"
#include "sys/signal.h"
#include "sys/sysmacros.h"
#include "sys/systm.h"
#include "sys/buf.h"
#include "sys/iobuf.h"
#include "sys/conf.h"
#include "sys/proc.h"
#include "sys/dir.h"
#include "sys/user.h"
#include "sys/utsname.h"
#include "sys/elog.h"
#include "sys/erec.h"
#include "sys/trap.h"
#include "sys/mmu.h"
#include "sys/seg.h"
#include "sys/8237.h"
#include "sys/8259.h"
#include "sys/map.h"
#include "sys/ioctl.h"
#define TVIQIC
#ifdef TVIQIC
#include "qic2tvi.h"
#endif TVIQIC

/* Please include any device dependant header files above the include of ct.h */
#include "ct.h"
#include "ctioctl.h"

/* events */
#define EDNRDY	0	                /* drive not ready		*/
#define	EERROR	1	                     
#define	EERRBOT	2	                    
#define	EERRSKP	3	                   
#define ELT	4	                /* wrong position		*/
#define EGT	5	                /* wrong position		*/
#define EEOT	6	                /* end of track		 	*/
#define	EBOT	7	                /* beginning of track	 	*/
#define EEQ	8	                /* same postion		 	*/
#define EINIT	9	                /* initial state for command	*/
#define EABORT	10	                /* retries exhausted		*/
extern uint malloc();
extern long umapin();
extern long mapin();
extern paddr_t physaddr();			/* the routine to convert    */
extern uint devstat[];				/* floppy! device status     */
extern hz;

#ifdef CT
/* tape driver externals */
extern unsigned char tapeio(), tapeioirq();	/* initiate & handle R/W */
extern unsigned char ctreset();
#endif

struct buf cctbuf;	                             /* buf header for queue */
struct buf rctbuf;	                        /* buf header for raw device */
struct buf fstbuf;	                       /* buf header for fast device */
struct iobuf cttab;	  /* required unix block device buffer state */
struct tcb tcb;			/* tape control block 			*/
uint altbuflg = 0;
int cthdpos = BOT;	                    /* current head position on tape */
int cteoftp;	/* reached end of tape flag */
int cteof;	/* reached end of file flag */
int ctdbg = CTDBGLVL;		        /* patchable debug level  */
int ctdbglvl = 0;		/* determines printf's		*/
char ctopenf = 0;
char ctflags = 0;
int lkflg = 0;			/* true if process locked in memory */
unsigned char dmachannel = CTDMACH;		/* movable so that jumpers match*/

long altbufsize = ALTBUFSZ;		/* force altbuf to be aligned   */
unsigned char altbuf[ALTBUFSZ];	/* alt. for dma rollovers & addr > 1MB  */
daddr_t ctblk = 0;

static int openwait=0;
static int ctinitted = 0;

/* large streaming tape buffer management */
int ctbs = 0;		/* buffer size clicks */
int ctba = 0;		/* buffer base clicks */
int ctbtop;		/* buffer top clicks */
int ctbin;		/* position in buffer for next input (clicks)*/
int ctbout;		/* position in buffer for next output (clicks)*/


/* open cartridge tape */
CTOPEN(dev, flag)
dev_t dev;
int flag;
{
    int x;
    register struct buf *bp;

    if (ISDEBUG (dev))
	{
	ctdbglvl = ctdbg;			/* set it to default */
	DBG5( printf("ctopen \n"));
	}
    else
	{
	ctdbglvl = 0;
	}
    DBG0(printf("\nctdbglvl = %d\n",ctdbglvl));
    DBG0(printf("Debug level 0 \n"));
    DBG1(printf("Debug level 1 \n"));
    DBG2(printf("Debug level 2 \n"));
    DBG3(printf("Debug level 3 \n"));
    DBG4(printf("Debug level 4 \n"));
    DBG5(printf("Debug level 5 \n"));
    DBG6(printf("Debug level 6 \n"));
    DBG7(printf("Debug level 7 \n"));
    DBG8(printf("Debug level 8 \n"));
    DBG9(printf("Debug level 9 \n"));
    DBG10(printf("Debug level 10 \n"));
    DBG11(printf("Debug level 11 \n"));
    DBG12(printf("Debug level 12 \n"));
    DBG13(printf("Debug level 13 \n"));
    DBG14(printf("Debug level 14 \n"));
    DBG15(printf("Debug level 15 \n"));

    u.u_error = 0;
    if (ctopenf)
	if (RESET (dev))
	    {
	    x = splbio();
#ifdef CT
	    DBG5(printf("OPEN RESET: rctbuf %lx, cctbuf %lx fstbuf %lx bp %lx cttab.b_active %x, flags %x\n",
		(unsigned long)&rctbuf, (unsigned long)&cctbuf, (unsigned long)&fstbuf,
		(unsigned long)bp, cttab.b_active, bp->b_flags));
	    if (cttab.b_active & (TAPEIO|CTCMD))
		{
		bp = cttab.b_actf;
		cteio(bp);			/* set EIO error */
		cttab.b_active |= IRQ;	/* simulate interrupt */
		DBG0(printf("OPEN lost int: rctbuf %lx, cctbuf %lx fstbuf %lx bp %lx cttab.b_active %x, flags %x\n",
		    (unsigned long)&rctbuf, (unsigned long)&cctbuf, (unsigned long)&fstbuf,
		    (unsigned long)bp, cttab.b_active, bp->b_flags));
/*		cttimer(CTDELAY);*/	/* debug */
		ctstart();
    /* also we might try adding CANCEL bit to active
     * flag and using that to not read residual in 
     * tapeioirq().
     * perhap's we could or should do a chkrdstatus(cttab.b_actf)
     * we might have to set b_error.
     */
/*		cttimer(CTDELAY);*/	
		}
#endif CT
	    if (ctbs)
		{
#ifdef CT
		if (cttab.b_active)	/* if possible I/O inprogress */
		    {
		    splx(x);
		    DEVRESET();		/* reset before freeing */
		    x = splbio();
		    }
#endif CT
		ctfree();		/* free kernel buffer */
		}
	    if (lkflg)			/* unlock process */
		{
		u.u_procp->p_flag &= ~SLOCK;
		lkflg = 0;
		}
	    if (cttab.b_active)		/* clear any pending I/Os */
		clrqueue(cttab.b_actf);
	    ctopenf = 0;
	    splx(x);
	    }
	else
	    {	/* only one open at a time */
	    u.u_error = EACCES;
	    DBG0(printf("ctopen: already opened. \n")); 
	    return;
	    }
    ctopenf++;

    /* initialize tape control block values */
    fstbuf.av_forw = 0;
    fstbuf.av_back = 0;
    fstbuf.b_flags = B_TAPE;
    rctbuf.av_forw = 0;
    rctbuf.av_back = 0;
    rctbuf.b_flags = B_TAPE;
    cctbuf.av_forw = 0;
    cctbuf.av_back = 0;
    cctbuf.b_flags = B_TAPE;
    cttab.b_dev = dev;				/* M005 */
    cttab.b_active = IDLE;
    cttab.b_actf = 0;
    cttab.b_flags = B_TAPE;
    tcb.tc_stat[0] = 0;				/* M007 */

    if (!ctinitted || (RESET (dev))) 
	{

#ifdef CT
	x= spl7();
	/* initialize the drive */
	DEVINIT();
	DBG6( printf("ctopen : finished initialization\n"));
/*	cttimer(CTDELAY);*/	
	splx(x);

	/* reset the drive */
	DEVRESET();
	DBG6( printf("ctopen : finished reset\n"));
/*	cttimer(CTDELAY);*/	
#endif
	if (ctbs)
	    ctfree();			/* free buffer, if any */
	u.u_error = 0;			/* reset after getting status */

	/* initialize dma */
	if (!ctinitted && (devstat[0] == 0) && (devstat[1] == 0))
	    /* uninitialized dma */
	    {
	    /* WARNING: currently the floppy doesn't know about us! */
	    initdma();
	    DBG4( printf("ctopen : initialized the dma\n"));
	    }
	ctinitted++;
#ifdef CT
	ctcommand(dev, CTSD);
	DBG6( printf("ctopen: finished select drive \n"));
	if (u.u_error != 0) 
	    {
	    DBG0( printf("ctopen: select error \n"));
	    goto errexit;
	    }
#endif
	}

    /* retension tape */
#ifdef CT
    if (RETENS (dev)) 
	{
	ctcommand(dev, CTPRW);	
	if (u.u_error != 0) 
	    {
	    DBG0( printf("ctopen: retension \n"));
	    goto errexit;
	    }
	}
#endif CT

    /* fast streaming tape */
    if (ISFAST (dev)) 
	{
#ifndef CT
	if (!ctba && !RESET(dev))		/* only malloc 1st time */
	    {
#endif
	    if (!ctmalloc())			/* malloc buffer */
		{
		u.u_error = ENOMEM;
		DBG0( printf("ctopen: malloc failed\n"));
		}
	    ctbin = ctbout = ctba;
	    ctbtop = ctba + ctbs;
	    DBG3( if (ctba) printf("ctopen: ctba %x, ctbs %x, ctbtop %x\n",ctba, ctbs, ctbtop));
	    fstbuf.b_flags = B_DONE;		/* indicates not busy */
#ifndef CT
	    }
#endif
	    lkflg = (u.u_procp->p_flag&(SLOCK|SSYS))?0:SLOCK;
	    u.u_procp->p_flag |= lkflg;	/* lock process in memory */
	    if (!lkflg)
		{
		DBG0( printf("ctopen: SLOCK not set\n")); 
		}
	}
#ifndef CT
    else
	{
	u.u_error = ENXIO;
	DBG0( printf("ctopen: Not FAST minor dev\n"));
	goto errexit;
	}
#endif
    ctflags = 0;
#ifdef CT
    /* erase tape */
    if (ISERASE (dev)) 
	{
	ctcommand(dev, CTERA);	
	if (u.u_error != 0) 
	    {
	    DBG0( printf("ctopen: erase err\n"));
	    goto errexit;
	    }
	}
#endif CT

    cteoftp = cteof = 0;	               /* not at end of file or tape */
    ctblk=0L;

    if (u.u_error != 0) 
	{
errexit:
	DBG0( printf("ctopen: error # %x \n",u.u_error));
	qc_statdump();
	ctopenf = 0;
	ctinitted = 0;
	}
    DBG5( printf("ctopen: exiting \n"));
}



CTCLOSE(dev, flag)
dev_t dev;
int flag;
{
    register struct buf *bp;
    int i;

    DBG5( printf("ctclose \n"));
    if (ctopenf == 0) 
	{
	DBG0(printf ("ctclose: not opened\n"));
	u.u_error = EACCES;
	return;
	}
#ifdef CT
    if ((ctba) && (flag&F_WRITE) && (ctbin > ctbout)) 
	{
	while (fstbuf.b_flags & B_BUSY)
	    {
	    DBG2(printf ("ctclose: delaying %x\n",CTDELAY*hz));
	    delay(CTDELAY*hz);
	    DBG2(printf ("ctclose: delay done\n"));
	    }
	rw_buffer(B_WRITE, ctbin-ctbout);	/* flush buffer */
	}
    if ((flag&F_WRITE)  &&  (ctflags&T_WASWRIT)) 
	{
	if (!cteoftp)
	    {
	    ctcommand (dev, CTWFM);	/* put 1st file mark if not at eot */
	    /* ctcommand (dev, CTWFM);	put 2nd file mark if not at eot */
	    DBG7( printf("ctclose : finished write\n"));
	    if (u.u_error != 0) 
		{
		DBG0( printf("ctclose: wrt File mk \n"));
		ctopenf = 0;
		cteoftp = 0;
		ctinitted = 0;
		return;
		}
	    }
	}
    if (ctbs)
	{
	if (cttab.b_active)
	    {
	    DBG0(printf("ctclose: wait for I/O \n"));
	    while (cttab.b_active); /* can't free mem until I/O completion*/
		{
		DBG0(printf ("ctclose: delaying %x\n",CTDELAY*hz));
		delay(CTDELAY*hz);
		DBG2(printf ("ctclose: delay done\n"));
		}
	    }
	ctfree();		/* free kernel buffer */
	}
#endif CT
    if (lkflg)
	{
	u.u_procp->p_flag &= ~SLOCK;
	lkflg = 0;
	}
#ifdef CT
    if (cteoftp) 
	{
	ctinitted = 0;
	}
    else if (REWIND (dev)) 
	{	
	/* reset the controller before doing rewind after a R/W cmd */
	if ((ctflags&(T_WASREAD|T_WASWRIT|T_WASFM))) 
	    {
	    ctflags = 0;		/* clear state */
	    /* reset the controller when finished with R/W cmd */
	    DEVRESET();			/* note this sets u.u_error flag */
	    ctinitted = 0;		/* after reset must reinit */
	    u.u_error = 0;		/* must clear after getting status */
	    DBG6( printf("ctclose: tape reset\n"));
	    ctcommand(dev, CTSD);
	    }
	DBG6( printf("ctclose: rewinding \n"));
	ctcommand (dev, CTBOT);
	qc_statdump();
	if (u.u_error != 0) 
	    {
	    DBG0( printf("ctclose: rewind err\n"));
	    qc_statdump();
	    ctinitted = 0;
	    }
	tcb.tc_trkno = TRK1;
	cthdpos = BOT;
	}
#endif CT
    cteoftp = 0;
    ctopenf = 0;
    if (u.u_error != 0) 
	DBG0( printf("ctclose: exit err# %x \n",u.u_error));
}

#ifdef CT
ctcommand (dev, cmd)
dev_t dev;
unsigned char cmd;
{
    register struct buf *bp;

    DBG7( printf("ctcommand dev: %x cmd: %x\n",dev,cmd));
    DBG7( printf("ctcommand: error %x flags %x \n",(int)cctbuf.b_error,cctbuf.b_flags));
    bp = &cctbuf;
    bp->b_flags = B_TAPE; 
    bp->b_bcount = (uint) cmd;  /* bp == &cctbuf => b_bcount = tape cmd */
    u.u_error =0;
    ctrequest(dev,bp);
}


ctrequest (dev, bp)
dev_t dev;
register struct buf *bp;
{
    int x;

    DBG7( printf("ctrequest dev: %x bp: %lx flags %x b_error %x\n",
	dev,(unsigned long)bp,bp->b_flags,bp->b_error));
    bp->b_dev = dev;
    while (bp->b_flags & B_BUSY) 
	{
	bp->b_flags |= B_WANTED;
	DBG0( printf("ctrequest: sleeping \n"));
	sleep ((caddr_t)bp, PRIBIO);
	bp->b_flags &= ~B_WANTED;		/* turn it off */
	}
    x=splbio();
    bp->b_flags |= B_BUSY|B_TAPE; 
    ctqueueio(bp);
    splx(x);
    if (u.u_error)
	{
	DBG0( printf("ctrequest: err b4 iowait %x\n",(int)u.u_error));
	}
    iowait( bp );
    if (u.u_error)
	{
	DBG0( printf("ctrequest: err after iowait %x\n",(int)u.u_error));
	}
    DBG9( dbgprt("ctrequest",bp)); 
    if (bp->b_flags & B_WANTED)
	wakeup ((caddr_t)bp);
/*    else*/	/* M00n */
/*	bp->b_flags = B_TAPE;*/	/* M00n */
}

/* ctbuffer - read write tape from malloc'd coremap buffer 
 *	      for internal use, does not do an iowait()
 * input:  bp->buf containing request (this MUST be &fstbuf!!!)
 * output: queues request
 * returns: none
 */
ctbuffer (bp)
register struct buf *bp;
{

    /*    bp = &fstbuf;	 fast device buffer service requests */
    DBG2( printf("ctbuffer: bp %lx, error %x flags %x \n",
	(unsigned long) bp,bp->b_error, bp->b_flags));
    while (bp->b_flags & B_BUSY) 
	{
	bp->b_flags |= B_WANTED;
	DBG0( printf("ctbuffer: sleeping \n"));
	sleep ((caddr_t)bp, PRIBIO);
	bp->b_flags &= ~B_WANTED;
	}
    bp->b_dev = cttab.b_dev;
    bp->b_flags |= B_BUSY|B_TAPE; 
    ctqueueio(bp);
    if (bp->b_error )
	{
	DBG0( printf("ctbuffer: ctqueueio() err=%x\n",(int)bp->b_error));
	DBG0( dbgprt("ctbuffer",bp));
	}
    if (bp->b_flags & B_WANTED)
	wakeup ((caddr_t)bp);
/*    else*/	/* M00n */
/*	bp->b_flags = B_TAPE;*/	/* M00n */
#endif CT
    DBG2( dbgprt("ctbuffer exit",bp));
}

CTSTRATEGY (bp)
register struct buf *bp;
{
    int x,i;
    char *buffp;

    DBG5( printf("ctstrategy \n"));
/*    bp->b_resid = bp->b_bcount;*/		/* M004 */
      bp->b_resid = 0;				/* M004 */
/*    if (bp != &cctbuf) */			/* M005 */
    if (cteoftp)
	{
	DBG0(printf("ctstrategy: I/O past eot\n"));
	goto errexit;
	}
    if (bp->b_bcount & BLOCKMASK)
	{
	DBG0(printf("ctstrategy: bcount (%x) not a multiple of %x\n",
	    bp->b_bcount, BLOCKSZ)); 
	goto errexit;
	}
    DBG3(printf("ctstrategy b_blkno: %lx block: %lx\n",
	bp->b_blkno,ctblk));
    bp->b_physaddr = physaddr( bp->b_un.b_addr );
    DBG3(dbgprt("ctstrategy",bp));
    DBG3(printf ("ctstrategy: physaddr=%lx\n", (unsigned long)bp->b_physaddr));
    if (bp->b_blkno < ctblk)
	{
	printf("ctstrategy: block out of sequence b_blkno: %lx ctblk: %lx\n",
	    bp->b_blkno, ctblk);
	}
    /* diagnostic dump of write */
#ifdef CTDDUMP
    DBG10(if (!(bp->b_flags & B_READ))	
	{
	buffp = (char *)mapin (bp->b_physaddr,BUFSEL);
	printf ("ctsrategy: buffp = %lx\n",(unsigned long)buffp);
	for (i=0;i<=16;i++)printf ("%x ",(uint*)buffp++);
	printf ("\n");
	});
#endif

#ifdef CT
    if(((bp->b_flags & B_READ)==B_READ))	/* M005 */
	{ 
	if ((ctflags&(T_WASWRIT))) 
	    {
	    DBG0(printf ("Cannot read after writing tape.\n"));
	    goto errexit;
	    }
	}
    else if ((ctflags&(T_WASREAD))) 		/* M005 */
	    {
	    DBG0(printf ("Cannot write after reading tape.\n"));
	    goto errexit;
	    }
#endif
    if (ctba)
	{
	if (ctsatisfied(bp))		/* satisfied from buffer so no need */
	    {				/* to queue this one */
	    DBG5( printf("ctstrategy: ctsatisfied, so do iodone \n"));
	    if (bp->b_error)
		{
		bp->b_flags |= B_ERROR;
		DBG0(printf("ctstrategy: iodone err %x\n",bp->b_error));
		}
	    iodone (bp);
	    return;
	    }
	else
	    {
	    bp->b_bcount = bp->b_resid; /* a phys I/O must be out so lets
					   queue this one behind it */
	    bp->b_resid = 0;		/* reset as may not be an error */
	    DBG2(printf("ctstrategy: ctsatisfied returned false \n"));
	    }
	}
#ifdef CT
    ctqueueio(bp);		/* add it to the list of iobufs */
#endif

    if (u.u_error ) 
	{
errexit:
	cteio(bp);			/* set EIO error */
	if (!bp->b_resid)
	    bp->b_resid = bp->b_bcount;
	ctlock("ctstrategy exit",bp);
	iodone (bp);
#ifdef CT
	cttab.b_actf = bp->av_forw;	/* advance to next I/O */
#endif
	}
    DBG5( printf("ctstrategy: exiting \n"));
}

ctqueueio(bp)
register struct buf *bp;
{
    int x;
    register struct buf *ap;

    bp->b_flags &= ~B_ERROR;
    bp->b_error = 0;
    bp->av_forw = NULL;
    DBG9( printf("ctqueueio: b_active = %x \n",cttab.b_active));

    /* insert I/O request at end of list */
    x=splbio();
    bp->av_forw = NULL;
    if (cttab.b_actf == NULL)
	cttab.b_actf = bp;
    else
	{
	ap = cttab.b_actf;
	while (ap->av_forw != NULL)
	    ap = ap->av_forw;
	ap->av_forw = bp;
	}
    if (cttab.b_active == IDLE)
	ctstart();
    else
	{
	DBG9( printf("ctqueueio: (not calling start)b_active=%x\n",(int)cttab.b_active));
	qc_statdump();
	}
    splx(x);
}

ctstart()
{
    register struct buf *bp;

    DBG5( printf("ctstart \n"));
    /*
     * is there anything
     * to do?
     */
#ifdef CT
top:
    if ((bp = cttab.b_actf) == NULL)
	{
	DBG9( printf("ctstart: no bp, active = %x \n",cttab.b_active));
	return;
	}
    DBG9( printf("ctstart: bp = %lx active = %x \n",bp, cttab.b_active));
    /* 
     * tape not open, clear queue 
     */
    if (ctopenf == 0) 
	{
	clrqueue(bp);
	DBG0(printf("ctstart: not open/no bp, active = %x \n",cttab.b_active));
	return;
	}
newstate:
    switch(cttab.b_active & (CTCMD|TAPEIO|IRQ)) 
	{
	case IDLE:				/* bp references a new req */
	    cttab.b_active |=  (bp == &cctbuf) ? CTCMD : TAPEIO;
	    if (bp==&fstbuf)
		cttab.b_active |= BUFIO;  	/* service our buffer */
	    goto newstate;

	case TAPEIO:				/* a read/write req.	*/
	    DBG9(
		printf("ctstart: b_bcount: %x, b_blkno: %lx block: %lx\n",
		bp->b_bcount,bp->b_blkno,ctblk);
	        if (bp->b_blkno < ctblk)
		    {
		    printf("ctstart: blks out of order b_blkno: %lx ctblk: %lx\n",
			bp->b_blkno,ctblk);
		    }
		);
	    if (tapeio(bp) == OK) 
		{		/* issue proper command	*/
		return;			/* Wait for irq		*/
		}
	    goto endstate;			/* Finished with req.	*/
	case TAPEIO+IRQ:/* a transfer irq: get status, decide what to do*/
	    ctblk = bp->b_blkno;		/* check next for sequence */
	    if (tapeioirq(bp) == OK) 
		{
		goto newstate;
		}
	    break;

	case CTCMD:		/* ctcommand() put cmd in bp->b_bcount */
	    qc_cmd((unsigned char)bp->b_bcount); 
	    return;				/* wait for irq		*/

	case CTCMD+IRQ:				       /* irq after cmd */
	    if (tapeioirq(bp) == OK) 
		{
		return;
		}
	    break;

	default:
	    printf("ctstart: unexpected state %x\n", cttab.b_active);
	    return;
	}
endstate:
DBG9(printf("ctstart: cttab.b_active=%x, bp->b_error=%x\n",cttab.b_active,
     bp->b_error));
DBG9(printf("ctstart: endstate = %x \n",cttab.b_active));
    /*
     * break from the main switch only to finish a request. If
     * there was an error, b_error has been set to some nonzero
     * error code, and we should set B_ERROR. In all cases, call
     * iodone(), and advance the queue.
     */
    if (bp->b_error != 0) 
	{
	bp->b_flags |= B_ERROR;
	DBG0(printf("ctstart: bp->b_flags=%x, bp->b_error=%x\n",bp->b_flags,bp->b_error));
	}


    if (!(cttab.b_active&BUFIO))	/* if not our own, then */
	iodone(bp);			/* release completed request */
    else
	{
	bp->b_flags = B_DONE;		/* indicate fstbuf is idle */
	/* bp->b_flags &= ~(B_WANTED|B_BUSY|B_ASYNC|B_AGE); */
	}
    cttab.b_actf = bp->av_forw;		/* get next I/O request */
    cttab.b_active = IDLE;
    DBG9(printf("ctstart: io done, new bp=%x, b_active=%x\n",cttab.b_actf,cttab.b_active));
    goto top;
    /* ain't nothin gonna execute past here */
#endif CT
}

CTREAD (dev)
dev_t dev;
{
    DBG5( printf("ctread \n"));
    physio (CTSTRATEGY, &rctbuf, dev, B_READ);
    ctcopy(B_READ);
    ctlock("ctread exit",&rctbuf);
}

CTWRITE (dev)
dev_t dev;
{
    DBG5( printf("ctwrite \n"));
    physio (CTSTRATEGY, &rctbuf, dev, B_WRITE);
    ctcopy(B_WRITE);
    ctlock("ctwrite exit",&rctbuf);
}

/* ioctl functions

	DEBUG_LVL:	set the debug level for debug printfs
			input: 	 bp->b_flags = new debug level (int)
				(see ctioctl.h for debug levels)
			returns: none

   The following functions all assume the ioctl arg (bp) to  be a 
   pointer to a buf structure setup by the user process as if he 
   were calling ctstrategy.


	CT_LOCK:	lock the user process in memory
			input: 	 bp-> buf structure
			returns: u.u_error set if process was not locked

	CT_READ:	read b_bcount blocks (clicks) into b_bun.b_addr
			input: 	 bp->b_un.b_addr
				 bp->b_bcount (clicks)
			returns: bp->b_resid (residual click count)
				 u.u_error set if error occurred

	CT_WRITE:	write b_bcount clicks from b_bun.b_addr
			input: 	 bp->b_un.b_addr
				 bp->b_bcount (clicks count)
			returns: bp->b_resid (residual clicks)
				 u.u_error set if error occurred
*/
CTIOCTL(dev, cmd, arg, flag)
dev_t	dev;
int	cmd;
union ioctl_arg arg;
int	flag;
{
    uint 		cnt;
    unsigned long	count;
    uint 		clicknum;
    unsigned char	dmamode;
    register struct buf ibuf;
    
    u.u_error =0;

    if (copyin(arg.sparg, &ibuf, sizeof(struct buf)) == -1) 
	{
	u.u_error = EFAULT;
	return;
	}
    DBG7( printf("ctioctl dev: %x cmd: %x sparg: %lx flag: %x\n",dev,cmd,
	(unsigned long)arg.sparg,flag));

    switch(cmd) 
	{
	case 0:
	    break;

	case DEBUG_LVL:
	    DBG7( printf("ctioctl: Debug level was %x, set to %x\n",
		ctdbglvl,ibuf.b_flags)); 
	    ctdbglvl = ibuf.b_flags;	/* pick up new debug level */
	    break;

	case CT_LOCK:			/* prevent process from being swapped */
	    lkflg = (u.u_procp->p_flag&(SLOCK|SSYS))?0:SLOCK;
	    u.u_procp->p_flag |= lkflg;
	    if (!lkflg)
	    {
		u.u_error = ENXIO;
		DBG0( printf("ctioctl: SLOCK not set, u_error %x\n",
		    u.u_error));
	    }
	    break;

#ifdef CT
	case CT_READ:
	    ibuf.b_flags = B_READ|B_CLICKIO|B_TAPE;
	    goto ioctl_io;
	    break;

	case CT_WRITE:
	    ibuf.b_flags = B_WRITE|B_CLICKIO|B_TAPE;
ioctl_io:
	    ibuf.b_physaddr = physaddr( ibuf.b_un.b_addr );
	    DBG3(printf("ctioctl b_flags: %x, b_addr: %lx ",ibuf.b_flags,
		(unsigned long)ibuf.b_un.b_addr));
	    DBG3(printf (" physaddr = %lx\n", (unsigned long)ibuf.b_physaddr));
/*	    ctrequest(dev,&ibuf);*/	/* M00n */
	    break;
#endif CT

	default:
	    break;
	}
    ctlock("ctioctl: CTIOCTL exit",&ibuf);
}

/* clrqueue - clear all pending I/O requests */
clrqueue(bp)
register struct buf *bp;
{

	while (bp != NULL) 
	    {
	    cteio(bp);			/* set EIO error */
	    DBG0(printf("clrqueue: clear bp %lx, iodone with error %x\n",
		bp, bp->b_error));
	    cttab.b_actf = bp->av_forw;
	    bp->b_flags &= ~(B_WANTED|B_BUSY|B_ASYNC|B_AGE);
	    iodone(bp);
	    cttab.b_active = IDLE;
	    bp = cttab.b_actf = bp->av_forw;
	    }
}

/* ctmalloc - allocate streaming tape buffer */
ctmalloc()
{
    register struct map *bp;
    register struct map *mp;
    static selnum = -1;
    char *ret_val;
    extern struct seg_desc gdt[];
    /* offsets in gdt of selectors */
    extern struct seg_desc vall_sel[], vall_selend[]; 
    unsigned memsize;
    int i, j;
    unsigned long hunk, phys_mem;


    /* free anything already grabbed */
    if (ctba) ctfree();
    /* find largest piece */
    mp=coremap;
#define GETBIGONE
#ifdef GETBIGONE
    for (bp = mapstart(mp); bp->m_size; bp++) 
	{
	ctbs = (bp->m_size > ctbs) ? bp->m_size : ctbs;
	}
#else
    ctbs = 0x60;
#endif
    ctbs = (ctbs > CTMAXBUF) ? CTMAXBUF : ctbs;
    DBG3(printf("ctmalloc: calling malloc for ctbs=%x\n",ctbs));
    ctba = malloc(coremap,ctbs);
    DBG3(printf("ctmalloc: ctba=%x ctbs=%x\n",ctba,ctbs));
    return(ctba);
}

/* ctfree - release streaming tape buffer */
ctfree()
{

    DBG3(printf("ctfree: calling mfree ctbs=%x,ctba=%x\n",ctbs,ctba));
    mfree(coremap,ctbs,ctba);
    ctbs = ctba = 0;
    DBG3(printf("ctfree: freed streaming tape buffer\n"));
}

/* ctlock - lock process in memory (hopefully not reset by system) 
 *	  - error diagnostics
 */
ctlock(s,bp)
char *s;
register struct buf *bp;
{
    if (u.u_error)
	DBG0(printf("%s: active %x resid %x bflag %x berror %x u.u_error %x bcount %x\n",
	s,cttab.b_active, bp->b_resid,bp->b_flags, bp->b_error, u.u_error, bp->b_bcount));
    if (lkflg)
	{
	if (!(u.u_procp->p_flag&SLOCK))
	    {
	    lkflg = (u.u_procp->p_flag&(SLOCK|SSYS))?0:SLOCK;
	    u.u_procp->p_flag |= lkflg;	/* lock process in memory */
	    DBG5( printf("%s: Found SLOCK turned off (lkflg=%x)\n",
		s,lkflg));
	    }
	}
}

cttimer(cnt)
uint cnt;
{
    uint x;

    x = 1000;
    DBG0(
	while(cnt--)
	    {
	    while(x--);
	    }
    );
}

#ifdef CT
ctcopy(mode)
int mode;
{

    if ((rctbuf.b_error == EFAULT) && (ctba))
	{
	rctbuf.b_error = 0;
	rctbuf.b_flags = B_BUSY | mode;
	rctbuf.b_un.b_addr += (rctbuf.b_bcount - rctbuf.b_resid);
	rctbuf.b_bcount = rctbuf.b_resid;
	rctbuf.b_resid = 0;
	ctsatisfied(&rctbuf);	/* copy now as user process is running */
	u.u_count = rctbuf.b_resid;
	u.u_error = rctbuf.b_error;
	rctbuf.b_flags = B_DONE;
	}
}

unsigned char tinb(port)
uint port;
{
    unsigned char byte;

    byte = inb(port);
    printf("inb: port %x returning: %x\n",(unsigned char)port,(unsigned char)byte);
    return(byte);
}

toutb(port,byte)
uint port;
unsigned char byte;
{
    printf("outb: port %x byte %x\n",(unsigned char)port,(unsigned char)byte);
    outb(port,byte);
}
#endif CT

