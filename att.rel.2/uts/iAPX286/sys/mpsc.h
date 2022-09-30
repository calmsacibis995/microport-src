/*      Copyright (c) 1985 AT&T */
/*        All Rights Reserved   */

/*      THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T     */
/*      The copyright notice above does not evidence any        */
/*      actual or intended publication of such source code.     */

/*
 * @(#)mpsc.h	1.3
 * definition of 8274 MPSC constants
 *
 */
#define	M_CHA	0x10		/* channel A bit (unshifted) */
#define	M_REG0	0x00		/* select reg 0 */
#define	M_REG1	0x01		/* select reg 1 */
#define	M_REG2	0x02		/* select reg 2 */
#define	M_REG3	0x03		/* select reg 3 */
#define	M_REG4	0x04		/* select reg 4 */
#define	M_REG5	0x05		/* select reg 5 */
#define	M_REG6	0x06		/* select reg 6 */
#define	M_REG7	0x07		/* select reg 7 */
#define	M_RS_EX_INT	0x10		/* reset external ints wr0 */
#define	M_CHAN_RES	0x18		/* channel reset wr0 */
#define	M_ENAB_RX	0x20		/* enable next Rx int wr0 */
#define	M_RS_TX_INT	0x28		/* reset external ints wr0 */
#define	M_ERR_RES	0x30		/* error reset wr0 */
#define	M_EOI	0x38		/* reset internal ints */
#define	M_INT_EN	0x01		/* enable ints wr1 */
#define	M_TX_INT_EN	0x02		/* Tx int enable */
#define	M_RX_DIS_INT	0x00		/* Rx int disable wr1 */
#define	M_RX_INT	0x18		/* interrupt on all chars wr1 */
#define M_SAV	0x04		/* status affects vector */
#define	M_RX_EN	0x01		/* enable Rx wr3 */
#define	M_RX_8BPC	0xC0		/* Rx 8 bits/char wr3 */
#define	M_16X	0x40		/* 16x clock rate */
#define	M_NO_PARITY	0x00		/* no parity */
#define	M_1STOP	0x04		/* one stop bit wr4 */
#define	M_RTS	0x02		/* request to send wr5 */
#define	M_TX_EN	0x08		/* Tx enable wr5 */
#define	M_TX_8BPC	0x60		/* Tx 8 bits/char wr5 */
#define	M_DTR	0x80		/* data terminal ready w5 */
#define	M_CHAR_AV	0x01		/* recieve char avail rr0 */
#define	M_CAR_DET	0x08		/* carrier detect */
#define	M_TX_EMPTY	0x04		/* Tx buffer empty rr0 */
#define	M_CTS_STAT	0x20		/* clear to send stat rr0 */
#define	M_NOP	0x00		/* no op */

#define	N8274		0x2		/* number of 8274s in system */

/*
 * data structures for 8274
 * All mpsc's (managed by the i8274 driver) are placed in a table
 * of mpsc structures. The ordering of the structs in the table is
 * criticle; it must go D0 ch A, D0 ch B, D1 ch A, D1 ch B, ...
 * (where D0 ch A is Device 0 channel A)
 */
struct mpsc {
	unsigned	m_ctrl;	/* control port for this channel */
	unsigned	m_data;	/* data port for this channel */
	unsigned	m_tctrl;/* timer control port */
	unsigned	m_tdata;/* timer data port */
	unsigned	m_tnum; /* timer number */
	unsigned	m_intlev;/* interrupt level */
	};


/*
 * baud rate definitions for 286/10
 */
#define	i74_B38400	2
#define	i74_B19200	4
#define	i74_B9600	8
#define	i74_B4800	16
#define	i74_B2400	32
#define i74_B1800	48
#define	i74_B1200	64
#define	i74_B600	128
#define	i74_B300	256
#define i74_B200	384
#define	i74_B150	512
#define	i74_B110	698
#define	i74_B75		1024
#define i74_B50		1536
#define i74_B0		0
#define	ISPEED		13	/* 9600 baud */

#define	MINORMSK	0x1F
#define	MODEMMSK	0x20

#define	RATEMD0		0x36

#define	TEST_VECT	0xA5
#define	VECT_MASK	0xE0
#define	NO_INT_VECT	0x1C
#define	INT_PENDING	0x02
