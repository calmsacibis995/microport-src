/* uportid = "@(#)ct.h		Microport Rev Id  1.3.8 1/9/87" */
/*
 *
 * Microport Cartridge Tape Header
 *
 * Copyright 1987 by Microport. All Rights Reserved.
 *
 * TVI TeleCAT-286 Streaming Tape Device Driver 
 * References:
 *	TVI Streaming Tape Interface Hardware Specifications
 *
 * Initial Coding:
 *		unix-net!doug
 *		unix-net!mark Mon Nov 10 22:19:51 PST 1986
 * Modification History:
 *	M000: uport!doug Fri Jan  9 10:19:15 PST 1987
 *		Add IOCTL for streaming support.
 *	M001: uport!doug Tue Feb 24 20:14:31 PST 1987
 *		Add T_WASFM for file mark detection.
 * To do:
 *	1) ioctl interface for process lock and long I/O count 
 *	2) incorporate sharing the 8237 dma channels with other drivers
 *	   (change _ctsetdma to setdma() for inclusion in machdep.c)
 *	3) test generic/specific separation by adapting driver to other
 *	   tape controllers.
 *
 *	All device specific parameters are defined in the header file included
 *	RIGHT BEFORE this one, as in:
 *
 *#include "qic2tvi.h"
 *#include "ct.h"
 *
 *	for the televideo TeleCAT-286 Streaming Tape Interface (STI) board
 */

 /* Define ALTBUF to allocate a Alternative buffer in driver for 
  * handling cases when DMA cannot be done to user space.
  */
#define BLOCKSZ   512		/* 1/29/87 uport!rex */
#define BLOCKMASK 0x1ff		/* 1/29/87 uport!rex */
#define SEGMASK	0xff80		/* mask click address down to 64K boundary */
#define ALTBUF
/*#define ALTBUFSZ MAXCOUNT*/
#define ALTBUFSZ BLOCKSZ
#define useracc1 useracc	/* 1/28/87 uport!rex */


/*
 *	DEBUG SWITCHES 
 *
 * change debug.h to define DEBUG and INTDEBUG switches,
 * this file should not be edited.
 */
#include "ctdebug.h"	/* M001 */
/*#define INTDEBUG*/	/* M001 */
/*#define DEBUG */	/* M001 */
/*#define ALTBUF*/	/* M001 */

/* abstractions */
#define OK	0
#define MORE	0
#define DONE	1
#define FAIL	1
#define TRUE	1
#define FALSE	0

/* Memory swap unit handling */
#define CLICKSIZE 512L;			/* size of kernel clicks */

/* Special b_flags define for bp->b_bcount = clicks */
#define B_CLICKIO 020000

/* additional macros for sysmacros.h */
/* clicks to words */
#ifdef BPCSHIFT
#define WPCSHIFT BPCSHIFT-1
#define	ctow(x)	((x)<<WPCSHIFT)
#else
#define	ctow(x)	((x)*(NBPC<<1))
#endif

/* words to clicks */
#ifdef BPCSHIFT
#define	wtoc(x)	((unsigned)(((long)(x)+(NBPC-1))>>WPCSHIFT))
#define	wtoct(x)	((unsigned)((x)>>WPCSHIFT))
#else
#define	wtoc(x)	((unsigned)(((long)(x)+(NBPC-1))/(NBPC>>1)))
#define	wtoct(x)	((unsigned)((x)/(NBPC>>1)))
#endif

/* additional macro for mmu.h */

#define	gstouv(x)	((char *)((long)((x) << 3 | GDT_TI | USER_RPL) << 16))
/* TAPE Driver specific defines for ctflags */
#define B_TAPE B_OPEN
#define T_WASFM   0x10		/* file mark was read on this open M001 */
#define T_WASREAD 0x20		/* device was read on this open */
#define T_WASWRIT 0x40		/* device was written on this open */
#define T_WORDIO  0x80		/* b_bcount is the I/O word count */

/* fixed constants */
#define TRK1	1		                              /* first track */
#define MAXTRK	4		                         /* number of tracks */
#define BOT	1		         /* beginning of tape record postion */
#define MINCOUNT 512		             /* minimum transfer size (512) */
#define MAXCOUNT 0xffff		             /* maximum transfer size (64K-1) */
#define CTMAXBUF 0x200			/* maximum kernel malloc in clicks */

/* minor device bit definitions */
#define REWIND(dev)	(((minor(dev))&0x04) == 0)	  /* rewind on close */
#define RETENS(dev)	(((minor(dev))&0x08))	  	/* retension on open */
#define RESET(dev)	(((minor(dev))&0x10))	  	/* reset on open */
#define ISDEBUG(dev)	(((minor(dev))&0x20))	  	/* debug level */
#define ISERASE(dev)	(((minor(dev))&0x40))	  	/* erase tape */

#define ISFAST(dev)	(((minor(dev))&0x80))	/* allocate kernel buffer */

/*
 *	State definition Bits
 *		
 *		ORed (|) into cttab.b_active to determine the current active
 *		state.
 *
 *		ANDed (&) with 1's complement (~) to remove state 
 *
 *		IRQ, CTCMD, TAPIO are used in switch statement of ctstart()
 *		*STAT* are used in chkrdstatus() as sub-states of a sort
 *		CTMORE is used in tapeioirq() for multiple block transfers
 */
#define	IDLE		0
#define	IRQ		(1<<0)	/* Have picked up an irq from the device*/
#define	CTCMD		(1<<1)	/* Special command being serviced	*/
#define	TAPEIO		(1<<2)	/* Transfer req being processed		*/
#define	BUFIO		(1<<3)	/* Xfer to/from buffer being processed  */
#define	CTMORE		(1<<4)	/* Active stream of requests		*/
#define	GOTSTATUS	(1<<5)	/* status read has finished		*/
#define	GETINGSTAT	(1<<6)	/* Geting the status bytes 		*/

/* macros for miscellaneous states */
#define GETNGSTATUS (cttab.b_active&GETINGSTAT)
#define EXCEPTION ((inb(RD_DR_STATUS)&QC_OK)!=QC_OK)
#define PICDISABLED ((inb(PICSLAVE)&SL7))

/*
 * tape control block
 */
struct tcb {
	char	tc_cmd;		                            /* qic02 command */
	char	tc_wrcntl;		         /* qic02 write command port */
	char	tc_drstat;	                             /* drive status */
	char	tc_rdstat;			   /* true if reading status */
	char	tc_rdexstat;		  /* true if reading extended status */
	short   tc_statidx;		    		/* index for tc_stat */
	char	tc_stat[QC_STATL];	                     /* qic02 status */
	char	tc_xstat[QC_EXSTATL];	            /* qic02 extended status */
	char	tc_trkno;	                             /* track number */
};

