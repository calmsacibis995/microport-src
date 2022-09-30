/* uportid = "@(#)8259.h	Microport Rev Id  2.3 7/29/87" */
/*      Copyright (c) 1985 AT&T */
/*        All Rights Reserved   */

/*      THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T     */
/*      The copyright notice above does not evidence any        */
/*      actual or intended publication of such source code.     */

/* sccid = @(#)8259.h	1.3 */
/*
** constants and definitions for the
** 8259 Programmable Interrupt Controller (PIC)
*/

/*
 * Modification History:
 *	uport!dwight Sun Feb 16 15:58:10 PST 1986
 * Upgraded for the IBM AT. Taken completely from the previous stl version
 * M000: lance rearranged for SIO problem.  Should allow variable
 * 		SPL masks from config(1) for link kit.
 * M001: lance - allowed SL5 (coprocessor) to int at level 0.
 *		This allows illegal number ops to cause errors.
 * M002: lance - 6-26-86
 *		Rearranged again for SIO, made keyboard spl6 and sio spl7.
 * M003:	uport!dwight	Tue Dec  2 19:34:59 PST 1986
 *	Added support for tape drive.
 * M004:	rex!uport	Wed Jul 29 19:13:23 PDT 1987
 *		Added SFNM to MICW4 if PICFIX2 is #defined
 */

/*
** bit definitions for Initialization Command Words 1 and 2
*/
#define	NEED_ICW4	0x01		/* ICW4 is needed		*/
#define	SNGL		0x02		/* single mode (reset = cascade)*/
#define	ADI		0x04		/* address interval		*/
#define	LTIM		0x08		/* level triggered mode		*/
#define	ALWAYS_SET	0x10		/* this bit is always set	*/

/*
** bit definitions for Initialization Command Word 3
*/
#define	MSLAVE_HERE	0x04		/* master connect to slave here	*/
#define	SSLAVE_HERE	0x02		/* slave connect to master here	*/

/*
** bit definitions for Initialization Command Word 4
*/
#define	MODE_8086	0x01		/* 8086 mode of operation	*/
#define	AUTOEOI		0x02		/* automatically generated eoi	*/
#define	MASTER		0x04		/* PIC is master		*/
#define	BUFFERED	0x08		/* buffered mode enabled	*/
#define	SFNM		0x10		/* special fully nested mode	*/

/*
** bit definitions for Operational Command Word 2
*/
#define	EOI		0x20		/* end-of-interrupt		*/
#define	SPECIFIC_EOI	0x40		/* enables specific eoi mode	*/
#define	ROTATE		0x80		/* rotate on eoi		*/

/*
** base locations for the PICs to interrupt at
*/
#define	MBASE		0x20		/* 0x20 to 0x27			*/
#define	SBASE		0x28		/* 0x28 to 0x2F			*/

/*
** combinatorial bit defines for initialization of the PICs
*/
/* master */
#define	MICW1		ALWAYS_SET | NEED_ICW4 | ADI

#ifdef ATMERGE
#define MICW2		0x08   /* set master base to what DOS expects */
#else
#define	MICW2		MBASE
#endif /* ATMERGE */

#define	MICW3		MSLAVE_HERE
#ifdef	PICFIX2
#define	MICW4		MASTER | BUFFERED | MODE_8086 | SFNM	/* M004 */
#else	/* ! PICFIX2 */
#define	MICW4		MASTER | BUFFERED | MODE_8086
#endif	/* ! PICFIX2 */
/* slave */
#define	SICW1		ALWAYS_SET | NEED_ICW4 | ADI

#ifdef ATMERGE 
#define SICW2		0x70    /* set slave base to what DOS expects */
#else
#define	SICW2		SBASE
#endif /* ATMERGE */

#define	SICW3		SSLAVE_HERE
#define	SICW4		BUFFERED | MODE_8086

/*
** port addresses
*/
#define	PICMSTAT	0x0020		/* address of master PIC status port */
#define	PICMASTER	0x0021		/* address of master PIC level port  */
#define	PICSSTAT	0x00A0		/* address of slave PIC status port  */
#define	PICSLAVE	0x00A1		/* address of slave PIC level port   */

/*
** bit definitions for master interrupt masks
*/
#define	ML0		0x01		/* clock timer chip	*/
#define	ML1		0x02		/* keyboard irq 	*/
#define	ML2		0x04		/* slave pic		*/
#define	ML3		0x08		/* sio controller #2*/
#define	ML4		0x10		/* sio controller #1*/
#define	ML5		0x20		/* parallel port #2	*/
#define	ML6		0x40		/* diskette controller	*/
#define	ML7		0x80		/* parallel port #1	*/

/*
** bit definitions for slave interrupt masks
*/
#define	SL0		0x01		/* real time clock	*/
#define	SL1		0x02		/* software (irq 2)	*/
#define	SL2		0x04		/* undefined		*/
#define	SL3		0x08		/* undefined		*/
#define	SL4		0x10		/* undefined		*/
#define	SL5		0x20		/* coprocessor		*/
#define	SL6		0x40		/* hard disk		*/
#define	SL7		0x80		/* line printer		*/

/*
** composite masks to send to the master PIC
** NOTE: Bit set disables the interrupt,
**	 Bit clear enables the interrupt.
*/
#define	SPL0MASTER	0			/* let a 1000 IRQs bloom*/
#define	SPL4MASTER	SPL0MASTER|ML6|ML0    	/* + clock, flop off */ /* M000 */
#define	SPL5MASTER	SPL4MASTER|ML5|ML7	/* + lp off	*/ /* M000 */
#define	SPL6MASTER	SPL5MASTER|ML1		/* + kb off */ /* M000,1 */
#define	SPL7MASTER	SPL6MASTER|ML3|ML4|ML2	/* + sio, slave off */ /* M000,1 */

/*
** composite masks to send to slave PIC
*/
#ifdef	ATMERGE
# define SPL0SLAVE	SL1|SL2|SL3|SL4
# define SPL4SLAVE	SL0|SPL0SLAVE|SL5|SL6	/* rtc + coproc + hdisk */
#else /* -ATMERGE */
#define	SPL0SLAVE	SL0|SL1|SL2|SL3|SL4		/* M003 */
#define	SPL4SLAVE	SPL0SLAVE|SL5|SL6		/* coproc + wini off */ /* M001 */
#endif /* ATMERGE */

#define	SPL5SLAVE	SPL4SLAVE /* M000 */
#define	SPL6SLAVE	SPL5SLAVE /* M000 */
#define	SPL7SLAVE	SPL6SLAVE|SL7			/* M003 */

#ifdef LCCFIX
/* For hierarchical spl levels based on current interrupt */
/* Make sure SPL?MASTER and SPL?SLAVE defines agree with */
/* any changes made here or you'll probably be sorry... */
#define	IR0	4	/* dosclock */
#define	IR1	6	/* keyboard */
#define	IR2	7	/* slave pic */
#define	IR3	7	/* sio 2 */
#define	IR4	7	/* sio 1 */
#define	IR5	5	/* lpt 2 */
#define	IR6	4	/* floppy */
#define	IR7	5	/* lpt 1 */
#define	IR8	4	/* unixclock */
#define	IR9	7	/* software (irq 2) */	/* M004: was 0 */
#define	IR10	7	/* undefined */		/* M004: was 0 */
#define	IR11	7	/* undefined */		/* M004: was 0 */
#define	IR12	7	/* undefined */		/* M004: was 0 */
#define	IR13	4	/* coprocessor */
#define	IR14	4	/* wini */
#define	IR15	7	/* lp */
#endif /* LCCFIX */
