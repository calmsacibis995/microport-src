/*      Copyright (c) 1985 AT&T */
/*        All Rights Reserved   */

/*      THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T     */
/*      The copyright notice above does not evidence any        */
/*      actual or intended publication of such source code.     */

/*
** @(#)8274.h	1.6
**
** Definitions for the Intel 8274 Multi-protocol Serial Controller
*/

/*
** Port addresses
*/
#define	CHA_DATA	0x00D8		/* channel A data		*/
#define	CHB_DATA	0x00DA		/* channel B data		*/
#define	CHA_CTL		0x00DC		/* channel A control		*/
#define	CHB_CTL		0x00DE		/* channel B control		*/

/*
** Register Bit definitions
*/

/*
** Write register 0
*/
#define	R0		0x00		/* select register 0		*/
#define	R1		0x01		/* select register 1		*/
#define	R2		0x02		/* select register 2		*/
#define	R3		0x03		/* select register 3		*/
#define	R4		0x04		/* select register 4		*/
#define	R5		0x05		/* select register 5		*/
#define	R6		0x06		/* select register 6		*/
#define	R7		0x07		/* select register 7		*/
#define	SND_ABORT	0x08		/* send abort ( SDLC )		*/
#define	RES_EXT		0x10		/* reset ext/status interrupts	*/
#define	RES_CHAN	0x18		/* reset channel		*/
#define	INT_NXT_CHR	0x20		/* enable intr on next rcv char	*/
#define	RES_TXINT	0x28		/* reset xmit intr/dma pending	*/
#define	RES_ERR		0x30		/* reset error			*/
#define	EOI		0x38		/* end-of-interrupt		*/
#define	RES_RXCRC	0x40		/* reset rcv CRC checker	*/
#define	RES_TXCRC	0x80		/* reset xmit CRC checker	*/
#define	RES_TXEOM	0xC0		/* reset xmit underrun/eom latch*/

/*
** Write register 1
*/
#define	ENB_EXT		0x01		/* enable ext/status interrupts	*/
#define	ENB_TX_INT	0x02		/* enable xmit intr/dma		*/
#define	STAT_AFF_VECT	0x04		/* status affects vector (chanB)*/
#define	ENB_1ST_RX	0x08		/* enable intr on 1st rcv char	*/
#define	ENB_ALL_RXP	0x10		/* enable intr on all chars, 	*/
					/*        parity affects vector	*/
#define	ENB_ALL_RX	0x18		/* enable intr on all chars,	*/
					/*  parity doesn't affect vector*/
#define	WAIT_ON_RX	0x20		/* wait on receive		*/
#define	ENB_WAIT	0x80		/* enable wait			*/

/*
** Write register 2 ( Channel A )
*/
#define	AB_INT		0x00		/* channels A & B interrupt	*/
#define	ADMA_BINT	0x01		/* channel A dma; channel B intr*/
#define	AB_DMA		0x02		/* channels A & B dma		*/
#define	PRIORITY	0x04		/* priority: RxA>RxB>TxA>TxB>ext*/
#define	MODE2_8085	0x08		/* 8085 mode 2			*/
#define	MODE_8086	0x10		/* 8086 mode			*/
#define	VECT_INT	0x20		/* vectored interrupts		*/
#define	SYNDET		0x80		/* Pin 10 = SYNDET ( SDLC )	*/

/*
** Write register 2 ( Channel B ) is interrupt vector
*/

/*
** Write register 3
*/
#define	ENB_RX		0x01		/* enable receiver		*/
#define	SYNC_INHB	0x02		/* sync char load inhibit	*/
#define	ADDR_SRCH	0x04		/* addr srch mode ( SDLC )	*/
#define	ENB_RXCRC	0x08		/* enable rcv crc		*/
#define	HUNT_MODE	0x10		/* enter hunt mode		*/
#define	AUTO_ENBL	0x20		/* auto enables			*/
#define	RX_5BIT		0x00		/* rcv 5 bits per char		*/
#define	RX_7BIT		0x40		/* rcv 7 bits per char		*/
#define	RX_6BIT		0x80		/* rcv 6 bits per char		*/
#define	RX_8BIT		0xC0		/* rcv 8 bits per char		*/

/*
** Write register 4
*/
#define	ENB_PARITY	0x01		/* enable parity		*/
#define	EVEN_PAR	0x02		/* even parity			*/
#define	ENB_SYNC	0x00		/* enable sync mode		*/
#define	STOPBIT1	0x04		/* one stop bit			*/
#define	STOPBIT15	0x08		/* one and a half stop bits	*/
#define	STOPBIT2	0x0C		/* two stop bits		*/
#define	SYNC_8BIT	0x00		/* eight bit sync char		*/
#define	SYNC_16BIT	0x10		/* sixteen bit sync char	*/
#define	SDLC_HDLC	0x20		/* sdlc/hdlc mode flag		*/
#define	EXT_SYNC	0x30		/* external sync mode		*/
#define	CLOCKX1		0x00		/* clock times 1		*/
#define	CLOCKX16	0x40		/* clock times 16		*/
#define	CLOCKX32	0x80		/* clock times 32		*/
#define	CLOCKX64	0xC0		/* clock times 64		*/

/*
** Write register 5
*/
#define	ENB_TXCRC	0x01		/* enable xmit crc		*/
#define	RTS		0x02		/* assert RTS			*/
#define	CRC_MODE	0x04		/* sdlc/crc16 crc mode		*/
#define	ENB_TX		0x08		/* enable xmitter		*/
#define	SND_BREAK	0x10		/* send a break			*/
#define	TX_5BIT		0x00		/* xmit 5 bits per char		*/
#define	TX_7BIT		0x20		/* xmit 7 bits per char		*/
#define	TX_6BIT		0x40		/* xmit 6 bits per char		*/
#define	TX_8BIT		0x60		/* xmit 8 bits per char		*/
#define	DTR		0x80		/* assert DTR			*/

/*
** Write register 6 contains the transmit sync char in sync mode
*/

/*
** Write register 7 contains the receive sync char in sync mode
*/

/*
** Read register 0
*/
#define	RX_CHAR_AVAIL	0x01		/* rcv character is available	*/
#define	INT_PEND	0x02		/* interrupt pending ( Chan A )	*/
#define	TX_BUFMT	0x04		/* xmit buffer empty		*/
#define	DCD		0x08		/* carrier detect is present	*/
#define	SYNCHUNT	0x10		/* sync/hunt			*/
#define	CTS		0x20		/* clear-to-send is present	*/
#define	TX_UNDERRUN	0x40		/* xmit underrun/eom		*/
#define	BREAK		0x80		/* break/abort detected		*/

/*
** Read register 1
** NOTE: The definitions for residue data are not defined because
**	 of lack of interest
*/
#define	PARITY_ERR	0x10		/* parity error			*/
#define	RX_OVERRUN	0x20		/* rcv overrun			*/
#define	FRAME_ERR	0x40		/* framing/crc error		*/
#define	END_OF_FRAME	0x80		/* end-of-frame ( SDLC )	*/

/*
** Read register 2 ( Channel B ) is the interrupt vector, suitably
** modified if STAT_AFF_VECT is on.
** The following are the values that will be in this
** register when an interrupt occurs.
*/
#define	INTR_TYPE	0x1C		/* bits that are modified	*/
#define	CHANA_INT	0x10		/* values >= this are from Chn A*/
#define	TX_INT		0x00		/* xmit buffer empty		*/
#define	EXTCHNG		0x04		/* ext/status change		*/
#define	RCVCHAR		0x08		/* rcv char available		*/
#define	SPECRCV		0x0C		/* special receive condition	*/
