/* uportid = "@(#)8237.h	Microport Rev Id  1.3.3 6/18/86" */

/* Copyright 1985 by Microport. All Rights Reserved.
 *
 * Include file for the Intel 8237 DMA Driver 
 * References:
 *	1) IBM PC/AT Technical Reference. IBM, Sept. 1985.
 *		pps 1-9 to 1-11
 *	2) Intel Microsystem and Component Handbook. Intel, 1984.
 *		pps 2-90 to 2-95 (8237A).
 *	3) IBM Technical Reference Manual for the AT, March 1984
 *		C: Fixed Disk and Diskette Adapter (Appendix)
 *
 * Initial Coding:
 *		uport!dwight	 Tue Oct 22 12:22:23 PDT 1985
 *
 * Modification History:
 *		uport!doug Sun Jan 18 14:13:45 PST 1987
 *		1st version supports SINGLEMODE only.
 *
 */

/* Definitions for the DMA ports:					*/
#define	DMA1	0x00		/* Base Address of DMA controller #1	*/
#define	DMA2	0xC0		/* Base Address of DMA controller #2	*/

/* Base, Current Address and Current Word count locations: slave dma	*/
#define	D1C0BCA	0x00		/* DMA #1, Ch. 0 Base & Current Addr.	*/
#define	D1C0BCW	0x01		/* DMA #1, Ch. 0 Base & Current Wrd Cnt */
#define	D1C1CAR	0x02		/* DMA #1, Ch. 1 Base & Current Addr	*/
#define	D1C1BCW	0x03		/* DMA #1, Ch. 1 Base & Current Wrd Cnt */
#define	D1C2CAR	0x04		/* DMA #1, Ch. 2 Base & Current Addr	*/
#define	D1C2BCW	0x05		/* DMA #1, Ch. 2 Base & Current Wrd Cnt */
#define	D1C3CAR	0x06		/* DMA #1, Ch. 3 Base & Current Addr	*/
#define	D1C3BCW	0x07		/* DMA #1, Ch. 3 Base & Current Wrd Cnt */

/* Address codes for software commands					*/
#define	C1BASE	0x08		/* Command Base for DMA chip #1		*/
#define	CMD1	0x08		/* Write Command Register		*/
#define	STAT1	0x08		/* Read Status Register			*/
#define	REQ1	0x09		/* Write Request Register		*/
#define	MASK1	0x0A		/* Write Single Mask Register Bit	*/
#define	MODE1	0x0B		/* Write Mode Register			*/
#define	CBPFF1	0x0C		/* Clear Byte Pointer Flip/Flop		*/
#define	MSTCLR1	0x0D		/* Reset DMA #1 (write only)		*/
#define	RTEMP1	0x0D		/* Read Tempory Register		*/
#define	MCLR1	0x0E		/* Clear Mask Register			*/
#define	WAMRB1	0x0F		/* Write all mask register bits		*/

/* Base, Current Address and Current Word count locations: master dma	*/
#define	D2C0BCA	0xC0		/* DMA #2, Ch. 0 Base & Current Addr.	*/
#define	D2C0BCW	0xC2		/* DMA #2, Ch. 0 Base & Current Wrd Cnt */
#define	D2C1BCA	0xC4		/* DMA #2, Ch. 1 Base & Current Addr	*/
#define	D2C1BCW	0xC6		/* DMA #2, Ch. 1 Base & Current Wrd Cnt */
#define	D2C2BCA	0xC8		/* DMA #2, Ch. 2 Base & Current Addr	*/
#define	D2C2BCW	0xCA		/* DMA #2, Ch. 2 Base & Current Wrd Cnt */
#define	D2C3BCA	0xCC		/* DMA #2, Ch. 3 Base & Current Addr	*/
#define	D2C3BCW	0xCE		/* DMA #2, Ch. 3 Base & Current Wrd Cnt */

/* Address codes for software commands: master dma			*/
#define	C2BASE	0xD0		/* Command Base addr for DMA chip #2	*/
#define	CMD2	0xD0		/* Write Command Register		*/
#define	STAT2	0xD0		/* Read Status Register			*/
#define	REQ2	0xD2		/* Write Request Register		*/
#define	MASK2	0xD4		/* Write Single Mask Register Bit	*/
#define	MODE2	0xD6		/* Write Mode Register			*/
#define	CBPFF2	0xD8		/* Clear Byte Pointer Flip/Flop		*/
#define	MSTCLR2	0xDA		/* Reset DMA #2 (write only)		*/
#define	RTEMP2	0xDA		/* Read Tempory Register		*/
#define	MCLR2	0xDC		/* Clear Mask Register			*/
#define	WAMRB2	0xDE		/* Write all mask register bits		*/

/* Addresses of the DMA Page Registers:					*/
#define	DMAPR0	0x87		/* channel 0				*/
#define	DMAPR1	0x83		/* channel 1				*/
#define	DMAPR2	0x81		/* channel 2				*/
#define	DMAPR3	0x82		/* channel 3				*/
#define	DMAPR5	0x8B		/* channel 5				*/
#define	DMAPR6	0x89		/* channel 6				*/
#define	DMAPR7	0x8A		/* channel 7				*/
#define	DMARF	0x8F		/* DMA Refresh				*/

/* Bit definitions for the DMA mode register:				*/
#define	FLDMARD	0x46		/* Read from floppy			*/
#define	FLDMAWR	0x4A		/* Write to floppy			*/

/* Mode Control Bit Definitions 				*/
/* use these to make mode passed to setdma()        		*/

#define VERIFYMEM	(0<<2)		/* verify transfer */
#define WRITEMEM	(1<<2)		/* write transfer */
#define READMEM		(2<<2)		/* read transfer */

#define AUTODISABLE	(0<<4)		/* Autoinitialization disable */
#define AUTOENABLE	(1<<4)		/* Autoinitialization enable */

#define ADRINC		(0<<5)		/* Address Increment select */
#define ADRDEC		(1<<5)		/* Address Decrement select */

#define DEMANDMODE	(0<<6)		/* Demand mode select       */
#define SINGLEMODE	(1<<6)		/* Single mode mode select  */
#define BLOCKMODE	(2<<6)		/* Block mode select        */
#define CASCADEMODE	(3<<6)		/* Cascade mode select      */

/* masks for the DMA channels (0,1,2,3,5,6, or 7)		    */
#define CHANLBITS	0x3		/* mask channel bits in channel*/

/* Mask bit defines */
#define MASKB		0x04		/* set mask bit for channel # */
#define UNMASKB		0x0		/* clear mask bit for channel # */

/* Macros for DMA channel to/from DMA descriptor conversion */
#define	DMADMAGIC	0x100
#define	DMAMAGMSK	0xff
#define	DMACTOD(c)	(c |= DMADMAGIC)
#define	DMADTOC(d)	((unsigned char) d & DMAMAGMSK)

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
