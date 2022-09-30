/* uportid = "@(#)ctint.c	Microport Rev Id  1.3.8 1/9/87" */
/*
 *
 * Televideo Cartridge Tape Interrupt Routine
 * 
 * Copyright 1987 by Microport. All Rights Reserved.
 *
 * TVI TeleCAT-286 Streaming Tape Device Driver 
 * References:
 *	TVI Streaming Tape Interface Hardware Specifications
 *
 * Initial Coding:
 *		unix-net!doug
 *		unix-net!mark Mon Nov 10 22:19:51 PST 1985
 * Modification History:
 *	M001: uport!doug & mark Thu Jan 15 19:49:48 PST 1987
 *		Read residual correctly from DMA (_ctresdma()).
 *		Handle bp->b_resid correctly. (Don't set unless error.)
 *	M002: uport!doug Fri Jan 16 20:54:18 PST 1987
 *		move _ctsetdma & _ctresdma to 8237.c
 *	M003: uport!doug Sat Jan 17 10:21:12 PST 1987
 *		Kernel malloc'd buffer management
 *	M004: uport!doug Tue Feb 24 18:57:50 PST 1987
 *		Added T_WASFM state indicating we read a file mark.
 *
 * To do:
 *	1) Buffer management for performance option.  M003
 *	2) incorporate sharing the 8237 dma channels with other drivers
 *	   _ctsetdma() & _ctresdma() should be modified to use tables 
 *	   of channel specific masks and addresses and placed in 8237.c 
 *	   as setdma() and resdma(), respectively.  M002
 */

#include "sys/types.h"
#include "sys/sysmacros.h"
#include "sys/param.h"
#include "sys/errno.h"
#include "sys/systm.h"
#include "sys/iobuf.h"
#include "sys/buf.h"
#include "sys/conf.h"
#include "a.out.h"
#include "sys/signal.h"
#include "sys/dir.h"
#include "sys/user.h"
#include "sys/mmu.h"
#include "sys/8237.h"
#include "sys/8259.h"
#define TVIQIC
#ifdef TVIQIC
#include "qic2tvi.h"
#endif TVIQIC
/* Please include any device dependant header files above the include of ct.h */
#include "ct.h"
#include "ctioctl.h"

/* dma address selector setup functions */
extern paddr_t physaddr();
extern caddr_t mapin();
extern caddr_t umapin();		/* mapin with USER_RPL */
extern unsigned char dmachannel;
extern struct buf rctbuf;	        /* buf header for raw device */
extern struct buf cctbuf;	        /* buf header for ct commands */
extern struct buf fstbuf;	       /* buf header for BUFIO device */
extern struct iobuf cttab;
extern int ctdbglvl;
extern int cteoftp;
extern char ctflags;
extern unsigned long altbufsize;
extern unsigned char altbuf[];
extern uint altbuflg;	
extern struct tcb tcb;			/* tape control block	*/

/* buffer management pointers */
extern int ctbin, ctbout;
extern int ctba, ctbs, ctbtop;


#ifdef CT
/* tape driver externals */
extern char chkrdstatus();
extern char qc_rnstb();
extern unsigned char _setdma();
extern unsigned long _resdma();
extern unsigned char ctsetdma();
extern unsigned char cachecheck();
#endif

static paddr_t ctio_cnt;			/* remaining count (bytes) */
static paddr_t ctio_adr;			/* current user xfer addr */
static char *dmadest;
static unsigned long dmasize;			/* dma xfer size (bytes) */

/*
 *		CT Interrupt Handler
 */
ctintr (level)
uint    level;
{
    struct buf *bp;
    int x;

    /*
     * cover our ass, interrupt wise 
     */
	
    x = splbio();
#ifndef	PICFIX1
    eoi(level);
#endif /* ! PICFIX1 */ 
#ifdef CT
    if ((bp = cttab.b_actf) == NULL) 
	{
#endif
	DBG0(printf("ctintr: spurious interrupt cttab.b_active=%x\n",cttab.b_active)); 
	cttab.b_active = IDLE;
#ifdef CT
	outb(INTACKTAPE,0);	/* Acknowledge tape interrupt */
	/* on TVI occurs when cassette is popped */
	/* check to see if new media inserted */
	}
    /* check and possibly read the status */
    else if((chkrdstatus(bp)) != OK)
	/* either got an exception, or reading the status */
	{

	switch (cttab.b_active & (TAPEIO|CTCMD)) 
	    {
	    case TAPEIO:
	    case CTCMD:
		cttab.b_active |= IRQ;
		ctstart();
		break;
	    }
	}
#endif
    splx(x);
}

#ifdef CT
char chkrdstatus (bp)
struct buf *bp;
{
    struct tcb *tc = &tcb;
    char retn = FAIL;

    if EXCEPTION
	{
	tc->tc_statidx = 0;
	tc->tc_rdstat = TRUE;
	tc->tc_rdexstat = FALSE;

	/* indicate reading status */
	cttab.b_active |= (GETINGSTAT);
	DBG13(printf("chkrdstatus: interrupt rstu! cmd=%x, drstat=%x\n",tc->tc_cmd,inb(RD_DR_STATUS)));
        DBG13(dbgprt("ctkrdstatus",cttab.b_actf));
	outb(WR_COMREQ,QC_RSTU);
	retn = OK;
	}
    else 
	{
	if GETNGSTATUS
	    {
	    if (tc->tc_rdstat) 
		{
		if (tc->tc_rdstat = (tc->tc_statidx < QC_STATL))
		    {
		    /* wait for RDY* & DIR* */
		    if (waitRDY() == FAIL)
			{
			DBG0(printf ("chkrdstatus: ERR waitRDY FAIL\n"));
			cteio(bp);			/* EIO Error */
			return(FAIL);
			}
		    tc->tc_stat[tc->tc_statidx++] = qc_rnstb();
		    return(OK);
		    }
		else 
		    {
		    cttab.b_active |=  GOTSTATUS;
		    cttab.b_active &=  ~GETINGSTAT;
		    DBG13(qc_statdump());
		    if (((tc->tc_stat[0]&0x81)==0x81) || (tc->tc_cmd == QC_RESET))
			{  /* status of 0x81 => FILEMARK Read */
			   /* qc_reset() always reads status */
			cttab.b_active &=  ~GOTSTATUS;
			bp->b_error = 0;
			bp->b_flags &= ~B_ERROR;/*turn off error flag*/
			}
		    else
			{
			DBG0(printf ("chkrdstatus: ERR tc_stat[0] = %x \n", tc->tc_stat[0]));
			cteio(bp);			/* EIO Error */
			}
		    retn = FAIL;
		    }
		}
	    else if (tc->tc_rdexstat) 
		{
		if (tc->tc_rdexstat = (tc->tc_statidx < QC_EXSTATL))
		    {
		    /* wait for RDY* & DIR* */
		    if (waitRDY() == FAIL)
			{
			DBG0(printf ("chkrdstatus: waitRDY FAIL\n"));
			cteio(bp);			/* EIO Error */
			return(FAIL);
			}
		    tc->tc_xstat[tc->tc_statidx++] = qc_rnstb();
		    return(OK);
		    }
		else 
		    {
		    DBG0(printf ("chkrdstatus: ERR ext stat\n"));
		    cttab.b_active |=  GOTSTATUS;
		    cttab.b_active &=  ~GETINGSTAT;
		    cteio(bp);			/* EIO Error */
		    retn = FAIL;
		    }
		}
	    }
	else  /* the command is finished */
	    {
	    cttab.b_active &=  ~GOTSTATUS;
	    bp->b_error = 0;
	    bp->b_flags &= ~B_ERROR; /* turn off error flag 	*/
	    retn = FAIL;
	    }
	}
    /* only acknowledge if not reading a status byte! */
    outb(INTACKTAPE,tc->tc_drstat);
    return(retn);
}
#endif

/*
 *	CT Kernel Malloc Buffer Management Routines                  
 *
 *	Buffer management is done in click values.
 *
 *	ctbtop 	+-------+   (ctba+ctbs)
 *		|	|
 *		|	|
 *		|	|
 *		|-------|<- ctbin click address of writes into buffer
 *		|live	|   (from tape on reads, from user on writes)
 *		|data	|
 *		|-------|<- ctbout click address of reads from buffer
 *		|	|   (to user on reads, to tape on writes)
 *		|	|
 *		|	|
 *		|	|
 *		|	|
 *	ctba   	+-------+   
 *
 *	Valid selectors are built from the click addresses when
 *	we copy in and out of the buffer.  Writes from the buffer to
 *	either the user's buffer or to the tape, always start at
 *	ctbout and continue as long as ctbin is not crossed.  Reads
 *	into the buffer always start at ctbin and continue as long
 *	as ctbout is not crossed.  
 *
 */

/* rw_buffer
 *	Read/write cartridge tape into buffer
 *	Input:  char mode - B_READ or B_WRITE
 *	Output:	ctbuffer(&fstbuf, dev) called to initiate BUFIO
 *		fstbuf.b_bcount = the no. of clicks to xfer
 *		fstbuf.b_phsyaddr = position in buffer (ctbin | ctbout)
 *		fstbuf.b_flags = B_READ | B_WRITE | B_CLICKIO
 *		fstbuf.b_un.b_addr = 0
 *	Returns: FAIL if I/O not queued (BUFIO in progress)
 *		 OK otherwise
 */
unsigned char
rw_buffer(mode,count)
unsigned char mode;
int count;
{

#ifdef CT
    if ((cttab.b_active & BUFIO) || (fstbuf.b_flags & B_BUSY))
	{
	DBG5(printf("rw_buffer: fstbuf busy bp=%lx b_active=%x b_flags=%x\n",
	    &fstbuf, cttab.b_active, fstbuf.b_flags));
	return(FAIL);		/* we already have an I/O in progress */
	}
#endif
    fstbuf.b_physaddr = (mode & B_READ) ? ctob((unsigned long)ctbin) :
	ctob((unsigned long)ctbout);   		/*  (position in buf) */
    fstbuf.b_flags = mode | B_CLICKIO;
    fstbuf.b_un.b_addr = 0;
    fstbuf.b_bcount = count;			/* count is in clicks */
    DBG2(printf("rw_buffer: fstbuf bp=%lx b_flags=%x, bcount=%x, b_physaddr=%lx\n",
	(unsigned long)&fstbuf,fstbuf.b_flags,fstbuf.b_bcount,(unsigned long)fstbuf.b_physaddr)); 
#ifdef CT
    ctbuffer(&fstbuf);			/* go ahead and queue BUFIO */
#endif
    return(OK);
}


/* satisfy_read:	
 *	Uses kernel buffer at click ctba.  
 *	Satisfy a read request from the buffer at position ctbout.
 *	input: 
 *	  ctbout - click address to copyout from
 *	  bp->b_un.b_addr - virtual address in user space
 *		to copyout to.
 *	  clicks - # clicks to xfer.
 *	output: 
 *	  ctbout updated to point to next click to xfer.
 *	returns: number of clicks moved
 */

uint
satisfy_read(bp,clicks)			/* M003 */
struct buf *bp;
uint clicks;
{
    paddr_t ctbuf;
    int rc, x;
    uint count; 			/* clicks to xfer */
    unsigned long cnt;			/* bytes to xfer */

    /* don't pass top */
    x = splbio();
    count = ((ctbout + clicks) > ctbtop) ? ctbtop - ctbout : clicks;
    DBG5(printf("sat_rd: ctbout %x, count %x, clicks %x, ctbin %x, \n",
	ctbout, count, clicks, ctbin));
    /* check that data is there */
    if ((ctbout<=ctbin) && (ctbout + count > ctbin))
	{
	DBG0(printf("sat_rd: overread, ctbout %x, count %x, clicks %x, ctbin %x, \n",
	    ctbout, count, clicks, ctbin));
	count = ctbin - ctbout;
	}
    else if (ctbout > ctbin)
	DBG0(printf("sat_rd: ctbin wrapped, ctbout %x, count %x, clicks %x, ctbin %x, \n",
	    ctbout, count, clicks, ctbin));

    if (count == 0)
	goto rderror;

    /* copy out to user buffer */
    ctbuf = ctob((unsigned long)ctbout);
    cnt = ctob((unsigned long)count);
    DBG5(printf("sat_rd: ctbout*512 %lx count*512 %lx\n", 
	(unsigned long)ctbuf,
	(unsigned long)cnt));
    DBG5(printf("sat_rd: b_un.b_addr %lx ctbuf %lx count*512 %lx\n", 
	(unsigned long)bp->b_un.b_addr, 
	(unsigned long)ctbuf, (unsigned long)cnt));	
#define CTCOPYOUT
#ifdef CTCOPYOUT
    if (rc=copyout(mapin(ctbuf,BUFSEL), bp->b_un.b_addr, (uint)cnt))
	{
	DBG0(printf("sat_rd: copyout failed: %x\n",rc));
	bp->b_error = EFAULT;
	goto rderror;
	}
    DBG1(printf("sat_rd: copyout(%lx,%lx,%x), use selector %d\n",
	(unsigned long)ctbuf, bp->b_un.b_addr, (uint)cnt, BUFSEL));
#endif
    ctbout += count;			/* increment position */
    splx(x);
    DBG5(printf("sat_rd: exit, ctbout %x, count %x, clicks %x, ctbin %x\n",
	ctbout, count, clicks, ctbin));
    return(count);
rderror:
#ifndef CT
    cteio(bp);			/* EIO Error */
#endif
    splx(x);
    DBG5(printf("sat_rd: error exit, ctbout %x, ctbin %x\n",
	ctbout, ctbin));
    DBG10(cttimer(CTDELAY));
    return(0);
}


/* satisfy_write:	
 *	Uses kernel buffer at click ctba.  
 *	Satisfy a write request into the buffer at position ctbin.
 *	input: 
 *	  ctbin - click address to copyin to
 *	  bp->b_un.b_addr - virtual address in user space
 *		to copyin from.
 *	  clicks - # clicks to xfer.
 *	output: 
 *	  ctbin updated to point to next click to copyin to.
 *	returns: number of clicks moved (satisfied)
 */

uint
satisfy_write(bp,clicks)			/* M003 */
struct buf *bp;
uint clicks;
{
    paddr_t ctbuf;
    uint count; 				/* clicks to xfer */
    int x, rc;
    unsigned long cnt;

    /* don't pass top of buffer */
    x = splbio();
    count = (ctbin + clicks) > ctbtop ? ctbtop - ctbin : clicks;
    DBG3(printf("sat_wrt: ctbin %x, count %x, clicks %x, ctbout %x, \n",
	ctbin, count, clicks, ctbout));
    /* check that tape is not still writing from there */
    if ((ctbin < ctbout) && (ctbin+count > ctbout))
	{
	DBG0(printf("sat_wrt: overwrite, ctbin %x, count %x, clicks %x, ctbout %x, \n",
	    ctbin, count, clicks, ctbout));
	count = ctbout - ctbin;
	}
    
    if (count != 0)
	{
	/* copy in from user buffer */
	ctbuf = ctob((unsigned long)ctbin);
	cnt = ctob((unsigned long)count);
	DBG5(printf("sat_wrt: b_un.b_addr %lx ctbuf %lx count*512 %lx\n", 
	    (unsigned long)bp->b_un.b_addr, 
	    (unsigned long)ctbuf, (unsigned long)cnt));	
	if (rc=copyin(bp->b_un.b_addr, mapin(ctbuf,BUFSEL), (uint)cnt))
	    {
	    DBG0(printf("sat_wrt: copyin failed: %x\n",rc));
	    cteio(bp);			/* EIO Error */
	    count = 0;
	    }
	DBG1( if (!rc) printf("sat_wrt: copyin(%lx,%lx,%x), use selector %d\n",
	    (unsigned long)bp->b_un.b_addr, (unsigned long)ctbuf, (uint)cnt,
	    BUFSEL));
	}
    ctbin += count;			/* increment position */

    DBG5(printf("sat_wrt: exit, ctbin %x, count %x, clicks %x, ctbout %x",
	ctbin, count, clicks, ctbout));
    DBG5(printf(", resid %x\n",
	bp->b_resid));
    splx(x);
    return(count);		/* return amount moved */
}

/* ctsatisfied:	(user tape I/O to/from kernel buffer)
*	Inputs: bp-> user I/O request buf (see buf.h)
*	Output:	True if request was satisfied, FALSE otherwise.
*
*	Uses kernel buffer at click ctba.  
*	Called by ctstrategy before queueing request.  If we satisfy
*	this request, it is never queued, otherwise we will first queue
*	our own request to service the buffer, then let ctstrategy
*	queue this one.
*
*   	Writes are satisfied by writing into the buffer.  When the 
*	write is processed, if the buffer is more than half full a
*	BUFIO (bp=&fstbuf) is queued to write the buffer. Note that
*   	ctsatisfied returns TRUE if it is finished with the I/O, or
*	FALSE, if there is more to do. 
*/
unsigned char
ctsatisfied(bp)			/* M003 */
register struct buf *bp;
{
uint count, margin, moved;
int x;

count = btoc(bp->b_bcount);
DBG5(printf("ctsatisfied: ctbin %x, count %x, moved %x, ctbout %x\n",
    ctbin, count, 0, ctbout));
if(((bp->b_flags & B_READ)==B_READ))
    {   /* B_READ => we read from ctbout, but not past ctbin */
    while (count)
	{
	if ((ctbout >= ctbtop) && (ctbin != ctbtop))
	    ctbout = ctba;	/* wrap around at top, if there is more */
	/* now we can use what ever formulae we wish to decide 
	 * how much, if any, we read. 
	 */
	DBG2(printf("ctsatisfied: ctbin-ctba>>1 %x\n",(ctbin-ctba)>>1));
	if ((ctbout<=ctbin) && 
	    (ctbin-ctbout <= ((ctbin-ctba)>>1)))
	    {  /* if data left in buffer is less than half that
		* read so far, then read again ...
		*/
	    if (ctbin == ctbout) 		/* if < count*4 */
		rw_buffer(B_READ,count<<3); /* read 8 times the req */
	    else
		rw_buffer(B_READ,ctbtop-ctbin);/* read the rest */
	    }
	else if (ctbin < ctbout)	/* ctbin has wrapped around */
	    rw_buffer(B_READ,ctbout-ctbin); /* read as much as pos */
	if (!(moved = satisfy_read(bp, count)))
	    {	/* can't get anymore, must wait til I/O done */
	    goto no_satisfaction;
	    }
	else
	    count -= moved;		/* decrement amount xfered */
	DBG5(printf("ctsatisfied: ctbout %x, count %x, moved %x, ctbin %x\n",
	    ctbout, count, moved, ctbin));
	}
    }
else		/* B_WRITE */
    {
    while (count && (ctbin <= ctbtop))
	{
	if (!(moved = satisfy_write(bp, count)))
	    {	/* can't put anymore, must wait til I/O done */
	    goto no_satisfaction;
	    }
	else
	    count -= moved;		/* decrement amount xfered */
	DBG5(printf("ctsatisfied: ctbin %x, count %x, moved %x, ctbout %x\n",
	    ctbin, count, moved, ctbout));
	if (ctbin >= ctbtop)
	    {
	    if (ctbin > ctbout)		 /* write rest of buffer */
		rw_buffer(B_WRITE,ctbtop-ctbout);     
	    if (ctbout > ctba)		/* have we written any yet? */
		ctbin = ctba;		 /* yes so ok to wrap around at top */
	    }
	else if ((ctbin & SEGMASK) > (ctbout & SEGMASK))  /* cross a 64K boundary? */
	    {
	    rw_buffer(B_WRITE,(ctbin&SEGMASK)-ctbout);	/* I/O to boundary */
	    }
	}
    }

    DBG5(printf("ctsatisfied: exiting ctbin %x, resid %x, ctbout %x\n",
	ctbin, bp->b_resid, ctbout));
    return(TRUE);			/* indicate I/O finished */

no_satisfaction:			/* we could not complete I/O */
#ifdef CT
    /* not enough, must wait to satisfy read */
    bp->b_resid = ctob((long)count); /* set resid */
    DBG5(printf("ctsatisfied: exit not sat, err %x, ctbin %x, resid %x, ctbout %x\n",
	bp->b_error, ctbin, bp->b_resid, ctbout));
    return(bp->b_error);		/* queue unless error 	   */
#else  /* If Buffer Manager Device */
    /* return a residual amount, but do not queue */
    bp->b_resid = ctob((long)count);
    DBG5(printf("ctsatisfied: err exit ctbin %x, resid %x, ctbout %x\n",
	ctbin, bp->b_resid, ctbout));
    return(TRUE);			/* return to user in any case */
#endif
} 

/*
 *	TAPE I/O Initiation and Interrupt Handling Routines
 *		called from ctstart().
 */

#ifdef CT
/* tapeio:	Issue the appropriate sequence of commands to transfer 
 *		data.
 *
 *	Input: bp->buf struct containing I/O request.
 *	Output:	ctio_cnt is set to bytes still to be xfer'd.
 *		ctio_adr is address to transfer to/from
 *		dmasize is set to DMA count requested.
 *	Returns:  DONE, if I/O completed, MORE, otherwise.
 *
 *	This is where all data requests start
 *
 *	If the xfer must be broken up into smaller pieces, it is
 *	done by the IRQ routine, tapeioirq() returning MORE to 
 *	ctstart() which cycles through here again. We use the 
 *	CTMORE bit let us know if a call is a new I/O or the next
 *	piece of the last one.  BUFIO requests may be really big
 *	as the count (bp->b_bcount) is in clicks.
 *
 * 	tapeio() will be entered from start on first strategy call and
 *   	also from a multiple command transfer during an interrupt
 *   	from the previous command's complete.	It cycles back from
 *   	tapeioirq() by the newstate: of ctstart() and should return
 *   	MORE unless it is finished with the request (returns DONE).
 *
 */
unsigned char
tapeio(bp)
struct buf *bp;
{

    /* 1st time set up counters */
    if (!(cttab.b_active & CTMORE))
	{
	/* for BUFIO b_bcount is in clicks (see new b_flags
	 * definition, in ct.h for now)
	 */
	dmasize = ctio_cnt = (bp->b_flags & B_CLICKIO) ?
	    ctob((unsigned long)bp->b_bcount) : bp->b_bcount;
	ctio_adr = bp->b_physaddr;	
	}
    else
	dmasize = ctio_cnt;		/* set to current count */

    DBG2(
	printf("tapeio: ctio_adr=%lx ctio_cnt=%lx ctbin=%x ",
	    (unsigned long)ctio_adr,(unsigned long)ctio_cnt,ctbin);
	printf("b_active=%x bp=%lx b_flags=%x ctbout=%x ctbtop=%x ctba=%x\n",
	    cttab.b_active, bp, bp->b_flags,ctbout,ctbtop,ctba);
	);

    /* check for a unsatisfied user I/O if in buffered mode */
    if (ctba && (!(cttab.b_active & BUFIO)))
	{	/* if all worked acording to plan we will be
		 * able to satisfy this request now.
		 */
	if (!ctsatisfied(bp))
	    {
	    cteio(bp);
	    DBG0(printf("tapeio: ctsatisfied failed!\n"));
	    }
	return(DONE);
	}

    /* set up dma for tape operation */
    if (ctsetdma(bp, ctio_adr, &dmasize) == FAIL)    
	{
	DBG0(printf("tapeio: ctsetdma failed!\n"));
	cteio(bp);				/* EIO Error */
	bp->b_resid = bp->b_bcount;		/* nothing xfered */
	return(DONE);
	}

    /* dmasize has been set to the actual DMA request count */
    if ((bp->b_flags & B_READ) == B_READ) 
	{
	if (ctflags & T_WASFM)    
	    {
	    DBG0(printf("tapeio: FILE MARK!\n"));
	    bp->b_error = ENXIO;
	    bp->b_flags |= B_ERROR;
	/*  cteio(bp);				/* EIO Error */
	    bp->b_resid = bp->b_bcount;		/* nothing xfered */
	    return(DONE);
	    }
	else
	    qc_sio(B_READ);
	}
    else 
	qc_sio(B_WRITE);
    cttab.b_active |= CTMORE;   /* tapeioirq() resets if no residual */
    return(MORE);
}


/* tapeioirq:	Handle irqs for transfer and command completes.
 *   	tapeioirq returns DONE if it is finished with the I/O, or
 *	MORE, if there is more to do. 
 *	Input: 	bp->buf struct with I/O request
 *	Output: all necessary counters, ctio_cnt, ctio_adr, dmasize,
 *		are updated to reflect I/O.  If BUFIO bit set, the
 *		buffer pointer ctio is also updated.
 *		When DONE is returned, bp->b_resid is set to residual
 *		bytes.  (If BUFIO, bp->b_resid is in clicks).
 *		If an error occurs, bp->b_error is set, and the B_ERROR
 *		bit in b_flags is set.  ENXIO is set if end of tape
 *		detected.  If a file mark is read, it is not an error,
 *		just completion of I/O.  Hard errors, and I/O parameter
 *		errors, (e.g. odd address, request not a multiple of
 *		512 bytes), return the error EIO.
 *	Returns: DONE, if I/O request is completed or errors
 *		 MORE, if we will go to tapeio() to do another piece.
 */
unsigned char
tapeioirq(bp)
struct buf *bp;
{
    struct tcb *tc = &tcb;		/* for checking status */
    uint err, i;
    unsigned long resid;
    caddr_t usrbuf;

    /* reset interrupt indicator for ctstart()'s benefit */
    /* will cause ctstart() to invoke non-irq routines */
    cttab.b_active &= ~IRQ;
    if(cttab.b_active&TAPEIO)
	{
	if(resid=_resdma(dmachannel))
	    {
	    dmasize -= resid;		/* set dmasize to amount moved */
	    DBG9(printf("tapeioirq: ctio_cnt=%lx,ctio_adr=%lx,resid=%lx,dmasize=%lx\n",
		(unsigned long)ctio_cnt,(unsigned long)ctio_adr,
		(unsigned long)resid,(unsigned long)dmasize));
	    }
	/* must be a successful completion of read or write */
	if (!(bp->b_error))
	    {
	    if ((bp->b_flags & B_READ) == B_READ) 
		{
		if (altbuflg) 
		    {/* alternate buffer*/
		    DBG2(printf("tapeioirq: read into alt. buffer: "));
		    DBG9(ctdump(altbuf));

		    /* for reads copy usr data from the alternate buffer */
		    DBG2(printf("tapeioirq: bcopy(%lx,%lx,%x);\n",altbuf,ctio_adr,(uint)dmasize));
		    bcopy(altbuf, mapin(ctio_adr, BUFSEL), (uint)dmasize);
		    

		    altbuflg = 0;
		    DBG2(printf("tapeioirq: finished with alt. buffer\n"));
		    }
		}
	    else 
		{	/* successful single tape block write */
		ctflags |= T_WASWRIT;
		DBG2(printf("tapeioirq:TAPEIO write done,res=%lx,ctio_cnt=%lx\n",
		    (unsigned long)resid,(unsigned long)ctio_cnt));
		}
	    }
/*	bp->b_resid -= dmasize;*/		/* M001 */
/*	if (resid)*/				/* M001 */
/*	    ctio_cnt = 0;*/			/* M001 */
/*	else*/					/* M001 */
/*	    ctio_cnt = bp->b_resid;*/		/* M001 */

	/* increment pointers */
 	ctio_cnt -= dmasize;		  /* dec remaining count M001 */
 	ctio_adr += dmasize;		  /* increment address */
	if (cttab.b_active & BUFIO)
	    {
	    if ((bp->b_flags & B_READ) == B_READ) 
		{
		ctbin += btoc(dmasize);	  /* update ctb input pointer */
		if (ctbin >= ctbtop)
		    ctbin = ctba; 	  /* wrap around at top */
		}
	    else  /* B_WRITE */
		{
		ctbout += btoc(dmasize);  /* update ctb output pointer */
		if (ctbout >= ctbtop)
		    ctbout = ctba; 	  /* wrap around at top */
		}

	    DBG2(if (ctba) 
		printf("tapeioirq: ctio_adr=%lx ctio_cnt=%lx ctbin=%x ",
		    (unsigned long)ctio_adr,(unsigned long)ctio_cnt,ctbin);
		printf("ctbin=%x ctbout=%x ctbtop=%x ctba=%x\n",
		    ctbin,ctbout,ctbtop,ctba)
		);
	    }

	/* tapeio & read/write failure */
	if (cteoftp = (tcb.tc_stat[0]&QC_EOT))	/* indicate end of tape */
	    {
	    printf("\nstreaming tape: end of tape\n");
	    bp->b_error = ENXIO;
	    } 
	if ((resid) || (ctio_cnt <= 0) ||  (bp->b_error))	/* M001 */
	    {		/* tapeio complete */
	    bp->b_resid = (bp->b_flags&B_CLICKIO) ? btoc(ctio_cnt) :
		(uint)ctio_cnt;				/* M001 */
	    if ((tc->tc_stat[0]&0x81)==0x81)	/* rfm => no error, no resid */
		{

		ctflags &= ~(T_WASREAD);
		ctflags |= T_WASFM;	/* M004 */
		DBG2(printf("tapeioirq: rfm, res=%x, not err=> reset resid!\n",bp->b_resid));
/*		bp->b_resid = 0;*/
		}
	    cttab.b_active &= ~CTMORE;		/* no more to go */
	    DBG2(printf("tapeioirq:TAPEIO fini,res=%x,dmasize=%lx\n", 
		bp->b_resid, 
		(unsigned long)dmasize));
	    return(DONE);
	    }
	else if (ctio_cnt < dmasize) 
	    {
	    if (ctio_cnt & (long)BLOCKMASK) 
		{ 		/* not multiple of BLOCKSZ  M001 */
		cteio(bp);			/* EIO Error */
		DBG0(printf("tapeioirq: %s err. ctio_cnt (%lx) must be multiple of %x. ctio_adr=%lx\n",
		    (bp->b_flags&B_READ)?"read":"write",
		    (unsigned long)ctio_cnt,BLOCKSZ,(unsigned long)ctio_adr));
		return(DONE);	/* not multiple of BLOCKSZ  M001 */
		}
	    }

	/* turn off all but these bits */
	cttab.b_active &= (CTCMD|BUFIO|TAPEIO|CTMORE);
	DBG9(printf("tapeioirq: ctio_cnt=%lx,ctio_adr=%lx,resid=%lx,dmasize=%lx,ret=%x\n",
	    (unsigned long)ctio_cnt,(unsigned long)ctio_adr,
	    (unsigned long)resid,(unsigned long)dmasize,MORE));
	return(MORE);
	}

    DBG14(printf("tapeioirq: cmd completion, ret=%x\n",DONE));
    return(DONE);		/* not nec. failure; just end */
}



/* ctsetdma:	- initialize the dma hardware for the I/O
 *	Inputs: bp->structure buf (buffered I/O)
 *		rqaddr (paddr_t) - memory address for DMA
 *		cntp (unsigned long *)    - amount to try to xfer
 *	Returns:	OK if hardware initialized
 *			FAIL otherwise
 *	Output:		sets altbuflg if alt buffer used
 *			sets *cntp to count actually requested
 *
 *	sets up the 8237 for requested size writes & reads.
 *	Returns with *cntp set to the size to be transfered, this
 *	will be less than the requested amount, if the 
 *	address region would otherwise cross the 16bit DMA boundry,
 *	(64K words).
 */
unsigned char
ctsetdma(bp, rqaddr, cntp)
struct buf *bp;
paddr_t rqaddr;
unsigned long *cntp;
{
    /* destination (source for write) address in 24bit physical space */
    int i, err;
    paddr_t dest;			/* used for altbuf xfers */
    caddr_t usrbuf;
    unsigned char dmamode;

    altbuflg = 0;
    DBG9(printf("ctsetdma: dest = %lx, *cntp = %lx\n", 
	(unsigned long)dest, *cntp));
    /* set up dma mode for transfer */
    dmamode = (bp->b_flags & B_READ) ?  WRITEMEM : READMEM;
    dmamode |= SINGLEMODE | AUTODISABLE | ADRINC;
    if (err = _setdma(dmachannel, dmamode, rqaddr, cntp))
	{
#ifndef	OLDDMA
	if (err != EDMACNT)
	{
#endif
	DBG0(printf("ctsetdma: _setdma error %x\n",err));
	bp->b_error = EFAULT;
	return(FAIL);
#ifndef	OLDDMA
	}
#endif
	}
    if ((uint)*cntp & BLOCKMASK)
	{/* adjusted dma size is not a multiple of 512 bytes */
	if (*cntp &= 0xFFFFFE00L)
	    err = _setdma(dmachannel, dmamode, rqaddr, cntp);
	else
	    {	/* if bad addr or less than 512 bytes */
	    DBG0(printf("ctsetdma: DMA page rollover, using altbuf\n" ));
	    DBG9(cttimer(CTDELAY));

	    *cntp = altbufsize;
	    DBG9(printf("ctsetdma: setting *cntp to %lx\n", *cntp));

	    dest = physaddr(altbuf);
	    if ((bp->b_flags & B_READ) != B_READ) 
		{ 
		/* for writes copy usr data to the alternate buffer */

		DBG9(printf("ctsetdma: mapin ctio_adr = %lx \n", ctio_adr));
		DBG2(printf("ctsetdma: bcopy(%lx,%lx,%x) \n", ctio_adr, dest,(uint)*cntp));
		bcopy(mapin(ctio_adr, BUFSEL), altbuf, (uint)*cntp);
		DBG9(printf("ctsetdma: altbuf contains: " ));
		DBG9(ctdump(altbuf));
		DBG9(printf("\n"));
		DBG9(cttimer(CTDELAY));
		}

	    DBG9(printf("ctsetdma: dest = %lx, *cntp = %lx\n", 
		(unsigned long)dest, *cntp));
	    if (_setdma(dmachannel, dmamode, dest, cntp)
#ifdef	OLDDMA
		    == FAIL) 
#else
		    != DMAOK) 
#endif
		{
		DBG0(printf("ctsetdma: can't set dma to altbuf = %lx\n",
		    (unsigned long)dest));
		bp->b_error = EFAULT;
		return(FAIL);
		}
	    else 
		{
		altbuflg = 1;
		}
	    }
	}
#ifndef	OLDDMA
    _enabledma(dmachannel);
#endif	/* OLDDMA */
    /* note *cntp may have been adjusted if ctio_cnt was too large
     * or DMA page boundary was crossed.  *cntp now reflects the 
     * actual requested amount.
     */
    return(OK);
}
#endif CT

/*
 * cteio():    indicate EIO error
 */
cteio(bp)
struct buf *bp;
{
    bp->b_flags |= B_ERROR;
    bp->b_error = EIO;
    DBG0(dbgprt("cteio",bp));
}

/*
 * dbgprt():    display  diagnostics
 */
dbgprt(s,bp)
char *s;
struct buf *bp;
{
DBG0(
    printf("%s: active=%x b_actf=%lx bp=%lx b_flags=%x ctflags=%x bp->b_bcount=%x err=%x res=%x\n",
	s,cttab.b_active,cttab.b_actf,bp,bp->b_flags,ctflags,bp->b_bcount,bp->b_error,bp->b_resid)); 
     /* DBG0 */
}

/*
 * ctdump():    	Dump 16 bytes in hex
 */
ctdump(cp)
unsigned char *cp;
{
    uint i;

    for(i=0;i<16;i++)
	printf("%x ",*cp++);
    printf("\n");
}

