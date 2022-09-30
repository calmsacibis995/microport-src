/*      Copyright (c) 1985 AT&T */
/*        All Rights Reserved   */

/*      THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T     */
/*      The copyright notice above does not evidence any        */
/*      actual or intended publication of such source code.     */

/*
** @(#)console.c	1.18
**	Intel 8274 Multi-protocol Serial Controller driver
**
**	NOTE:
**		The 286/10 processor board does not support
**		ANY sort of modem control. Therefore, the
**		CLOCAL, HUPCL, and FNDELAY flags, and the modem signals
**		DTR, RTS, CTS, and DCD have no meaning;
**		they are unhappily ignored.
**		Also because of this, a break condition is not
**		detected. When modem control is supported in the
**		hardware, the code in the MDMCTL286 will support
**		the detection of a break condition.
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
#include "sys/8274.h"
#include "sys/clock.h"
#include "sys/ioctl.h"
#include "sys/ppi.h"

#define	ON	1
#define	OFF	0

#define	CON_CNT	2		/* 2 channels on one 8274	*/

#define	CONDEV	1		/* console minor device		*/

struct	tty	con_tty[ CON_CNT ];

/*
** The ctl, data, brgcnt and brgmode arrays are here to provide a minor
** device to port number mapping
*/
#define	CHA	0
#define	CHB	1
unsigned int	ctl[ CON_CNT ] =	/* 8274 control ports	*/
{
	CHA_CTL,
	CHB_CTL
};

unsigned int	data[ CON_CNT ] =	/* 8274 data ports	*/
{
	CHA_DATA,
	CHB_DATA
};

unsigned int	brgcnt[ CON_CNT ] =	/* baud rate gen counter ports 	*/
{
	CNT2MODE,
	CNT1MODE
};

unsigned char	brgmode[ CON_CNT ] =	/* baud rate gen mode bytes	*/
{
	BRGA,
	BRGB
};

/*
** counter values for the baud rate generator.
** These values assume an input clock frequency
** of 1.23Mhz to the baud rate generator, and
** the sio channel programmed to be CLOCKX16.
*/
unsigned int	con_speeds[] =
{
	0,		/* 0 baud	*/
	1538,		/* 50 baud	*/
	1025,		/* 75 baud	*/
	699,		/* 110 baud 	*/
	572,		/* 134.5 baud	*/
	513,		/* 150 baud	*/
	384,		/* 200 baud	*/
	256,		/* 300 baud	*/
	128,		/* 600 baud	*/
	64,		/* 1200 baud	*/
	43,		/* 1800 baud	*/
	32,		/* 2400 baud	*/
	16,		/* 4800 baud	*/
	8,		/* 9600 baud	*/
	4,		/* exta (19200)	*/
	4		/* extb (19200)	*/
};

/*
** initialization sequences for the two channels
*/
unsigned char	chainit[] =	/* Channel A	*/
{
	/*
	** clock x 16, 1 stop bits
	*/
	R4,	CLOCKX16 | STOPBIT1,
#ifdef MDMCTL286
	/*
	** enable ext/status intr, enable xmit intr,
	** intr on all rcv char
	*/
	R1,	ENB_TX_INT | ENB_ALL_RXP | ENB_EXT,
#else
	/*
	** enable xmit intr, intr on all rcv char
	*/
	R1,	ENB_TX_INT | ENB_ALL_RXP,
#endif
	/*
	** enable rcv, 8 bits per char
	*/
	R3,	ENB_RX | RX_8BIT,
	/*
	** enable xmit, 8 bits per char
	*/
	R5,	ENB_TX | TX_8BIT | DTR | RTS
};

unsigned char	chbinit[] =	/* Channel B	*/
{
	/*
	** clock x 16, 1 stop bits
	*/
	R4,	CLOCKX16 | STOPBIT1,
#ifdef MDMCTL286
	/*
	** enable ext/status intr, enable xmit intr,
	** intr on all rcv char
	*/
	R1,	ENB_TX_INT | ENB_ALL_RXP | ENB_EXT | STAT_AFF_VECT,
#else
	/*
	** enable xmit intr, intr on all rcv char
	*/
	R1,	ENB_TX_INT | ENB_ALL_RXP | STAT_AFF_VECT,
#endif
	/*
	** enable rcv, 8 bits per char
	*/
	R3,	ENB_RX | RX_8BIT,
	/*
	** enable xmit, 8 bits per char
	*/
	R5,	ENB_TX | TX_8BIT | DTR | RTS,
};

/*
** array of firstime flags
*/
int	notinit[ CON_CNT ] = { 1, 1 };


/*
** conopen
**	Open a device
*/
conopen( dev, flag )
int	dev,
	flag;
{
	register struct tty	*tp;
	extern			conproc();

	/*
	** minor device validation
	*/
	if ( dev >= CON_CNT )
	{
		u.u_error = ENXIO;
		return;
	}

	tp = &con_tty[ dev ];		/* point to correct tty struct	*/

	/*
	** if we are not open, or not waiting for open
	** then we should initialize this tty
	*/
	if ( ! ( tp->t_state & ( ISOPEN | WOPEN ) ) )
	{
		ttinit( tp );
		tp->t_proc = conproc;
		conparam( dev );
	}

	tp->t_state |= CARR_ON;

	/*
	** execute the common open code for this line discipline
	*/
	( *linesw[ tp->t_line ].l_open )( tp );
}


/*
** conclose
**	Close a device
*/
conclose( dev )
int	dev;
{
	register struct tty	*tp;

	tp = &con_tty[ dev ];		/* point to correct tty struct	*/

	/*
	** execute the common close code for this line discipline
	*/
	( *linesw[ tp->t_line ].l_close )( tp );

	/*
	** and drop dtr, but not on the console
	*/
	if ( dev != CONDEV )
		state( dev, DTR, OFF );
}


/*
** conread
**	Read from a device
*/
conread( dev )
int	dev;
{
	register struct tty	*tp;

	tp = &con_tty[ dev ];		/* point to correct tty struct	*/

	/*
	** execute the common read code for this line discipline
	*/
	( *linesw[ tp->t_line ].l_read )( tp );

}


/*
** conwrite
**	Write to a device
*/
conwrite( dev )
int	dev;
{
	register struct tty	*tp;

	tp = &con_tty[ dev ];		/* point to correct tty struct	*/

	/*
	** execute the common write code for this line discipline
	*/
	( *linesw[ tp->t_line ].l_write )( tp );
}


/*
** conioctl
**	Call the common ioctl code, and
**	modify the state of the device if necessary
*/
conioctl( dev, cmd, arg, mode )
unsigned int	dev,
		cmd,
		mode;
union ioctl_arg	arg;
{
	switch ( cmd )
	{
		default:
			/*
			** if the ioctl is one in which the parameters of
			** the chip have to be changed, change them
			*/
			if ( ttiocom( &con_tty[ dev ], cmd, arg, mode ) )
			{
				conparam( dev );
			}
	}
}


/*
** conparam
**	Modify the state of the device as indicated in
** 	the tty structure
*/
conparam( dev )
int	dev;
{
	register struct tty	*tp;
	register int		flags;	/* to ease access to c_flags	*/
	unsigned int		port;	/* to ease access to ctl port	*/
	unsigned char		xmode,	/* xmit modes			*/
				rmode,	/* rcv modes			*/
				psmode;	/* parity/stopbit modes		*/

	tp = &con_tty[ dev ];		/* point to correct tty struct	*/

	port = ctl[ dev ];

	/*
	** if we have never been here before,
	** initialize both channels enough so that
	** coninit can do a proper job.
	*/
	if ( notinit[ 0 ] & notinit[ 1 ] )
	{
		outb( PPIPC, 2 );	/* reset chan A loopback	*/
		outb( CHA_CTL, RES_CHAN );
		outb( CHB_CTL, RES_CHAN );

		outb( CHA_CTL, R2 );
		outb( CHA_CTL, 0 );
		outb( CHB_CTL, R2 );
		outb( CHB_CTL, 0 );
		outb( CHB_CTL, R4 );
		outb( CHB_CTL, STOPBIT1 | CLOCKX16 );
		outb( CHB_CTL, R1 );
		outb( CHB_CTL, ENB_TX_INT | ENB_ALL_RXP | STAT_AFF_VECT );
		outb( CHA_CTL, R4 );
		outb( CHA_CTL, STOPBIT1 | CLOCKX16 );
		outb( CHA_CTL, R1 );
		outb( CHA_CTL, ENB_TX_INT | ENB_ALL_RXP );
	}

	/*
	** if we have never initialized this particular channel
	** before, init it now
	*/
	if ( notinit[ dev ] )
	{
		coninit( dev );
		notinit[ dev ] = 0;
	}
	
	flags = tp->t_cflag;

	/*
	** if baud rate is zero, turn off the line
	*/
	if ( ! ( flags & CBAUD ) )
	{
		/*
		** turn off receiver and transmitter, but not on console
		**
		**	NOTE:
		**		Most drivers simply turn off the 
		**		device here. The signal is initiated
		**		because, since there are no modem signals,
		**		a 'carrier transition' interrupt will not
		**		occur. Normally, a 'carrier transition'
		**		interrupt would initiate this signal.
		*/
		signal( tp->t_pgrp, SIGHUP );

		if ( dev != CONDEV )
		{
			outb( ctl[ dev ], R3 );
			outb( ctl[ dev ], RX_8BIT );
			outb( ctl[ dev ], R5 );
			outb( ctl[ dev ], TX_8BIT );
		}
		return;
	}

	/*
	** set the baud rate
	*/
	setbaud( dev, flags & CBAUD );

	/*
	** initialize all the mode flags
	*/
	psmode = CLOCKX16;
	rmode = (unsigned char)0;
	xmode = ENB_TX | RTS | DTR;


	/*
	** set up the mode flags to reflect the
	** current state of t_cflags. At this point
	** in time, the only things that you can modify
	** are what the dz driver modified: receiver enable,
	** bits per character, parity enable, type of
	** parity, and stop bits.
	*/
	if ( flags & CREAD )
	{
		rmode |= ENB_RX;
	}

	if ( flags & CS6 )
	{
		rmode |= RX_6BIT;
		xmode |= TX_6BIT;
	}

	if ( flags & CS7 )
	{
		rmode |= RX_7BIT;
		xmode |= TX_7BIT;
	}

	if ( flags & PARENB )
	{
		psmode |= ENB_PARITY;
		if ( ! ( flags & PARODD ) )
		{
			psmode |= EVEN_PAR;
		}
	}

	if ( flags & CSTOPB )
	{
		psmode |= STOPBIT2;
	}
	else
	{
		psmode |= STOPBIT1;
	}

	/*
	** now ( finally ) program the chip with the modes
	** we just set up. Register 4 first.
	*/
	outb( port, R4 );
	outb( port, psmode );	/* parity/stop bit stuff	*/
	outb( port, R3 );
	outb( port, rmode );	/* receiver mode stuff		*/
	outb( port, R5 );
	outb( port, xmode );	/* transmit mode stuff		*/
}


/*
** conintr
**	Entry point for all interrupts from all
**	8274 devices. This routine figures out
**	why it was called, and dispatches to
** 	the appropriate routine to handle the condition.
*/
conintr( vec )
unsigned int	vec;
{
	unsigned char	reason;		/* reason why we are here	*/
	unsigned int	dev;		/* device # to pass to handler	*/

	for ( ;; )
	{
		outb( CHB_CTL, R2 );		/* vector is in RR2B	*/
		reason = inb( CHB_CTL );

		if ( inb( CHA_CTL ) & INT_PEND )
		{
			/*
			** If the vector we read from the chip is >= CHANA_INT,
			** the interrupt is from channel A, otherwise,it is from
			** channel B
			*/
			if ( ( reason & INTR_TYPE ) >= CHANA_INT )
				dev = CHA;
			else
				dev = CHB;

			switch ( reason & ~CHANA_INT )
			{
				case TX_INT:		/* xmit buffer empty */

					conxintr( dev );
					break;

				case EXTCHNG:		/* ext/status change */

					coneintr( dev );
					break;

				case RCVCHAR:		/* receive char avail */

					conrintr( dev );
					break;

				case SPECRCV:		/* special rcv cond */

					consrintr( dev );
					break;
			}
		}
		else
			break;
	}
	/*
	** must turn off all interrupts until the common interrupt
	** exit code because of another 8274 interrupt occurring
	** between INT_PEND not set, and the eoi to the 8259 PIC
	*/
	asm( "	cli" );
	eoi( vec );
}


/*
** conxintr
**	Transmit buffer empty interrupt handler
*/
conxintr( dev )
unsigned int	dev;
{
	register struct tty	*tp;
	unsigned int		ctlport,
				dataport;

	sysinfo.xmtint++;

	ctlport = ctl[ dev ];
	dataport = data[ dev ];
	tp = &con_tty[ dev ];

	/*
	** reset xmit interrupt pending
	*/
	outb( ctlport, RES_TXINT );

	/*
	** eoi to the device
	*/
	outb( CHA_CTL, EOI );

	/*
	** if we are supposed to send an xoff or xon,
	** do it now
	*/
	if ( tp->t_state & TTXON )
	{
		tp->t_state &= ~TTXON;
		tp->t_state |= BUSY;
		outb( dataport, CSTART );
	}
	else
	{
		if ( tp->t_state & TTXOFF )
		{
			tp->t_state &= ~TTXOFF;
			tp->t_state |= BUSY;
			outb( dataport, CSTOP );
		}
		else
		{
			/*
			** otherwise, just try to initiate more
			** output
			*/
			if ( tp->t_state & BUSY )
			{
				tp->t_state &= ~BUSY;
				conproc( tp, T_OUTPUT );
			}
		}
	}
}


/*
** coneintr
**	External/Status change interrupt handler.
**	With the Intel 286/10 board ignoring all of
**	these, we shouldn't get them. But, in case
**	we do, just ack them and get out of here.
*/
coneintr( dev )
unsigned int	dev;
{
	register struct tty	*tp;
	unsigned int		ctlport;
	char			c;

	sysinfo.mdmint++;

	ctlport = ctl[ dev ];
	tp = &con_tty[ dev ];

	/*
	** eoi to the device
	*/
	outb( CHA_CTL, EOI );

#ifdef MDMCTL286
	/*
	** if this is a break condition, and if we are not ignoring
	** breaks, and if we are supposed to interrupt on break,
	** tell the upper levels to do it.
	*/
	if ( inb( ctlport ) & BREAK )
	{
		if ( ! ( tp->t_iflag & IGNBRK ) )
		{
			if ( tp->t_iflag & BRKINT )
			{
				( *linesw[ tp->t_line ].l_input )( tp, L_BREAK );
			}
		}
	}
#endif

	/*
	** no matter what happened, reset the condition
	*/
	outb( ctlport, RES_EXT );
}


/*
** conrintr
**	Receive character available interrupt handler
*/
conrintr( dev )
unsigned int	dev;
{
	register struct tty	*tp;
	unsigned int		ctlport,
				dataport;
	unsigned char		c;

	sysinfo.rcvint++;

	ctlport = ctl[ dev ];
	dataport = data[ dev ];
	tp = &con_tty[ dev ];

	/*
	** eoi to the device
	*/
	outb( CHA_CTL, EOI );

	/*
	** while there are characters present...
	*/
	while ( inb( ctlport ) & RX_CHAR_AVAIL )
	{
		c = inb( dataport );		/* get the character	*/
		
		/*
		** if we aren't open, nobody to give the char to,
		** so chuck it.
		*/
		if ( ! ( tp->t_state & ( ISOPEN | WOPEN ) ) )
			continue;

		/*
		** if we are supposed to do xon/xoff protocol,
		** and the char we got is one of those guys,
		** take care of it.
		*/
		if ( tp->t_iflag & IXON )
		{
			unsigned char	ctmp;

			ctmp = c & 0x7F;

			/*
			** if we are stopped, and if the char is a xon, or if
			** we should restart on any char, resume
			*/
			if ( tp->t_state & TTSTOP )
			{
				if ( ctmp == CSTART || tp->t_iflag & IXANY )
					conproc( tp, T_RESUME );
			}
			else
			{
				if ( ctmp == CSTOP )
					conproc( tp, T_SUSPEND );
			}
			if ( ctmp == CSTART || ctmp == CSTOP )
				continue;	/* get more chars	*/
		}

		/*
		** if we are supposed to strip char to 7 bits, do it
		*/
		if ( tp->t_iflag & ISTRIP )
			c &= 0x7F;
		else
			c &= 0xFF;

		/*
		** if we don't have anywhere to put the char,
		** continue
		*/
		if ( tp->t_rbuf.c_ptr == NULL )
		{
			return;
		}

		/*
		** stuff the char into buffer
		** and signify that we have a char
		*/
		*tp->t_rbuf.c_ptr = c;
		tp->t_rbuf.c_count--;
		( *linesw[ tp->t_line ].l_input )( tp, L_BUF );
	}
}


/*
** consrintr
**	Special receive interrupt handler
*/
consrintr( dev )
unsigned int	dev;
{
	unsigned char		reason;
	unsigned char		lbuf[ 3 ];	/* local char buffer	*/
	unsigned char		c;
	unsigned int		lcnt;		/* cnt of chars in lbuf	*/
	unsigned int		flg;
	register struct tty	*tp;
	caddr_t 		tptr;
	unsigned int		ctlport,
				dataport;

	ctlport = ctl[ dev ];
	dataport = data[ dev ];
	tp = &con_tty[ dev ];

	/*
	** eoi to the device
	*/
	outb( CHA_CTL, EOI );

	flg = tp->t_iflag;

	/*
	** try to find out why we are here
	*/
	outb( ctlport, R1 );
	switch ( reason = ( inb( ctlport ) & ( RX_OVERRUN | PARITY_ERR ) ) )
	{
		case RX_OVERRUN:	/* receiver overrun		*/
		case RX_OVERRUN | PARITY_ERR:
			
			/*
			** if this happens, the best thing that we can
			** do is get out of here as quickly as possible,
			** to avoid another overrun
			*/
			break;

		case PARITY_ERR:	/* parity error			*/

			c = inb( dataport );	/* get the char		*/
			lcnt = 1;

			/*
			** if we have to check parity, we've got a lot of
			** work to do
			*/
			if ( flg & INPCK )
			{
				/*
				** but, if we are ignoring parity errors,
				** we get off easy. Otherwise we have to
				** mark the condition
				*/
				if ( ! ( flg & IGNPAR ) )
				{
					if ( flg & PARMRK )
					{
						lbuf[ 2 ] = 0xFF;
						lbuf[ 1 ] = 0;
						lcnt = 3;
						sysinfo.rawch += 2;
					}
					else
						c = 0;
				}
				else
				{
					/*
					** throw the char away
					*/
					break;
				}
			}

			/*
			** if we are supposed to do xon/xoff protocol,
			** and the char we got is one of those guys,
			** take care of it.
			*/
			if ( tp->t_iflag & IXON )
			{
				unsigned char	ctmp;
	
				ctmp = c & 0x7F;
	
				/*
				** if we are stopped, and if the char
				** is a xon, or if we should restart 
				** on any char, resume
				*/
				if ( tp->t_state & TTSTOP )
				{
					if ( ctmp == CSTART ||
							tp->t_iflag & IXANY )
						conproc( tp, T_RESUME );
				}
				else
				{
					if ( ctmp == CSTOP )
						conproc( tp, T_SUSPEND );
				}
				if ( ctmp == CSTART || ctmp == CSTOP )
					break;
			}
	
			/*
			** if we are supposed to strip char to 7 bits, do it
			*/
			if ( tp->t_iflag & ISTRIP )
				c &= 0x7F;
			else
				c &= 0xFF;

			/*
			** stash the char
			*/
			lbuf[ 0 ] = c;

			/*
			** if we go no place to put the char, get out
			*/
			if ( tp->t_rbuf.c_ptr == NULL )
			{
				break;
			}

			tptr = tp->t_rbuf.c_ptr;
			while ( lcnt )
			{
				*tptr++ = lbuf[ --lcnt ];
				tp->t_rbuf.c_count--;
				( *linesw[ tp->t_line ].l_input )( tp, L_BUF );
			}
			break;

		default:

			break;

	}

	/*
	** reset the error
	*/
	outb( ctlport, RES_ERR );
}


/*
** conproc
**	General command routine that initiates action
*/
conproc( tp, cmd )
register struct tty	*tp;
int			cmd;
{
	register int	c;	/* char to process	*/
	extern		ttrstrt();
	unsigned int	port;
	int		dev;

	/*
	** get device number and control port
	*/
	dev = tp - con_tty;
	port = ctl[ dev ];

	/*
	** based on cmd, do various things to the device
	*/
	switch ( cmd )
	{
		case T_TIME:		/* stop sending a break	*/

			tp->t_state &= ~TIMEOUT;
			state( dev, SND_BREAK, OFF );
			goto start;

		case T_WFLUSH:		/* output flush		*/

			tp->t_tbuf.c_size -= tp->t_tbuf.c_count;
			tp->t_tbuf.c_count = 0;
			/* FALL THROUGH */

		case T_RESUME:		/* enable output 	*/

			tp->t_state &= ~TTSTOP;
			/* FALL THROUGH */
		
		case T_OUTPUT:		/* do some output	*/
start:
		{
			register struct ccblock *tbuf;

			tbuf = &tp->t_tbuf;
			/*
			** if we are stopped, timed-out, or
			** just plain busy, don't do anything
			*/
			if ( tp->t_state & ( TIMEOUT | TTSTOP | BUSY ) )
				break;

			if ( tbuf->c_ptr == 0 || tbuf->c_count <= 0 )
			{
				if ( tbuf->c_ptr )
					tbuf->c_ptr -= tbuf->c_size - 
								tbuf->c_count;
				if ( !( CPRES & ( *linesw[ tp->t_line ].l_output )( tp ) ) )
				{
					break;
				}
			}
			tp->t_state |= BUSY;
			outb( data[ dev ], *tbuf->c_ptr++ );
			tbuf->c_count--;
			break;
		}

		case T_SUSPEND:		/* block on output	*/
			
			tp->t_state |= TTSTOP;
			break;

		case T_BLOCK:		/* block on input	*/

			/*
			** either we send a xoff right now, or
			** we make sure that the next char sent
			** is an xoff.
			*/
			tp->t_state &= ~TTXON;
			tp->t_state |= TBLOCK;
			if ( tp->t_state & BUSY )
				tp->t_state |= TTXOFF;
			else
			{
				tp->t_state |= BUSY;
				outb( data[ dev ], CSTOP );
			}

			break;

		case T_RFLUSH:		/* flush input		*/
			
			/*
			** if we are not blocked on input
			** nothing to do
			*/
			if ( ! ( tp->t_state & TBLOCK ) )
				break;

			/* FALL THROUGH */

		case T_UNBLOCK:		/* enable input		*/

			tp->t_state &= ~( TTXOFF | TBLOCK );

			/*
			** Again, if we are not busy, send an xon
			** right now. Otherwise, make sure that the next
			** char out will be an xon
			*/
			if ( tp->t_state & BUSY )
				tp->t_state |= TTXON;
			else
			{
				tp->t_state |= BUSY;
				outb( data[ dev ], CSTART );
			}

			break;

		case T_BREAK:		/* send a break		*/
			
			state( dev, SND_BREAK, ON );
			tp->t_state |= TIMEOUT;
			timeout( ttrstrt, tp, HZ / 4 );
			break;

		case T_SWTCH:		/* shell layer switch	*/
			
			/*
			** nothing for us to do
			*/
			break;

		case T_PARM:		/* update parameters	*/

			conparam( dev );
			break;

		default:

			break;
	}
}


/*
** coninit
**	initialize an 8274 port
*/
coninit( dev )
{
	unsigned int	x,	/* value for spl			*/
			i;	/* counter				*/

	/*
	** reset the channel,
	** then initialize the channel.
	** Interrupts are turned off to guard against spurious
	** interrupts.
	*/
	x = spl4();

	if ( dev == CHA )
	{
		for ( i = 0; i < sizeof( chainit ); i++ )
		{
			outb( CHA_CTL, chainit[ i ] );
		}

	}
	else
	{
		for ( i = 0; i < sizeof( chbinit ); i++ )
		{
			outb( CHB_CTL, chbinit[ i ] );
		}
	}

	splx( x );
}


/*
** setbaud
**	Set the baud rate for a particular device by
**	kicking the appropriate 8254 timer
*/
setbaud( dev, select )
int		dev;
unsigned int	select;
{
	unsigned int	cntport;	/* to ease access to port	*/
	unsigned char	cntmode;	/* to ease access to mode	*/
	unsigned int	rate;
	unsigned int	x;

	cntport = brgcnt[ dev ];
	cntmode = brgmode[ dev ];

	rate = con_speeds[ select ];

	x = spl4();
	outb( CTCMODE, cntmode );
	outb( cntport, (char)rate );
	outb( cntport, (char)( rate >> 8 ) );
	splx( x );
}

/*
** state
**	Change state of device by turning on or off
**	a break condition
**
**	NOTE:
**		This routine has been modified to only
**		handle starting and stopping a break because
**		Intel hardware doesn't support anything else
*/
state( dev, cond, on )
int		dev,
		on;
unsigned char	cond;
{
	struct tty	*tp;
	unsigned int	s = ENB_TX | RTS | DTR;

	tp = &con_tty[ dev ];

	/*
	** first, set s to proper number of
	** bits per char
	*/
	if ( tp->t_cflag & CS6 )
		s |= TX_6BIT;
	if ( tp->t_cflag & CS7 )
		s |= TX_7BIT;

	/*
	** now, output the state back to the chip,
	** modifying the state to reflect the condition
	*/
	outb( ctl[ dev ], R5 );
	outb( ctl[ dev ], on ? s | cond : s & ~cond );
}
