/*      Copyright (c) 1985 AT&T */
/*        All Rights Reserved   */

/*      THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T     */
/*      The copyright notice above does not evidence any        */
/*      actual or intended publication of such source code.     */

/*
** @(#)sio.c	1.18
**	Intel iSBC 544 Intelligent Communications Controller Driver
**
**	Bits 1 & 0 in the minor device number select the unit, and
**	Bits 3 & 2 select the board
**	Bit 7 on for modem control
**
*/

#include "sys/types.h"
#include "sys/param.h"
#include "sys/systm.h"
#include "sys/conf.h"
#include "sys/dir.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/proc.h"
#include "sys/file.h"
#include "sys/tty.h"
#include "sys/termio.h"
#include "sys/sysinfo.h"
#include "sys/sio.h"
#include "sys/clock.h"
#include "sys/ioctl.h"
#include "sys/mmu.h"
#include "sys/seg.h"

/*
** Macros for various device things
*/
#define	BOARD( dev )	( ( dev >> 2 ) & 0x3 )
#define UNIT( dev )	( dev & 0x3 )

#define N544	4		/* 4 boards maximum */
#define	I544_CNT (N544 * 4)	/* 4 channels on each of 4 544's */

uint getword();
uint putword();

/*
** tty structures, one per line
*/
struct	tty	sio_tty[ I544_CNT ];

/*
** board structures, one per board
*/
struct i544board i544board[ N544 ];

/*
** Baud rate table.
** This tabel is indexed by the CBAUD field in c_cflag in
** the termio ioctl structure. The -1 entries are invalid speeds.
*/
int	i544speeds[] = { 0, -1, -1, 110, -1, 150, -1, 300, 600,
			1200, -1, 2400, 4800, 9600, 19200, 19200 };

/*
** sioopen
**	Open a device
*/
sioopen( dev, flag )
int	dev,
	flag;
{
	register struct tty	*tp;
	register struct i544board *bd;
	extern	sioproc();

	/*
	** point to correct tty struct and board struct
	*/
	tp = &sio_tty[ UNMODEM( dev ) ];
	bd = &i544board[ BOARD( dev ) ];

	/*
	** validate device and see if it is alive
	*/
	if ( ( UNMODEM( dev ) >= I544_CNT ) || ( ! ( bd->state & ALIVE ) ) ) {
		u.u_error = ENXIO;
		return;
	}

	/*
	** if not open, or not waiting for open
	** then initialize this tty
	*/
	if ( ! ( tp->t_state & ( ISOPEN | WOPEN ) ) ) {
		ttinit( tp );
		tp->t_proc = sioproc;
		sioparam( UNMODEM( dev ) );
	}

	/*
	** make sure that dtr is up
	*/
	bd->line[ UNIT( dev ) ]->parm |= DTRDY;
	sendcmd( dev, PARAM );

	/*
	** if it is a local line (CLOCAL or not MODEMDEV) or if
	** carrier is present then continue, else if not FNDELAY then
	** wait for carrier.
	*/
	if ( ( tp->t_cflag & CLOCAL ) || ( ! ( dev & MODEMDEV ) ) ||
	     bd->line[ UNIT( dev ) ]->state & CARRIER ) {
		tp->t_state |= CARR_ON;
	} else {
		tp->t_state &= ~CARR_ON;
		if ( ! ( flag & FNDELAY ) ) {
			tp->t_state |= WOPEN;
			while ( ! ( tp->t_state & CARR_ON ) ) {
				siodelay( HZ );
				if ( bd->line[ UNIT( dev ) ]->state & CARRIER ) {
					tp->t_state |= CARR_ON;
				}
			}
			tp->t_state &= ~WOPEN;
		}
	}

	/*
	** execute the common open code for this line discipline
	*/
	( *linesw[ tp->t_line ].l_open )( tp );
}

/*
** sioclose
**	Close a device
*/
sioclose( dev )
int	dev;
{
	register struct tty	*tp;

	tp = &sio_tty[ UNMODEM( dev ) ];	/* point to correct tty struct */

	/*
	** execute the common close code for this line discipline
	*/
	( *linesw[ tp->t_line ].l_close )( tp );

	/*
	** if hang up on close then hang up.
	*/
	if ( tp->t_cflag & HUPCL ) {
		i544board[ BOARD( dev ) ].line[ UNIT( dev ) ]->parm &= ~DTRDY;
		sendcmd( dev, PARAM );
	}
}


/*
** sioread
**	Read from a device
*/
sioread( dev )
int	dev;
{
	register struct tty	*tp;

	tp = &sio_tty[ UNMODEM( dev ) ];	/* point to correct tty struct */

	/*
	** execute the common read code for this line discipline
	*/
	( *linesw[ tp->t_line ].l_read )( tp );

}

/*
** siowrite
**	Write to a device
*/
siowrite( dev )
int	dev;
{
	register struct tty	*tp;

	tp = &sio_tty[ UNMODEM( dev ) ];	/* point to correct tty struct */

	/*
	** execute the common write code for this line discipline
	*/
	( *linesw[ tp->t_line ].l_write )( tp );
}

/*
** sioioctl
**	Call the common ioctl code, and
**	modify the state of the device if necessary
*/
sioioctl( dev, cmd, arg, mode )
unsigned int	dev,
		cmd,
		mode;
union ioctl_arg	arg;
{
	switch ( cmd ) {
		default:
			/*
			** if the ioctl is one in which the parameters of
			** the chip have to be changed, change them
			*/
			if ( ttiocom( &sio_tty[ UNMODEM( dev ) ], cmd, arg, mode ) ) {
				sioparam( dev );
			}
	}
}

/*
** sioparam
**	Modify the state of the device as indicated in
**	the tty structure
*/
sioparam( dev )
int	dev;
{
	register struct tty	*tp;
	register struct i544firm *firm;	/* pointer to firmware structure */
	register struct i544line *line;	/* pointer to line structure */
	int speed;			/* temp holder for speed */

	tp = &sio_tty[ UNMODEM( dev ) ]; /* point to correct tty struct */
	firm = i544board[ BOARD( dev ) ].firm;
	line = i544board[ BOARD( dev ) ].line[ UNIT( dev ) ];

	/*
	** make sure that modem is enabled
	*/
	line->parm &= ~NOMODEM;

	/*
	** if baud rate is zero, hang up line
	*/
	if ( ! ( tp->t_cflag & CBAUD ) ) {
		line->parm &= ~DTRDY;
		sendcmd( dev, PARAM );

		/*
		** If the device is not a modem then no carrier transition
		** will be caused by turning off dtr, so a hangup signal must
		** be initiated here.
		*/
		if ( ! ( dev & MODEMDEV ) )
			signal( tp->t_pgrp, SIGHUP );
		return;
	}

	/*
	** set the baud rate
	*/
	speed = i544speeds[ tp->t_cflag & CBAUD ];
	if ( speed != -1 ) {
		putword( &i544board[ BOARD( dev ) ].line[ UNIT( dev ) ]->ibaud, speed );
		putword( &i544board[ BOARD( dev ) ].line[ UNIT( dev ) ]->obaud, speed );
	}

	/*
	** set the board structures in preparation for re-programming
	*/
	firm->cunit = UNIT( dev );

	/*
	** if parity enabled then set even or odd, else set none
	*/
	line->parm = ( tp->t_cflag & PARENB ?
	    ( tp->t_cflag & PARODD ? PODD : PEVEN ) : PNO );

	/*
	** check for reciever disabling
	*/
	if ( ! ( tp->t_cflag & CREAD ) ) {
		line->parm |= RECINT;
	}

	/*
	** if doing xon/xoff on input and are not
	** restarting on any character then the 544
	** board can do xon/xoff processing, otherwise
	** the board is told not to, and if doing xon/xoff
	** then the software does it.
	*/
	if ( ( tp->t_iflag & IXON ) && ( ! ( tp->t_iflag & IXANY ) ) ) {
		line->enb |= XOUT;
	} else {
		line->enb &= ~XOUT;
	}

	/*
	** the board always does xon/xoff for output
	*/
	if ( tp->t_iflag & IXOFF ) {
		line->enb |= XIN;
	} else {
		line->enb &= ~XIN;
	}

	/*
	** enable output and interrupts
	*/
	line->enb |= ENBOUT;
	firm->intenb = ENBINTR;
	line->parm |= DTRDY;

	/*
	** program the 544
	*/
	sendcmd( dev, PARAM );
}


/*
** siointr
**	Entry point for all interrupts from all 544 devices.
**	If a second board asserts the interrupt while servicing
**	an interrupt the second interrupt will be missed because
**	the interrupt from the PIC is signaled on the state change,
**	So must cycle through all the boards until there are no
**	more interrupts pending.
*/
siointr( vec )
unsigned int	vec;
{
	register int intproc;	/* interrupt processed flag */
	register int board;	/* board index in for loop */

	do {
		intproc = 0;
		for ( board = 0; board < N544; board++ ) {
			if ( ( i544board[ board ].state & ALIVE ) &&
			    ( i544board[ board ].firm->status != CLEAR ) ) {
				handleintr( board );
				intproc++;
			}
		}
	} while ( intproc );

	asm( "	cli" );
	eoi( vec );
}

/*
** service status command on line
** according to type
*/
handleintr( board )
int board;
{
	register struct tty *tp;
	register struct i544board *bd;
	int unit;
	int dev;
	int i;			/* counter for various loops */
	extern		ttrstrt();

	/*
	** get pointer to board structure
	*/
	bd = &i544board[ board ];

	/*
	** get the current unit and device
	*/
	unit = bd->firm->sunit;
	dev = ( board << 2 ) | unit;

	/*
	** get tty structure pointer
	*/
	tp = &sio_tty[ dev ];

	switch (bd->firm->status) {

	case CMDACP:

		/*
		**	if there is a command on the que then it has been
		**	accepted, deque it and do the next one.
		*/
		if ( bd->cqlen ) {
			/*
			** check for output command completed
			*/
			if ( bd->cmdque[ bd->cqbase ].cmd == OUTPUT ) {
				bd->state &= ~( OUTBUSY << unit );
				sioproc( tp, T_OUTPUT );
			}

			/*
			** check for input command completed
			*/
			if ( bd->cmdque[ bd->cqbase ].cmd == INPUT ) {
				bd->state &= ~( INBUSY << unit );
				getinput( tp, dev );
			}

			bd->cqbase = ( bd->cqbase + 1 ) % CQSIZE;
			bd->cqlen--;
			docmd( board );
		}

		bd->firm->status = CLEAR;
		break;

	case INVCMD :
		printf( "sio: invalid command, board %d, line %d\n",
		    board, (uint)bd->firm->sunit );
		bd->firm->status = CLEAR;
		break;

	case INRDY:
		sysinfo.rcvint++;
		getinput( tp, dev );
		bd->firm->status = CLEAR;
		break;

	case OUTRDY :
		sysinfo.xmtint++;

		/*
		** if was busy and the output has drained then
		** do some more output.
		*/
		if ( ( tp->t_state & BUSY ) && ( getword( &bd->line[ unit ]->obc ) == getword( &bd->line[ unit ]->obs ) - 1 ) ) {
			tp->t_state &= ~BUSY;
			sioproc( tp, T_OUTPUT );
		}
		bd->firm->status = CLEAR;
		break;

	case RING:
		sysinfo.mdmint++;
		bd->firm->status = CLEAR;
		break;

	case CARIER:
		sysinfo.mdmint++;
		bd->firm->status = CLEAR;

		/*
		** signal the process group
		*/
		if ( tp->t_state & CARR_ON ) {
			signal( tp->t_pgrp, SIGHUP );
			tp->t_state &= ~CARR_ON;
			bd->line[ unit ]->parm &= ~DTRDY;
			sendcmd( dev, PARAM );
		}
		break;

	case ABAUDR:	/* Auto baud rate recognition not supported */
	default:
		printf( "sio: invalid status=%d\n", (uint)bd->firm->status );
		bd->firm->status = CLEAR;
		break;
	}
}

/*
** sioproc
**	General command routine that initiates action
*/
sioproc( tp, cmd )
register struct tty	*tp;
int			cmd;
{
	struct i544board *bd;
	char *outbase;		/* base of output buffer for current line */
	extern		ttrstrt();
	int		dev;
	int		count;	/* number of free chars in output buffer */
	int		i;	/* position for this char in output buffer */
	int		unit;	/* unit number for current line */
	struct ccblock *tbuf;

	/*
	** get device number
	*/
	dev = tp - sio_tty;

	/*
	** based on cmd, do various things to the device
	*/
	switch ( cmd ) {
		case T_TIME:		/* stop sending a break */
			/*
			** the current 544 won't send a break
			*/
			tp->t_state &= ~TIMEOUT;
			goto start;

		case T_WFLUSH:		/* output flush */
			tp->t_tbuf.c_size -= tp->t_tbuf.c_count;
			tp->t_tbuf.c_count = 0;
			/* FALL THROUGH  */

		case T_RESUME:		/* enable output */
			tp->t_state &= ~TTSTOP;
			i544board[ BOARD( dev ) ].line[ UNIT( dev ) ]->enb |= ENBOUT;
			/* FALL THROUGH  */

		case T_OUTPUT:		/* do some output */
	start:
			/*
			** if stopped, timed-out, or just plain
			** busy, don't do anything
			*/
			if ( tp->t_state & ( TIMEOUT | TTSTOP | BUSY ) ) {
				break;
			}

			unit = UNIT( dev );
			bd = &i544board[ BOARD( dev ) ];
			outbase = ( (char *)bd->firm ) +
			    getword( &bd->line[ unit ]->oba );
			count = getword( &bd->line[ unit ]->obc );
			i = 0;

			/*
			** if output is busy on this channel then quit
			*/
			if ( bd->state & ( OUTBUSY << unit ) ) {
				break;
			}

			/*
			** get characters to output and send them to the 544
			*/
			tbuf = &tp->t_tbuf;
			do {
				if ( tbuf->c_ptr == NULL || tbuf->c_count <= 0 ) {
					if ( tbuf->c_ptr ) {
						tbuf->c_ptr -= tbuf->c_size - tbuf->c_count;
					}
					if ( ! ( CPRES & (*linesw[ tp->t_line ].l_output )( tp ) ) ) {
						break;
					}
				}

				outbase[ ( getword( &bd->line[ UNIT( dev ) ]->obp ) + i++ )
					% getword( &bd->line[ UNIT( dev ) ]->obs ) ] = *tbuf->c_ptr++;
					tbuf->c_count--;
			} while ( --count );

			/*
			** tell the board that it has more characters
			*/
			if ( i != 0 ) {
				tp->t_state |= BUSY;
				bd->state |= ( OUTBUSY << unit );
				putword( &bd->line[ UNIT( dev ) ]->obn, i );
				sendcmd( dev, OUTPUT );
			}
			break;

		case T_SUSPEND:		/* block on output */
			tp->t_state |= TTSTOP;
			i544board[ BOARD( dev ) ].line[ UNIT( dev ) ]->enb &= ~ENBOUT;
			break;

		case T_BLOCK:		/* block on input */
			tp->t_state |= TBLOCK;
			break;

		case T_RFLUSH:		/* flush input */
			/*
			** if were blocked then fall in to unblock
			*/
			if ( ! ( tp->t_state & TBLOCK ) ) {
				break;
			}
			/* FALL THROUGH  */

		case T_UNBLOCK:		/* enable input */
			/*
			** unblock and try to read more input
			*/
			tp->t_state &= ~TBLOCK;
			getinput( tp, dev );
			break;

		case T_BREAK:		/* send a break */
			/*
			** the current 544 can't send a break
			*/
			tp->t_state |= TIMEOUT;
			timeout( ttrstrt, tp, HZ / 4 );
			break;

		case T_SWTCH:		/* shell layer switch */
			/*
			** nothing for us to do
			*/
			break;

		case T_PARM:		/* update parameters */
			sioparam( dev );
			break;

		default:
			break;
	}
}

/*
** sioscan
**	find out what kind of 544 is present
*/
unsigned long
sioscan()
{
	unsigned int dev;
	unsigned int board;
	int i;
	unsigned long addr = 0L;
	struct i544board *bd;
	extern struct seg_desc gdt[];

	printf( "scanning 544 configuration:\n" );
	gdt[ I544SEL ].sd_hibase = 0xEE;
	for ( board = 0; board < N544; board++ ) {
		dev = board << 2;
		printf( "\tXE%x000 ", dev );

		/*
		** get board pointers for this device
		*/
		bd = &i544board[ board ];
		bd->firm = (struct i544firm *)( gstokv( I544SEL ) +
			(unsigned long)( board << 14 ) );

		/*
		** see if there is a board here
		*/
		sioinit( dev );

		/*
		** if present then set flag
		*/
		if ( bd->state & PRESENT ) {
			printf( "present (prom level %d.%d)\n",
			    (int)bd->firm->vers / 10,
			    (int)bd->firm->vers % 10 );
			addr = 0xE0000;
		} else {
			printf( "not present\n" );
		}

		if ( bd->state & ALIVE ) {
			for ( i = 0; i < 4; i++ ) {

				/*
				** set up pointers to line structures
				*/
				bd->line[ i ] =
				    (struct i544line *)( (unsigned long) bd->firm +
				    getword( &bd->firm->firstl ) +
				    ( getword( &bd->firm->lsize ) * i ) );
			}

			/*
			** set up constant fields
			*/
			bd->cqbase = 0;
			bd->cqlen = 0;
		} else
			break;
	}

	/*
	** now check for 544A's
	*/
	gdt[ I544SEL ].sd_hibase = 0xFE;
	for ( ; board < N544; board++ ) {
		dev = board << 2;

		/*
		** get board pointers for this device
		*/
		bd = &i544board[ board ];
		bd->firm = (struct i544firm *)( gstokv( I544SEL ) +
			(unsigned long)( board << 14 ) );

		/*
		** check for a non 544A here already
		*/
		if ( bd->state & PRESENT ) {
			continue;
		}

		printf( "\tFE%x000 ", dev );
		/*
		** see if there is a board here
		*/
		sioinit( dev );

		/*
		** if present then set flag
		*/
		if ( bd->state & PRESENT ) {
			printf( "present (prom level %d.%d)\n",
			    (int)bd->firm->vers / 10,
			    (int)bd->firm->vers % 10 );
			if ( ! addr )
				addr = 0xFE0000;
		} else {
			printf( "not present\n" );
		}

		if ( bd->state & ALIVE ) {
			for ( i = 0; i < 4; i++ ) {

				/*
				** set up pointers to line structures
				*/
				bd->line[ i ] =
				    (struct i544line *)( (unsigned long) bd->firm +
				    getword( &bd->firm->firstl ) +
				    ( getword( &bd->firm->lsize ) * i ) );

				/*
				** set output buffer length so that the board
				** will interrupt when the buffer is half empty
				*/
				putword( &bd->line[ i ]->obl,
				    getword( &bd->line[ i ]->obs ) >> 1 );
			}

			/*
			** set up constant fields
			*/
			bd->cqbase = 0;
			bd->cqlen = 0;
		} else
			break;
	}
	printf( "\n" );

	/*
	** send base address up to the kernel
	*/
	return( addr );
}

/*
** sioinit
**	initialize a 544
*/
sioinit( dev )
{
	struct i544board *bd;
	unsigned long i;

	/*
	** get pointer to board structure
	*/
	bd = &i544board[ BOARD( dev ) ];

	/*
	** reset the board and see if it comes to life
	*/
	bd->firm->oper = 0;
	bd->firm->intenb = CLEAR;
	bd->firm->cmd = RESET;

initdelay:
	for ( i = 0x80000l; i != 0; i-- )
		;

	switch ( bd->firm->oper ) {

		case DIAGRUN:
			bd->state = PRESENT;
			break;

		case DIAGSUC:
			bd->state = ALIVE | PRESENT;
			break;

		case DIAGERR:
			printf("sio: board %d diagnostic %d failed, error %x\n",
			    dev, (uint)bd->firm->diagn, (uint)bd->firm->diage );
			bd->firm->cmd = CONTI;
			goto initdelay;

		default:
			bd->state = CLEAR;
	}

	bd->firm->status = CLEAR;
}

getinput( tp, dev )
register struct tty *tp;
{
	register struct i544board *bd;
	register char c;	/* holder for characters */
	register int i;		/* counter for loop */
	uint ibc_count;		/* holder for input buffer count */
	char *inbase;		/* base of input buffer */
	int unit;
	int x;

	x = spl5( x );
	bd = &i544board[ BOARD( dev ) ];
	unit = UNIT( dev );
	inbase = ( (char *)bd->firm ) + getword( &bd->line[ unit ]->iba );
	ibc_count = getword( &bd->line[ unit ]->ibc );

	/*
	** check to see if it is safe to get the input
	*/
	if ( ( bd->state & ( INBUSY << unit ) ) || ibc_count == 0 ||
	    tp->t_state & TBLOCK ) {
		return;
	}

	/*
	** check for a break
	*/
	if ( bd->line[ unit ]->error ) {
		bd->state |= ( INBUSY << unit );
		putword( &bd->line[ unit ]->ibn, ibc_count );
		bd->line[ unit ]->error = CLEAR;
		sendcmd( dev, INPUT );
		if ( tp->t_iflag & BRKINT ) {
			(*linesw[ tp->t_line ].l_input)( tp, L_BREAK );
		}
		return;
	}

	/*
	** if not open, nobody to give the chars to,
	** so chuck them.
	*/
	if ( ! ( tp->t_state & ( ISOPEN | WOPEN ) ) ) {
		bd->state |= ( INBUSY << unit );
		putword( &bd->line[ unit ]->ibn, ibc_count );
		sendcmd( dev, INPUT );
		return;
	}

	/*
	** get the input
	*/
	i = 0;

get_loop:
	/*
	** check for end of input
	*/
	if ( i == ibc_count ) {
		goto get_exit;
	}

	/*
	** get a character
	*/
	c = inbase[ ( getword( &bd->line[ unit ]->ibp ) + i++ ) %
	    getword( &bd->line[ unit ]->ibs ) ];

	/*
	** check xon/xoff
	*/
	if ( tp->t_iflag & IXON ) {
		unsigned char	ctmp;

		ctmp = c & 0x7F;

		/*
		** if blocked and if the char is a xon, or if
		** restarting on any char then resume
		*/
		if ( tp->t_state & TTSTOP ) {
			if ( ctmp == CSTART || tp->t_iflag & IXANY )
				sioproc( tp, T_RESUME );
		} else {
			if ( ctmp == CSTOP )
				sioproc( tp, T_SUSPEND );
		}
		if ( ctmp == CSTART || ctmp == CSTOP ) {
			goto get_loop;
		}
	}

	/*
	** check 7 bit striping
	*/
	if ( tp->t_iflag & ISTRIP ) {
		c &= 0x7F;
	}

	/*
	** if nowhare to put the char then give up
	*/
	if ( tp->t_rbuf.c_ptr == NULL ) {
		i = ibc_count;
		goto get_exit;
	}

	/*
	** stuff the char into buffer
	** if buffer is full then exit loop
	*/
	*tp->t_rbuf.c_ptr++ = c;
	if ( --tp->t_rbuf.c_count ) {
		goto get_loop;
	}
get_exit:

	/*
	** signify that there are characters
	*/
	bd->state |= ( INBUSY << unit );
	putword( &bd->line[ unit ]->ibn, i );
	sendcmd( dev, INPUT );
	tp->t_rbuf.c_ptr -= tp->t_rbuf.c_size - tp->t_rbuf.c_count;
	( *linesw[ tp->t_line ].l_input )( tp, L_BUF );
	splx( x );
}

/*
** sendcmd
**	que up a command
*/
sendcmd( dev, cmd )
{
	struct i544board *bd;
	int x;

	x = spl5();
	bd = &i544board[ BOARD( dev ) ];

	/*
	** check for overflow
	*/
	if ( bd->cqlen >= CQSIZE ) {
		printf( "sio: command que overflow\n" );
		splx( x );
		return;
	}

	/*
	** put command on que
	*/
	bd->cmdque[ ( bd->cqbase + bd->cqlen ) % CQSIZE ].cmd = cmd;
	bd->cmdque[ ( bd->cqbase + bd->cqlen ) % CQSIZE ].line = UNIT( dev );

	/*
	** increment length and if it was zero then send off this command
	*/
	if ( bd->cqlen++ == 0 ) {
		docmd( BOARD( dev ) );
	}

	splx( x );
}

/*
** docmd
**	if there is a command on the que then give it to the 544
*/
docmd( board )
{
	struct i544board *bd;

	bd = &i544board[ board ];

	if ( bd->cqlen == 0 ) {
		return;
	}

	if ( bd->firm->cmd ) {
		printf( "sio: cmd sync error\n" );
	}

	bd->firm->cunit = bd->cmdque[ bd->cqbase ].line;
	bd->firm->cmd = bd->cmdque[ bd->cqbase ].cmd;
}

/*
** getword
**	get a word from the 544 dual ported memory
**	the 544 is byte wide
*/
uint
getword( addr )
unsigned char *addr;
{
	uint temp;

	temp = ( (uint)*(addr+1) << 8 );
	temp |= (uint)*addr;
	return temp;
}

/*
** putword
**	put a word to the 544 dual ported memory
**	the 544 is byte wide
*/
uint
putword( addr, data )
unsigned char *addr;
uint data;
{
	*addr = data & 0xff;
	*(addr+1) = ( data >> 8 ) & 0xff;
}

/*
** siodelay
**	This routine is the same as delay except that
**	it sleeps at PZERO so it can be aborted by a signal.
*/
siodelay(ticks)
{
	extern wakeup();

	if (ticks<=0)
		return;
	timeout(wakeup, (caddr_t)u.u_procp+1, ticks);
	sleep((caddr_t)u.u_procp+1, TTIPRI);
}
