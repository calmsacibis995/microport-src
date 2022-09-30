/* uportid = "@(#)8237.c	Microport Rev Id  2.4 5/6/87" */
/*                                                                            */
/* Microport 8237 DMA Controller Access 				      */
/*                                                                            */
/* Copyright UNIX NET Ltd. 1987	                                              */
/* All Rights Reserved                                                        */
/* The following Source Code is Proprietary and Confidential.                 */
/* The copyright notice above does not evidence any                           */
/* actual or intended publication of such Source Code.                        */
/*                                                                            */
/* Distribution of this Source Code is protected by your Company signed       */
/* "Source Code License Agreement" which restricts the storage of any         */
/* portion or modification of this Source Code to a single non-volatile       */
/* non-removable random access device (i.e. hard disk) per license.           */
/* PLEASE don't copy this onto your hard disk unless you're Company has       */
/* registered the model and serial number as "Designated Equipment" per       */
/* the Agreement.                                                             */
/*                                                                            */
/* A reward of up to 10% of recovered damages is offered for information      */
/* leading to prosecution or settlement of Source Code License violations.    */
/*                                                                            */
/* MODIFICATION HISTORY:  (see evlock for those source files being modified)  */
/*                                                                            */
/*                                                                            */
/*
 * References:
 *	1) IBM PC/AT Technical Reference. IBM, Sept. 1985.
 *		pps 1-9 to 1-11
 *	2) Intel Microsystem and Component Handbook. Intel, 1984.
 *		pps 2-90 to 2-95 (8237A).
 *
 * Initial Coding:
 *	baysw!doug Sun Jan 18 14:13:45 PST 1987
 *
 * Modification History:
 *
 * To do:
 *	1) Support of modes other than SINGLEMODE
 *	2) incorporate sharing the 8237 dma channels with other drivers
 *	3) Document above in linkkit
 */

#include "sys/types.h"
#include "sys/param.h"
#include "sys/8237.h"
#include "sys/buf.h"
/*
 * Debug level defines for DMA interface information
 */
#ifndef	DBG
# ifdef	DEBUG
#  define DBG(l, p)	if (dmadblvl & (1<<l)) { p; }
# else	not DEBUG
#  define DBG(l, p)
# endif	DEBUG
#endif	DBG
#define	DBG0(p)		DBG(0, p)
#define	DBG1(p)		DBG(1, p)
#define	DBG2(p)		DBG(2, p)
#define	DBG3(p)		DBG(3, p)
#define	DBG4(p)		DBG(4, p)
#define	DBG5(p)		DBG(5, p)
#define	DBG6(p)		DBG(6, p)
#define	DBG7(p)		DBG(7, p)
#define	DBG8(p)		DBG(8, p)
#define	DBG9(p)		DBG(9, p)
#define	DBG10(p)	DBG(10, p)
#define	DBG11(p)	DBG(11, p)
#define	DBG12(p)	DBG(12, p)
#define	DBG13(p)	DBG(13, p)
#define	DBG14(p)	DBG(14, p)
#define	DBG15(p)	DBG(15, p)

#define DMASPL	spl7()

#ifndef TERMCNTMSK
#define TERMCNTMSK	0x0f		/* only look at TC bits */
#endif

/* return codes for kernel dma functions */
#ifndef EDMAFAULT	
#define EDMAFAULT	0xf		/* Block un-aligned address */
#endif

#ifndef EDMACNT	
#define EDMACNT		0xf0		/* Rollover Address */
#endif

#ifndef DMAOK
#define DMAOK		0		/* Successful DMA operation */
#endif

int	dmadblvl = 0;			/* patchabel debug level */

/*
** tables of DMA base address for each channel
*/
unsigned char dmaadr[] =	/* DMA base and current address table	*/
{
       	D1C0BCA,	/* DMA #1, Ch. 0 Base & Current Addr.	*/
       	D1C1CAR,	/* DMA #1, Ch. 1 Base & Current Addr	*/
       	D1C2CAR,	/* DMA #1, Ch. 2 Base & Current Addr	*/
       	D1C3CAR,	/* DMA #1, Ch. 3 Base & Current Addr	*/
	D2C0BCA,	/* DMA #2, Ch. 0 Base & Current Addr.	*/
	D2C1BCA,	/* DMA #2, Ch. 1 Base & Current Addr	*/
	D2C2BCA,	/* DMA #2, Ch. 2 Base & Current Addr	*/
	D2C3BCA,	/* DMA #2, Ch. 3 Base & Current Addr	*/
};

unsigned char dmacnt[] =	/* DMA base and current count table	*/
{
       	D1C0BCW,	/* DMA #1, Ch. 0 Base & Current Wrd Cnt */
       	D1C1BCW,	/* DMA #1, Ch. 1 Base & Current Wrd Cnt */
       	D1C2BCW,	/* DMA #1, Ch. 2 Base & Current Wrd Cnt */
       	D1C3BCW,	/* DMA #1, Ch. 3 Base & Current Wrd Cnt */
	D2C0BCW,	/* DMA #2, Ch. 0 Base & Current Wrd Cnt */
	D2C1BCW,	/* DMA #2, Ch. 1 Base & Current Wrd Cnt */
	D2C2BCW,	/* DMA #2, Ch. 2 Base & Current Wrd Cnt */
	D2C3BCW 	/* DMA #2, Ch. 3 Base & Current Wrd Cnt */
};

unsigned char dmapgr[] =	/* DMA page address table		*/
{
	DMAPR0,		/* channel 0				*/
	DMAPR1,		/* channel 1				*/
	DMAPR2,		/* channel 2				*/
	DMAPR3,		/* channel 3				*/
	0,		/* channel 4 is used in cascade mode	*/
	DMAPR5,		/* channel 5				*/
	DMAPR6,		/* channel 6				*/
	DMAPR7		/* channel 7				*/
};

unsigned char dmabpff[] = 		/* Clear Byte Pointer Flip/Flop	*/
{
	CBPFF1,			/* DMA #1 */ 
	CBPFF2			/* DMA #2 */ 
};

unsigned char dmawmr[] =
{
	MODE1,			/* Write Mode Register DMA #1	*/
	MODE2			/* Write Mode Register DMA #2	*/
};

unsigned char dmawsmr[] =		/* Write Single Mask Register Bit*/
{
	MASK1,			/* DMA #1 */ 
	MASK2			/* DMA #2 */ 
};

unsigned char dmardst[] =		/* Read Status Port*/
{
	STAT1,			/* DMA #1 */ 
	STAT2			/* DMA #2 */ 
};

/*
 * 	8237  State Variables
 */
unsigned char dmastat[] =		/* DMA Status Register values	*/
{
	0,			/* DMA #1 */ 
	0			/* DMA #2 */ 
};

unsigned char dmasmr[] =	/* DMA Single Mask Reg values	*/
{
	0,		/* channel 0				*/
	0,		/* channel 1				*/
	0,		/* channel 2				*/
	0,		/* channel 3				*/
	0,		/* channel 4 is used in cascade mode	*/
	0,		/* channel 5				*/
	0,		/* channel 6				*/
	0		/* channel 7				*/
};

/*
 * opendma (channel)
 *		Request the use of the DMA channel 'channel'.
 *		If the channel is not currently in use, a DMA descriptor
 *		will be returned as an 'int', otherwise a -1 is returned.
 */
static	unsigned char	o_channels = 0;

opendma (channel)
	int	channel;
{
	if ((1 << channel) & o_channels)
		return (-1);
	o_channels |= (unsigned char) (1 << channel);
	return (channel |= 0x100);
}

/*
 * closedma (dmad)
 *		Free the use of the DMA channel refered to by the
 *		DMA descriptor 'dmad'.
 */
closedma (dmad)
	int	dmad;
{
	unsigned char	chanmask = 1 << (dmad & 0xff);
	if (!(chanmask & o_channels))
		return;
	o_channels &=  ~chanmask;
}

/*
 * setdma():    Provide a descriptor interface to the 8237
 * 		dma controller setup routine
 */
setdma(dmad, mode, addr, count)
	int	dmad, count;
	char	mode;
	unsigned long addr;
{
	unsigned long	lcnt = count;
	unsigned char	chan = (dmad & 0xff);
	unsigned char	_setdma();

	if ( _setdma(chan, (unsigned char) mode, addr, &lcnt) )
		return(-1);
	_enabledma(chan);
	return(0);
}

/*
 * resdma():    Returns the residual count of a DMA transfer
 */
resdma(dmad)
	int	dmad;
{
	unsigned char	chan = (dmad & 0xff);
	unsigned long	_resdma();
	return( (int) _resdma(chan) );
}

/*
 * _dmaok():    Check the dma request for errors before setting the 8237
 *		dma controller chip for a transfer.
 *   input:     char channel  0,1,2,3,5,6 or 7
 *		char mode     (see Mode Register Control Bits in 8237.h)
 *			      Channel bits should be excluded.
 *			      Only SINGLEMODE currently supported.
 *		paddr_t badr physical address to DMA to/from (byte addr)
 *		unsigned long *cntp  ptr to long DMA xfer count (bytes)
 *		(Note this is shifted to make a word count for chs 5-7)
 *
 *   returns	DMAOK if dma request is valid as is
 *		EDMAFAULT if the host address is invalid
 *		EDMACNT  if the host count is too large
 *		      *cntp is reduced to max possible count when
 *		      EDMACNT is returned. 
 */
unsigned char
_dmaok(channel, mode, badr, cntp)
unsigned char channel, mode;
paddr_t badr;
unsigned long *cntp;
{
    unsigned char mask, labyte, habyte, lowcount, hicount, page; 
    unsigned short count, x;
    paddr_t dma_addr;

    DBG11(printf("_dmaok: channel = %x, mode = %x, badr = %lx, count = %lx\n",
	channel,mode,badr,*cntp));
    if ((channel > 4) && (badr & 1))	/*  Even addr on chs 5,6,7 */
	{			
	    DBG0(printf("_dmaok: ERROR odd address= %lx\n",badr));
	    return(EDMAFAULT);
	}
    if (channel>4)		 /* shift word addr  and count */
	{
	dma_addr = badr >> 1;
	count = (unsigned short)(*cntp >> 1);
	page  = (lobyte(hiword(badr)))&0xfe; /* hi  8 of addr        */
	}
    else
	{
	dma_addr = badr;	
	count = (unsigned short)*cntp;
	page  = (lobyte(hiword(badr)))&0xff; /* hi  8 of addr        */
	}
    labyte = lobyte(loword(dma_addr));/* low 8 bits of CAR    */
    habyte = hibyte(loword(dma_addr));/* high 16bits of CAR   */
    count--;
    lowcount = lobyte(count);               /* # words to be xferred*/
    hicount = hibyte(count);                
    if (((unsigned short)habyte+(unsigned short)hicount+
	(((unsigned short)labyte+(unsigned short)lowcount)>0x00ff)) > 0x00FF) 
	{			     
	/* dma page rollover	*/
	hicount = (unsigned char)(0x0ff - (unsigned short)habyte);
	lowcount = (unsigned char)(0x0ff - (unsigned short)labyte);
	count=((unsigned short)hicount<<8)+(unsigned short)lowcount+1;
	*cntp = (channel>4) ? count << 1 : count;
	DBG10(printf("_dmaok: dma_addr = %lx, page = %x\n",
	    dma_addr,(unsigned char)page));
	DBG10(printf("_dmaok: DMA page rollover, new dmasize %lx, badr+count %lx\n",
	    *cntp,badr+*cntp));
	return(EDMACNT);
	}
    return(DMAOK);
}

/*
 * _setdma():    Setup the 8237 dma controller chip for a transfer.
 *   input:     char channel  0,1,2,3,5,6 or 7
 *		char mode     (see Mode Register Control Bits in 8237.h)
 *			      Channel bits should be excluded.
 *			      Only SINGLEMODE currently supported.
 *		paddr_t badr physical address to DMA to/from (byte addr)
 *		unsigned long *cntp  ptr to long DMA xfer count (bytes)
 *		(Note this is shifted to make a word count for chs 5-7)
 *
 *   returns	DMAOK if dma set
 *		EDMAFAULT if the host address is invalid
 *		EDMACNT  if the host count is too large
 *		      *cntp is reduced to max possible count when
 *		      EDMACNT is returned. 
 *   NOTE:	the 8237 is programmed only when DMAOK is returned.
 */
unsigned char
_setdma(channel, mode, badr, cntp)
unsigned char channel, mode;
paddr_t badr;
unsigned long *cntp;
{
    unsigned char ret, dmamode, mask, labyte, habyte, lowcount, hicount, page; 
    unsigned short count, x;
    paddr_t dma_addr;

    /*
     *		check request validity
     */
    if (ret = _dmaok(channel, mode, badr, cntp))
	return(ret);

    /*
     *		setup dma parameters
     */
    if (channel>4)		 /* shift word addr  and count */
	{
	dma_addr = badr >> 1;
	count = (unsigned short)(*cntp >> 1);
	page  = (lobyte(hiword(badr)))&0xfe; /* hi  8 of addr        */
	}
    else
	{
	dma_addr = badr;	
	count = (unsigned short)*cntp;
	page  = (lobyte(hiword(badr)))&0xff; /* hi  8 of addr        */
	}
    /* read operation == I/O write to memory & visa versa */
    dmamode = mode&(~CHANLBITS);
    dmamode |= (channel & CHANLBITS);
    labyte = lobyte(loword(dma_addr));/* low 8 bits of CAR    */
    habyte = hibyte(loword(dma_addr));/* high 16bits of CAR   */
    count--;
    lowcount = lobyte(count);               /* # words to be xferred*/
    hicount = hibyte(count);                

    /*
     *		diagnostics
     */
    DBG8(printf("A")); 
    DBG9(printf("_setdma: dma_addr=%lx, page=%x, dmamode=%x\n",
	dma_addr,(unsigned char)page,(unsigned char)dmamode));
    DBG10(printf("_setdma: \n");
        printf("_setdma:  ports: %x  %x  %x %x %x %x  %x %x \n",
	    dmawsmr[(channel&4)>>2], 
	    dmabpff[(channel&4)>>2],
	    dmawmr[(channel&4)>>2],
	    dmaadr[channel], 
	    dmaadr[channel],
	    dmapgr[channel],
	    dmacnt[channel],
	    dmacnt[channel]);
	printf("          data:  %x %x %x %x %x %x %x %x \n",
    	     (0x4|(channel&CHANLBITS)),
	     dmamode,
	     dmamode,
	     labyte,
	     habyte,
	     page,
	     lowcount,
	     hicount)
    );



    /*
     *		disable DMA channel 
     *
     *	dmasmr[(channel)]= 0x4|(channel&CHANLBITS);
     *	outb(dmawsmr[(channel&4)>>2], 0x4|(channel&CHANLBITS));
     */
    _disabledma(channel);

    /*
     *		program DMA channel
     */
    outb(dmabpff[(channel&4)>>2], dmamode);  /* kick the flip-flop   */
    outb(dmawmr[(channel&4)>>2], dmamode); /* Set the mode           */
    outb(dmaadr[channel], labyte);          /* Output low address     */
    outb(dmaadr[channel], habyte);          /* Output high address    */
    outb(dmapgr[channel], page); /* Out bits A23..A17 on D7..D1 to page reg.*/
    outb(dmacnt[channel], lowcount);        /* Low byte of count      */
    outb(dmacnt[channel], hicount);      /* High byte of count        */
    return(DMAOK);
}

/*
 * _resdma():       Read the 8237 dma controller residual count.
 *   input:
 *	unsigned char channel  chs 0,1,2,3,5,6, or 7
 *   output:
 *	unsigned long residual count after DMA (bytes)
 *	this is shifted to get a byte count for 16bit DMA channels
 *	5-7.
 */
unsigned long
_resdma(channel)
unsigned char channel;
{
    unsigned short x;
    unsigned short resid;

#ifdef SPLINT
    x = DMASPL;
#else 
    asm ("	pushf");
    asm ("	cli");
#endif 

    outb(dmabpff[(channel&4)>>2],1);
    resid=((unsigned short)inb(dmacnt[channel]))+1;
    resid+=((unsigned short)inb(dmacnt[channel]))<<8;
#ifdef SPLINT
    splx(x);
#else 
    asm ("	popf");
#endif 
    if (channel>4)
	return((unsigned long)resid<<1);   /* convert words to bytes */
    else
	return((unsigned long)resid);
}

/*
 * _disabledma():    Disable the 8237 dma controller chip for a transfer.
 *   input:     char channel  0,1,2,3,5,6 or 7
 *
 *   returns	VOID
 */
_disabledma(channel)
unsigned char channel;
{
    dmasmr[(channel)]= 0;
    outb(dmawsmr[(channel&4)>>2], 0x4|(channel&CHANLBITS)); /* set mask bit */
    DBG10(
        printf("_disabledma:  out %x to port %x\n", 
    	     0x4|(channel&CHANLBITS), dmawsmr[(channel&4)>>2])
	);
}

/*
 * _enabledma():    Enable the 8237 dma controller chip for a transfer.
 *   input:     char channel  0,1,2,3,5,6 or 7
 *
 *   returns	VOID
 */
_enabledma(channel)
unsigned char channel;
{
    DBG9(
        printf("_enabledma:  out %x to port %x\n", 
    	     channel&CHANLBITS, dmawsmr[(channel&4)>>2])
	);
    dmasmr[(channel)]= 1;
    DBG8(printf("E"));
    outb(dmawsmr[(channel&4)>>2], channel&CHANLBITS); /* clear bit in mask */
}

/*
 * _dmaenabled():    Test the 8237 dma controller Enable
 *   input:     char channel  0,1,2,3,5,6 or 7
 *
 *   returns	single mode mask register contents (saved in software)
 */
unsigned char 
_dmaenabled(channel)
unsigned char channel;
{
    DBG10(
        printf("_dmaenabled:  dma smr val chan %x: %x\n", 
    	     channel, dmasmr[(channel)])
	);
    return (dmasmr[(channel)]);
}

/*
 * _setdmacnt():    set count for next block
 *   input:     char channel  0,1,2,3,5,6 or 7
 *		int  count
 *
 *   returns	none - no error checking done here
 */
unsigned char 
_setdmacnt(channel,mode,lowcount,hicount)
unsigned char channel, mode, lowcount, hicount;
{
    unsigned char dmamode;

    DBG8(printf("F"));
    dmamode = mode&(~CHANLBITS);
    dmamode |= (channel & CHANLBITS);
    DBG11(
        printf("_setdmacnt:  chan %x mode %x lo cnt %x hi cnt %x\n",
	    channel, dmamode, lowcount, hicount)
	);
    outb(dmabpff[(channel&4)>>2], dmamode);  /* kick the flip-flop   */
    outb(dmacnt[channel], lowcount);        /* Low byte of count      */
    outb(dmacnt[channel], hicount);      /* High byte of count        */
    return(DMAOK);
}

/*
 * _dmastatus():   return the status register for DMA 1 or 2 masked
 *		   for channel of caller.
 *   input:     char channel  0,1,2,3,5,6 or 7
 *
 *   returns	unsigned char with following bit definitions:
 *			0 -> 3 set (1) if Terminal Count condition
 *			4 -> 7 set (1) if DMA request set 
 */
unsigned char 
_dmastatus(channel)
unsigned char channel;
{
    unsigned char chanmask=0x11;	/* default of channel 0 */
    unsigned char stat;

    DBG8(printf("X"));
    DBG10(
        printf("_dmastatus:  dma status val chan %x: %x\n", 
    	     channel, dmastat[(channel&0x4)>>2]));
    chanmask=(0x11<<(channel&CHANLBITS)); /* 11=chan 0/4 .. 88=chan 3/7 */
    dmastat[(channel&4)>>2] &= (TERMCNTMSK); /* save ONLY previous TCs */
    /* or on new TCs and any requests */
    dmastat[(channel&4)>>2] |= inb(dmardst[(channel&4)>>2]);
    stat=(dmastat[(channel&4)>>2]&chanmask); /* mask off other channels */
    DBG10(
	printf("_dmastatus:  chan %x: %x, status reg: %x\n", 
	     channel, stat,dmastat[(channel&4)>>2]));
    if (stat&TERMCNTMSK)			/* did we get a TC? */
	{
	/* if so, turn off read TC bit */
	dmastat[(channel&4)>>2] ^= (chanmask&TERMCNTMSK);
	}
    return (stat);
}
