/* uportid = "@(#)765.h	Microport Rev Id  1.3.8 11/24/86 " */
/* Copyright 1985 by Microport. All Rights Reserved.
 *
 * fd.h: Include file for the Intel 8272 (NEC 765) Floppy Disk Controller
 *
 * References:
 *	Intel Microprocessor and Peripheral Handbook. Intel, 1983.
 *		A: Order #210844-001, pps 6-224 to 6-242 (8272A).
 *	IBM Technical Reference Manual for the AT, March 1984
 *		B: Fixed Disk and Diskette Adapter (Appendix/Update)
 *
 * Initial Coding:
 *		uport!dwight	Fri Oct 18 22:19:51 PDT 1985
 *
 * Modification History:
 * 	M000:  uport!dean     Wed Jan  9 20:36:49 PST 1987
 *		Added MP386 ifdef's.
 *	M001:  uport!larry    Thu Mar  5 20:35:13 PST 1987
 *		Added fdupfix() uport minor conversion.
 *	M002: uport!dean Wed Mar 18 21:42:07 PST 1987
 *		Added #define for b_cylin for V.3 system
 *	M003: uport!fredo  Wed Apr  8 19:51:56 PST 1987
 *        Added #define QDCMD for cache buffer draining.
 *	M004: uport!rex		Mon Aug 31 13:31:51 PDT 1987
 *		Added SDS for Fred's fdwrtchk() routine
 *		Added #define for WPCHECK to control fdwrtchk()
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
#include <sys/utsname.h>
#include <sys/elog.h>
#ifdef MP386
#include <sys/immu.h>
#else
#include <sys/erec.h>
#endif /* MP386 */
#include <sys/errno.h>
#include <sys/trap.h>

#ifndef MP386
#include <sys/mmu.h>
#include <sys/seg.h>
#endif	/* MP386 */

#ifdef MP386
#define b_cylin b_start
#define b_secno b_sector
#endif

/* I/O Port Addresses:	(B:p11)						*/
#define	DOR		0x3F2	/* Floppy Digital Output Reg 		*/
#define	STATREG	0x3F4	/* Floppy controller main status reg	*/
#define	DATAREG	0x3F5	/* Floppy controller main data reg	*/
#define	DCR		0x3F7	/* Digital Control Register Address	*/

/* I/O Commands:							*/
#define	STATUS	(unsigned char) (inb(STATREG))
#define	CMD(x)	outb((int) DATAREG, (char) (x))
#define	RECALIB		0x07	/* Recalibrate command 		*/
#define	SPECIFY		0x03	/* The Specify command 		*/
#define	SDS		0x04	/* Sense Drive Status		*/
#define	SIS		0x08	/* Sense Interrupt Status	*/
#define	SEEKCMD		0x0F	/* The seek command		*/
#define WRCMD		0xC5	/* Write Data			*/
#define	RDCMD		0xE6	/* Read Data			*/
#define	RDTRACK		0x62	/* Read an entire track		*/
#define	READID		0x4A	/* Read sector ID		*/
#define	FLFORMAT	0x4D	/* Format a track		*/
#define	QDCMD		0xEF /* Queue Drain command        	*/

/* Specify Command							*/
/* Note: The values for HUT,SRT, and HLD are from the bios, p 5-170	*/
#define	HUT		0x01	/* Head Unload Time (= 16 ms)		*/
#define	SRT		0x0D	/* Step Rate Time (= 3 ms)		*/
#define	HLD		0x02	/* Head Load Time (= 4 ms)		*/

#define	NDM		1	/* Non-Dma Mode				*/
#define	DMA		0	/* DMA mode				*/
#define	SPECWRD1	((SRT<<4)|HUT)	/* first command word		*/
#define	SPECWRD2	(HLD|DMA)	/* second command word		*/

/* The Recalibrate Command						*/
#define	RECAL0	0	/* Recalibrate Drive 0			*/
#define	RECAL1	1	/* Recalibrate Drive 1			*/

/* Bit Definitions for the main status register:			*/
#define	FDD0BUSY	(1<<0)	/* FDD #0 is in the seek mode		*/
#define	FDD1BUSY	(1<<1)	/* FDD #1 is in the seek mode		*/
#define	FDD2BUSY	(1<<2)	/* FDD #2 is in the seek mode		*/
#define	FDD3BUSY	(1<<3)	/* FDD #3 is in the seek mode		*/
#define	FDCBUSY	(1<<4)	/* Read or write command is in progress	*/
#define	NONDMA	(1<<5)	/* Controller is in non-DMA mode	*/
#define	DIO		(1<<6)	/* Data Transfer Direction (see FRTP)	*/
#define	RQM		(1<<7)	/* Ready to send/receive data		*/

#define	FRTP		1	/* Data Direction: From Reg to Processor*/
#define	FPTR		0	/* Data Direction: From Processor to Reg*/

/* Bit fields for the auxilary status registers. A: p6-235		*/
/* Status Register 0:							*/
#define	SR0US0	(1<<0)	/* Drive unit #				*/
#define	SR0US1	(1<<1)	/* Drive unit #				*/
#define	HD0		(1<<2)	/* Head Address				*/
#define	NR		(1<<3)	/* Not ready				*/
#define	EC		(1<<4)	/* Equipment Check			*/
#define	SE		(1<<5)	/* Seek end				*/
#define	IC		(0xC0)	/* Interrupt Condition			*/

/* Status Register 1:							*/
#define	MA		(1<<0)	/* Missing Address Mark			*/
#define	NW		(1<<1)	/* Not writable				*/
#define	ND		(1<<2)	/* No Data				*/
#define	OR		(1<<4)	/* Overrun				*/
#define	DE		(1<<5)	/* Data Error				*/
#define	EN		(1<<7)	/* End of Cylinder			*/

#define BADSR1	(EN|DE|OR|ND|NW|MA)	/* Something wrong		*/

/* Status Register 2:							*/
#define	MD		(1<<0)	/* Missing Address Mark in Data Field	*/
#define	BC		(1<<1)	/* Bad Cylinder				*/
#define	SN		(1<<2)	/* Scan Not Satisfied			*/
#define	SH		(1<<3)	/* Scan Equal Hit			*/
#define	WC		(1<<4)	/* Wrong Cylinder			*/
#define	DD		(1<<5)	/* Data Error in Data Field		*/
#define	CM		(1<<6)	/* Control Mark				*/

/* Status Register 3:							*/
#define	SR3US0	(1<<0)	/* Unit Select 0, Status Reg 3		*/
#define	SR3US1	(1<<1)	/* Unit Select 1, Status Reg 3		*/
#define	HD3		(1<<2)	/* Head Address,  Status Reg 3		*/
#define	TS		(1<<3)	/* Two Side				*/
#define	T0		(1<<4)	/* Track 0				*/
#define	RDY		(1<<5)	/* Ready				*/
#define	WP		(1<<6)	/* Write Protect			*/
#define	FT		(1<<7)	/* Fault				*/

/* Bit definitions for the Digital Output Register (DOR; B:p11)		*/
#define	DRVASEL	0		/* Select Drive A, bit 0		*/
#define	DRVBSEL	1		/* Select Drive B, bit 0		*/
#define	FLRESET	(1<<2)	/* Reset Floppy Device			*/
#define	EDIDMA	(1<<3)	/* Enable Diskette irq's and DMA	*/
#define	DRIVEAE	(1<<4)	/* Drive A Motor Enable			*/
#define	DRIVEBE	(1<<5)	/* Drive B Motor Enable			*/

#define	OK		1
#define	FAIL		0

/* State machine definitions. Driven by b_active.
 * For the 765, one should do a SEEK, and READID before every transfer
 * request. In order to handle this overhead, b_active has two fields.
 * The low four bits are the main field; XFER represents an active R/W
 * is in progress, FLCMD indicates a special command. IRQ is for
 * servicing in interrupt. The high four bits represent the substates,
 * such as doing a SEEK and READID before every r/w. When LIVE_READID is
 * set, the main state is properly initialized.
 *
 * For multitrack xfer's, a cache is used.
 */
#define	IDLE		0
#define	IRQ		(1<<0)	/* Have picked up an irq from the FDC	*/
#define	FLCMD		(1<<1)	/* Special command being serviced	*/
#define	XFER		(1<<2)	/* Transfer req being processed		*/
#define	FLMORE		(1<<3)	/* Active stream of requests		*/
#define	GETSTATUS	(1<<4)	/* Do a SIS upon irq			*/
#define	LIVE_READID	(1<<5)	/* Read id substate			*/
#define	LIVE_SEEK	(1<<6)	/* Currently simulating an implied seek	*/
#define	SUBSTATE	(fdtab.b_active & 0x70)

#define	CPHASE	0	/* Command Phase			*/
#define	RPHASE	1	/* Result Phase				*/

/* Definitions for minor dev's (see fdinit())				*/
#define	SPT9	(1<<0)		/* 9 spt 				*/
#define	SIDES	(1<<1)		/* 1 = double sided			*/
#define	TPI96	(1<<2)		/* 96 tracks per inch			*/
#define	FLUNIT	(1<<3)		/* lower drive				*/
#define	SPT9_15	(1<<4)		/* 15 sector per track floppy		*/
#define	CNTRLR1	(1<<5)		/* second floppy card			*/
#define	SSTEP	(1<<6)		/* single stepping			*/
#define	FSOFF	(1<<7)		/* start on cylinder 1			*/
#define	DS15SPT	7		/* 15 spt, dbl sided, dbl den		*/

#ifdef MP386
#define 	DEV		((fdupfix(bp->b_dev) & FLUNIT) >> 3)
#else
#define 	DEV		((bp->b_dev & FLUNIT) >> 3)
#endif /* MP386 */
#define	UNIT(X)	((unsigned char) (((X) & FLUNIT) >> 3))
#define	RWRETRY	25	/* Number of retries for r/w		*/
#define	SKRETRY	0	/* Number of retries for seek		*/
#define	RIRETRY	0	/* Number of retries for readid		*/

struct	cmd {		/* Special irq-driven commands for controller	*/
	unsigned char cmd;	/* command code				*/
	unsigned char p[8];	/* parameter bytes			*/
	unsigned char sts;	/* main status byte			*/
	unsigned char s[8];	/* other status bytes			*/
	unsigned char np;	/* # of parameters			*/ 
	unsigned char ns;	/* # of returned values			*/
};

/* Parameters for the floppy devices					*/
struct fdparams {
	unsigned char trrate;	/* data transmission rate		*/
	unsigned char spt;	/* sectors per track			*/
	unsigned char tpc;	/* tracks per cylinder			*/
	unsigned char ncyl;	/* # of cylinders			*/
	unsigned short nbps;	/* # of bytes per sector		*/
	unsigned char nsec;	/* code for bytes per sector		*/
	unsigned char eot;	/* end of track				*/
	unsigned char gpl;	/* gap length				*/
	unsigned char dtl;	/* data transmission length		*/
	unsigned char spc;	/* sectors per cylinder			*/
	unsigned char step;	/* single step = 1, double step = 2	*/
	unsigned char fgpl;	/* format gap length			*/
	unsigned char fsoff;	/* offset, in cylinders			*/
	daddr_t limit;		/* max # of blocks accessible		*/
	};

struct results {
	unsigned char r[9];
};

struct fdfmt {
	unsigned char c, h, r, n;
};

struct  iobuf  fdtab;	/* unix block device coupling, req state info 	*/
struct  buf	rfdbuf;	/* private buffer header for raw i/o		*/
struct  buf	cfdbuf;	/* private buffer header for special functions	*/
struct  cmd	fdcmd;	/* command buffer, used with cfdbuf		*/


static char	dorstat;	/* Reflects the current state of the DOR*/
static unsigned char fdsel[] = { DRIVEAE|DRVASEL, DRIVEBE|DRVBSEL };

/*
 * Other 765 ioctl mnemonics and switches
 */
#define	WPCHECK				/* M004:   turn on fdwrtchk()	*/
#define	DEBUG
#ifdef	DEBUG
#define DBG(l, p)	if (dbglvl & (1<<l)) { p; }
#else
#define DBG(l, p)
#endif /* DEBUG */
#define	DBG1(p)		DBG(1, p)		/* open			*/
#define	DBG2(p)		DBG(2, p)		/* close 		*/
#define	DBG3(p)		DBG(3, p)		/* fdintr 		*/
#define	DBG4(p)		DBG(4, p)		/* fdstart 		*/
#define	DBG5(p)		DBG(5, p)		/* multitrack 		*/
#define	DBG6(p)		DBG(6, p)		/* cyl/sec ordering	*/
#define	DBG7(p)		DBG(7, p)		/* ioctl 		*/
#define	DBG8(p)		DBG(8, p)		/* ioctl 		*/
#define	DBG9(p)		DBG(9, p)		/* ioctl 		*/
#define	DBG10(p)		DBG(10, p)	/* fdinit 		*/
#define	DBG11(p)		DBG(11, p)	/* fdspecify 		*/
#define	DBG12(p)		DBG(12, p)	/* fdseek 		*/
#define	DBG13(p)		DBG(13, p)	/* strategy specific	*/
#define	DBG14(p)		DBG(14, p)	/* fdreset 		*/
#define	DBG15(p)		DBG(15, p)	/* alternate buffer 	*/

#define	INITMASK	0x57			/* bits for reinit	*/

#define	FLBSIZE	512			/* 765 cache size	*/

/* The cyl structure represents an image of the cylinder that the 
 * head is currently positioned over. It is used to implement full track
 * read and writes. c represents the current cylinder and head.
 *
 * The track handling is done in the start state machine. If the
 * current request is in fdbuf, then fdstart() will xfer the data directly
 * to/from the fdbuf array.
 */
#define MAXCSIZE	30	/* 15 spt * 2 heads			*/
#define MAXHEADS	2	/* # of heads				*/
#define NSEC		15	/* max # of sectors per track.		*/
#define NUNITS		2	/* # of units				*/

struct cache {
	unsigned char sts;	/* current cache status - see defines below */
	uint c;			/* cylinder that tbuf represents	*/
	uint h;			/* head that tbuf represents	*/
	unsigned char *cp;	/* ptr to data buffer		*/
} cache;

/* defines for the cache status: */
#define FLCRD	  (1<<0)	/* updating track from disk. head position in c, h */
#define FLCWR	  (1<<1)	/* flushing track to disk. head position in c,h */
#define FLCDIRTY (1<<2)	/* cache needs flushing before head moves */
#define FLSYNC	  (1<<3)	/* cache is being synced to disk */

#ifdef MP386
#define splbio() spl6()
#endif
