/*      Copyright (c) 1985 AT&T */
/*        All Rights Reserved   */

/*      THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T     */
/*      The copyright notice above does not evidence any        */
/*      actual or intended publication of such source code.     */

/* @(#)8259.h	1.8 */
/*
** constants and definitions for the
** 8259 Programmable Interrupt Controller (PIC)
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
#define	MSLAVE_HERE	0x80		/* master connect to slave here	*/
#define	SSLAVE_HERE	0x07		/* slave connect to master here	*/

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
#define	MICW2		MBASE
#define	MICW3		MSLAVE_HERE
#define	MICW4		MASTER | BUFFERED | MODE_8086
/* slave */
#define	SICW1		ALWAYS_SET | NEED_ICW4 | ADI
#define	SICW2		SBASE
#define	SICW3		SSLAVE_HERE
#define	SICW4		BUFFERED | MODE_8086

/*
** port addresses
*/
#define	PICMSTAT	0x00C0		/* address of master PIC status port */
#define	PICMASTER	0x00C2		/* address of master PIC level port  */
#define	PICSSTAT	0x00C4		/* address of slave PIC status port  */
#define	PICSLAVE	0x00C6		/* address of slave PIC level port   */

/*
** bit definitions for master interrupt masks
*/
#define	ML0		0x01		/* clock timer chip	*/
#define	ML1		0x02		/* front panel I button	*/
#define	ML2		0x04		/* undefined		*/
#define	ML3		0x08		/* 544 sio controller	*/
#define	ML4		0x10		/* undefined		*/
#define	ML5		0x20		/* 215 disk controller	*/
#define	ML6		0x40		/* 8274 console		*/
#define	ML7		0x80		/* slave interrupt	*/

/*
** bit definitions for slave interrupt masks
*/
#define	SL0		0x01		/* undefined		*/
#define	SL1		0x02		/* undefined		*/
#define	SL2		0x04		/* undefined		*/
#define	SL3		0x08		/* undefined		*/
#define	SL4		0x10		/* undefined		*/
#define	SL5		0x20		/* undefined		*/
#define	SL6		0x40		/* undefined		*/
#define	SL7		0x80		/* line printer		*/

/*
** composite masks to send to the master PIC
** NOTE: Bit set disables the interrupt,
**	 Bit clear enables the interrupt.
*/
#define	SPL0MASTER	ML2 | ML4
#define	SPL4MASTER	SPL0MASTER | ML6 | ML7
#define	SPL5MASTER	SPL4MASTER | ML3
#define	SPL6MASTER	SPL5MASTER | ML5
#define	SPL7MASTER	SPL6MASTER | ML0

/*
** composite masks to send to slave PIC
*/
#define	SPL0SLAVE	SL0 | SL1 | SL2 | SL3 | SL4 | SL5 | SL6
#define	SPL4SLAVE	SPL0SLAVE | SL7
#define	SPL5SLAVE	SPL4SLAVE
#define	SPL6SLAVE	SPL5SLAVE
#define	SPL7SLAVE	SPL6SLAVE
