/* uportid = "@(#)qic02.c	Microport Rev Id  1.3.8 1/18/87" */
/* Copyright 1987 by Microport. All Rights Reserved.
 *
 * TVI TeleCAT-286 Streaming Tape Device Driver 
 * References:
 *	TVI Streaming Tape Interface Hardware Specifications
 *
 * Initial Coding:
 *		unix-net!doug
 *		unix-net!mark Mon Nov 10 22:19:51 PST 1986
 * Modification History:
 *	M001: uport!doug Mon Jan 19 00:59:19 PST 1987
 *		Put qc_sio() routine in here (moved from ctsetdma())
 * To do:
 *	1) incorporate sharing the 8237 dma channels with other drivers
 */

/*
 *
 * QIC02 Interface Routines
 * 
 */
#include "sys/types.h"
#include "sys/errno.h"
#include "sys/sysmacros.h"
#include "sys/param.h"
#include "sys/systm.h"
#include "sys/buf.h"
#include "sys/iobuf.h"
#include "sys/conf.h"
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

extern struct buf cctbuf;
extern struct buf rctbuf;               /* buf header for raw device */
extern struct iobuf cttab;              /* buf header for raw device */
extern struct tcb tcb;			/* tape control block 	     */
extern char ctflags;			/* tape read/write flags     */
extern int ctdbglvl;



/* qc_rstu - read tape drive status bytes 				*/
/*	     input: 	none						*/
/*	     returns:  	none						*/
qc_rstu ()
{    
    register struct tcb *tc = &tcb;
    int timeout = 0xffff;

    tc->tc_statidx = 0;
    tc->tc_rdstat = TRUE;
    tc->tc_rdexstat = FALSE;
    if (cttab.b_actf == NULL)
	{
	cttab.b_actf = &cctbuf;
	if ((cttab.b_active&(TAPEIO|CTCMD))== IDLE)
	    cttab.b_active |= CTCMD;
	}
    /* indicate reading status */
    cttab.b_active |= (GETINGSTAT);
    outb(WR_COMREQ,QC_RSTU);
}	   


/* qc_sio - start I/O operation
 *
 *	     input: 	unsigned char
 *	     returns:  	none
 */
qc_sio (mode)
unsigned char mode;
{    
    register struct tcb *tc = &tcb;

    if (mode == B_READ)
	{ 
	outb(WR_CNTL,tc->tc_drstat|QC_ONLINE);
	if ((ctflags&(T_WASWRIT))) 
	    {
	    qc_reset();		/* must reset if in write state */
	    }
	DBG14(if (!((tc->tc_drstat = inb(RD_DR_STATUS)) & QC_ONLINE))
	    printf("tapeio: online found low\n"));
	if ((ctflags&(T_WASREAD))) 
	    {
	    outb(RDSTM_ON,1);
	    DBG2 (printf ("qc_sio: outb RDSTM_ON 1\n"));
	    }
	else
	    {
	    qc_cmd(QC_RD);
	    DBG2 (printf ("qc_sio: start read\n"));
	    }
	ctflags = T_WASREAD;
	}
    else			/* mode = B_WRITE */
	{ 
	if ((ctflags&(T_WASREAD))) 
	    {
	    qc_reset();			/* must reset if in read state */
	    }
	DBG14(if (!((tc->tc_drstat = inb(RD_DR_STATUS)) & QC_ONLINE))
	    printf("tapeio: online found low\n"));
	outb(WR_CNTL,tc->tc_drstat|QC_ONLINE);
	qc_cmd(QC_WRT);
	DBG2 (printf ("qc_sio: start write\n"));
	ctflags = T_WASWRIT;
	}
}


/* qc_estu - read extended tape drive status bytes 			*/
qc_estu ()
{    
    register struct tcb *tc = &tcb;

    DBG1(printf("qc_estu:  \n"));
    tc->tc_statidx = 0;
    tc->tc_rdstat = FALSE;
    tc->tc_rdexstat = TRUE;
    if (cttab.b_actf == NULL)
	cttab.b_actf = &cctbuf;
    cttab.b_active |= GETINGSTAT;	/* indicate reading status */
    outb(WR_COMREQ,QC_ESTU);
}	   

/* qc_cmd - send a qic 02 command				  	*/
qc_cmd (cmd)
char cmd;
{    
    register struct tcb *tc = &tcb;
    int timecnt = 20000;

    if EXCEPTION
	{
	DBG3(printf("qc_cmd: drive has exception status - %x\n",tc->tc_drstat));
	qc_rstu();
	}
    else 
	{

	timecnt = 20000;
	while (timecnt-- && ( (tc->tc_drstat = inb(RD_DR_STATUS)) & QC_RDY));
	if(!timecnt)
	    {
	    DBG1(printf ("qc_cmd: cannot send a command %x \n",tc->tc_drstat));
	    }
	else
	    {
	    outb(WR_COMREQ, cmd);
	    }
	}
}	   
/* qc_init - initialize the tape controller	 			*/
/*	     input: 	none						*/
/*	     returns:  	none						*/
qc_init ()
{    
    register struct tcb *tc = &tcb;
    int x;


    DBG6(printf("qc_init:  \n"));
    tc->tc_wrcntl = QC_ONLINE;
    tc->tc_cmd = 0;
    tc->tc_trkno = 1;
    tc->tc_statidx = 0;
    tc->tc_rdstat = FALSE;
    tc->tc_stat[0] = 0xa0;
    tc->tc_stat[1] = 0xb1;
    tc->tc_stat[2] = 0xc2;
    tc->tc_stat[3] = 0xd3;
    tc->tc_stat[4] = 0xe4;
    tc->tc_stat[5] = 0xf5;
    tc->tc_rdexstat = FALSE;
    tc->tc_xstat[0] = 0x80;
    tc->tc_xstat[1] = 0x81;
    tc->tc_xstat[2] = 0x82;
    tc->tc_xstat[3] = 0x83;
    tc->tc_xstat[4] = 0x84;
    tc->tc_xstat[5] = 0x85;
    outb(INTACKTAPE,0);	/* must do this to get any interrupts */
}

/* qc_reset - reset tape drive 						*/

qc_reset ()
{
    register struct tcb *tc = &tcb;
    int x, i;
    int loopcnt;

    DBG6(printf("qc_reset:  \n"));
    x = splbio();
    tc->tc_rdstat = TRUE;
    tc->tc_wrcntl |= QC_IRQ;
    tc->tc_drstat = inb(RD_DR_STATUS);
    DBG6(printf("qc_reset: drive status before reset: %x\n",tc->tc_drstat));
    cttab.b_actf = &cctbuf;
    cttab.b_active = CTCMD;
    cttab.b_active |= (GETINGSTAT);
    splx(x);
    tc->tc_cmd = QC_RESET;
    if (resetape())
	{
	iowait(cttab.b_actf); 
	}
    else
	DBG1(printf("qc_reset: resetape failed \n"));
    qc_statdump();
    x=splbio();
    cttab.b_active = IDLE;
    cttab.b_actf = 0;
    tc->tc_rdstat = FALSE;
    tc->tc_rdexstat = FALSE;
    rctbuf.b_flags = B_TAPE;
    cctbuf.b_flags = B_TAPE;
    rctbuf.b_error = 0;
    cctbuf.b_error = 0;
    rctbuf.av_forw = 0;
    cctbuf.av_forw = 0;
    rctbuf.av_back = 0;
    cctbuf.av_back = 0;
    splx(x);

}

qc_statdump()
{
    struct tcb *tc = &tcb;
    int i;

    DBG1(printf("qc_statdump RSTU status bytes: "));
    DBG1(for (i=0;i<QC_STATL;i++)printf(" %x",tc->tc_stat[i]));
    DBG1(printf("\n"));
}
