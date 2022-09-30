/*      Copyright (c) 1985 AT&T */
/*        All Rights Reserved   */

/*      THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T     */
/*      The copyright notice above does not evidence any        */
/*      actual or intended publication of such source code.     */

/*
** @(#)sio.h	1.7
**	Structures for the iSBC 544 Board
*/

/*
** command que structure
*/
struct	cmdque {
	unsigned char 	cmd;		/* the command that is que'd */
	unsigned char	line;		/* the line for this command */
};

#define CQSIZE	9	/* number of que'd commands allowed */

/*
** per board data structure
*/
struct	i544board {
	struct i544firm	*firm;		/* per board data */
	struct i544line	*line[4];	/* per line data */
	unsigned int	state;		/* per board state data */
	int		cqbase;		/* index of the next command */
	int		cqlen;		/* number pending */
	struct cmdque	cmdque[CQSIZE]	/* command que */
};

/*
** Firmware command/status/information structure
*/
struct i544firm {
	unsigned char	cmd;		/* command from host */
	unsigned char	status;		/* status to host */
	unsigned char	cmdsem;		/* command semaphore */
	unsigned char	stsem;		/* status semaphore */
	unsigned char	cunit;		/* unit number associate with command */
	unsigned char	sunit;		/* unit number associate with status */
	unsigned char	oper;		/* operational status */
	unsigned char	deverr;		/* error reporting */
	unsigned int	firstl;		/* addr of first line structure */
	unsigned int	lsize;		/* size of each line structure */
	unsigned char	nlines;		/* number of lines on board */
	unsigned char	intenb;		/* interrupt enable */
	unsigned int	vers;		/* firmware version number */
	unsigned char	diagn;		/* firmware diagnostic number */
	unsigned char	diage;		/* failed diagnostic information */
	unsigned int	joaddr;		/* jump out command address */
	unsigned char	portno;		/* Port I/O command port number */
	unsigned char	portv;		/* Port I/O command data */
};

/*
** Line sturcture, one per unit, 4 per board
*/
struct i544line {
	unsigned char	enb;		/* enable bits */
	unsigned char	parm;		/* parameter bits */
	unsigned char	state;		/* state of line */
	unsigned char	error;		/* error bits */
	unsigned int	ibaud;		/* input baud rate */
	unsigned int	obaud;		/* output baud rate */
	unsigned int	iba;		/* input buffer address */
	unsigned int	ibs;		/* input buffer size */
	unsigned int	ibp;		/* input buffer pointer */
	unsigned int	ibc;		/* input buffer count */
	unsigned int	ibn;		/* number of bytes input */
	unsigned int	oba;		/* output buffer address */
	unsigned int	obs;		/* output buffer size */
	unsigned int	obp;		/* output buffer pointer */
	unsigned int	obc;		/* output buffer count */
	unsigned int	obn;		/* number of bytes output */
	unsigned int	obl;		/* output byte limit */
	unsigned char	scmd;		/* pending command to board */
	unsigned char	wtime;		/* output delay timeout value */
	unsigned int	leng;		/* pending command data length */
	unsigned int	instate;	/* input state of line */
};

/*
** Definitions for state field in board structure
** Note: OUTBUSY and INBUSY are shifted by line number where used
*/
#define ALIVE	0x01		/* board is working */
#define PRESENT	0x02		/* board has responded */
#define OPENWAIT 0x04		/* waiting for initialize */
#define INBUSY	0x0100		/* an input command is qued for this line */
#define OUTBUSY	0x1000		/* an output command is qued for this line */

/*
** Commands for the command/status/information structure
** -----------------------------------------------------
*/
#define	RESET	0x01	/* resets the entire board and runs micro diagnostics */
#define	INPUT	0x02	/* signals that the host has transferred ibn bytes from the input buffer */
#define	OUTPUT	0x03	/* signals that the host has placed obn bytes in the output buffer for output */
#define	PARAM	0x04	/* signals that the host has changed parameters parm or ibaud */
#define	JPOUT	0x05	/* causes the 544 to call the subroutine indicated by 'joaddr' */
#define	CONTI	0x06	/* causes the reset to continue, ignoring the error */
#define	PRTOUT	0x07	/* for diagnostic only */
#define	PRTIN	0x08	/* for diagnostic only */
#define	OFLUSH	0x09	/* flush output buffer */
 
/* Misc. information for the command/status/information structure */
#define	CLEAR	0
#define	ENBINTR	0x01

/*
** Status to the host from the firmware
*/
#define	CMDACP	0x01	/* acknowledges the execution of the last command */
#define	INVCMD	0x02	/* indicates that the last command was in error */
#define	INRDY	0x03	/* indicates that the input is ready (ibc bytes) */
#define	OUTRDY	0x04	/* indicates that the output is ready (obc bytes) */
#define	RING	0x05	/* indicates that the line sunit is ringing */
#define	CARIER	0x06	/* indicates that there has been a carrier loss on line */
#define	ABAUDR	0x07	/* indicates that the baud rate has been recognized on line */
 
/*
** Diagnostic information
*/
#define	ROMERR	0	/* ROM checksum error */
#define	RAMFIL	0x01	/* RAM failure */
#define	PARINT	0x02	/* parallel port initialization failure */
#define	PAROPR	0x03	/* parallel port operating state failure */
#define	ROMSUM	0	/* sum of ROM bytes */
#define	RAMVAL	0x01	/* value that should have been in RAM location */
#define	PARLV1	0x02	/* PARLV1	value from port A of parallel interface */
#define	PARLV2	0x03	/* PARLV2	value from port A of parallel interface */
#define	DIAGRUN	0x01	/* running diagnostic */
#define	DIAGSUC	0x02	/* diagnostics succeeded */
#define	DIAGERR	0x03	/* diagnostic error */

/* 
** Line (Unit) structure information
** ---------------------------------
*/
#define	ENBOUT	0x01	/* enable output */
#define	XOUT	0x02	/* enable output Control-S/Control-Q */
#define	XIN	0x04	/* enable input Control-S/Control-Q */
#define	DTRDY	0x01	/* Data Terminal Ready */
#define	PNO	0x00	/* no parity */
#define	PODD	0x02	/* odd parity */
#define	PEVEN	0x04	/* even parity */
#define	CARRIER 0x01	/* Carrier state set */
#define	DSRDY	0x02	/* data set ready */
#define	IBUFOV	0x01	/* input buffer overflowed */
#define	RECINT	0x10	/* turn off receive input interrupts */
#define	NOMODEM 0x08	/* turn off ring detect and carrier detect interrupts */

/*
** defines for minor device flags
*/
#define MODEMDEV 0x80	/* bit 7 on for modems */
