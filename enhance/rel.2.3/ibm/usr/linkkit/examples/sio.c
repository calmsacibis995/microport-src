static char *uportid = "@(#)sio.c	Microport Rev Id  1.3.5 6/26/86";
/*
(#)sio.c	1.10
**      IBM Serial/Parallel  Adapter driver
** Written for the AT clone machines 9/15/85
*/

#define	MODEM	/* Do modem interpretation ! 

extern	int	sio_chans;
#define	UNIT(x)		(x & 0x7)

#define ON      1
#define OFF     0

int dotime = 0;

extern struct tty	sio_tty [ ];
extern int		sio_open[ ];
extern int		sio_flag[ ];/* Stty flags for this line */

#define	TTYSPL		spl7

extern unsigned int    ctl[ ];		/* 16450 control ports */
extern unsigned int  intnum[ ]; 	/* 16450 interrupt vectors */

/* counter values for the baud rate generator. */
unsigned int    sio_speeds[] =
{
	0,	 /* 0 baud                   */
	2304,	 /* 50 baud                  */
	1536,	 /* 75 baud                  */
	1047,	 /* 110 baud                 */
	857,	 /* 134.5 baud               */
	768,	 /* 150 baud                 */
	576,	 /* 200 baud                 */
	384,	 /* 300 baud                 */
	192,	 /* 600 baud                 */
	96,      /* 1200 baud                */
	64,      /* 1800 baud                */
	48,      /* 2400 baud                */
	24,      /* 4800 baud                */
	12,      /* 9600 baud                */
	6,       /* 19200 baud (exta speed)  */
};

int	siodebug = 0;		/* Can't patch, so have special ioctl */

/*
** sioopen
**      Open a device
*/
sioopen( dev, flag )
int     dev,
	flag;
{

	/* minor device validation 
	 */
	if ( unit >= sio_chans )
	{
		u.u_error = ENXIO;
		return;
	} 

	tp = &sio_tty[ unit ];	   /* point to correct tty struct  */

	 Do diagnostic to make sure there's a comm port there; 
	 Check read/write modem (&int driver) outputs reg;
				
	if ( not open or not waiting for an open)
	{				/* then initialize tty */
		reset state machine;
		ttinit( tp );
		tp->t_proc = sioproc;
		sioparam( unit );
	}

	/* This should be in the T_INPUT case in sioproc(),
	 * which would require a t_dev element in the tp structure. 
	 */
	if (this isn't a modem device) {
		/* Can't call out, if someone's called us */
		if (someone's calling in on a modem) {
			if (this unit's already open)
				set device state machine to not open;
			u.u_error = EIO;
			return;
			}
		sio_open[ unit ] |= LOCAL_OPEN;
	} else {
		set flag indicating an open on a modem device;
		turn off CLOCAL;
	}
	sio_flag[ unit ] = tp->t_cflag;
	if (this is a modem) {
		if (we've got data carrier detect)
			set appropriate status bits;
		else {
			if (this device hasn't been opened)
				turn off carrier status;
			mark the device state as opened;
			/* Incoming modem procs (getty's) wait until 
			 * local opens are gone */
			enable serial io irq's;
			while (carrier isn't on || (this is a call-in device 
				&& it hasn't been opened yet)) {
				/* siosintr will wake us up */
				sleep((caddr_t) &sio_flag[unit], TTOPRI);
				}
			set modem open status bits in sio_open[unit];
			if (this is a call-in device)
				mark it as opened;
			}
	} else				/* not a modem device */
		{
		/* having these here allow modem sleep/wakeup to work */
		tp->t_state &= ~WOPEN;
		tp->t_state |= CARR_ON | ISOPEN;
		}

	setints( unit );

	/* execute the common open code for this line discipline */
	( *linesw[ tp->t_line ].l_open )( tp );
}

/*
** sioclose
**      Close a device
*/
sioclose( dev )
int     dev;
{
	/* execute the common close code for this line discipline */
	( *linesw[ tp->t_line ].l_close )( tp );
				
	if (not a modem device)
		mark status bits as "not opened";
	} else {
		/* kill signal during wait will call close routine,
		 * so clear WAIT 
		 */
		sio_open[ unit ] &= ~(MODEM_OPEN|MODEM_WAIT);
		if (this is a dial-in device)
			sio_open[ unit ] &= ~INCOMING_OPEN;
	}
	/* if still open, return */
	if (sio_open[unit]) {	/* recode this line for CTS */
	    /* If hangup flag but local waiting, drop DTR/RTS for a second */
	    return;
	    }

	/* turn off the interrupt enables for this line 
	** Interrupts are turned off to guard against spurious
	** interrupts.
	*/
	splx( x ); 			/* reenable interrupts to the system */

	/* set the init flag to 0 so the next open will re-enable interrupts */
	/* set all of the various flags as unitialized */
}

/*
** sioread
**      Read from a device
*/
sioread( dev )
int     dev;
{
	register struct tty     *tp;

	tp = &sio_tty[ UNIT(dev) ];	   /* point to correct tty struct  */

	/* execute the common read code for this line discipline */
	( *linesw[ tp->t_line ].l_read )( tp );
}


/*
** siowrite
**      Write to a device
*/
siowrite( dev )
int     dev;
{
	register struct tty     *tp;

	tp = &sio_tty[ UNIT(dev) ];	   /* point to correct tty struct  */

	/* execute the common write code for this line discipline */
	( *linesw[ tp->t_line ].l_write )( tp );
}


/*
** sioioctl
**      Call the common ioctl code, and
**      modify the state of the device if necessary
*/
sioioctl( dev, cmd, arg, mode )
unsigned int    dev,
		cmd,
		mode;
union ioctl_arg arg;
{
	int	unit;
	int		x;

	
	if (the ioctl is one in which the chip parameters have to be changed)
		 change them;
}


/*
** sioparam
**      Modify the state of the device as indicated in
**      the tty structure
*/
sioparam( unit )
int     unit;
{
	tp = &sio_tty[ unit ];	   /* point to correct tty struct */

	/* initialize the mode flag */

	/*
	** if we have never initialized this particular channel
	** before, init it now
	*/
	if ( sio_open[ unit ] == 0 )
	{
		/*
		** Interrupts are turned off to guard against spurious
		** interrupts.
		*/
		x = TTYSPL();
		initialize channel;
		splx( x ); 		/* reenable interrupts to the system */
	}

	establish current flags;

	/*
	** set up the mode flags to reflect the
	** current state of t_cflags. At this point
	** in time, the only things that you can modify
	** are : 
	** bits per character, parity enable, type of
	** parity, and stop bits.
	*/

	/* initialize the mode flag */
	rmode = (unsigned char) 0;

	determine current mode, based on flags setting;

	if (we're supposed to set parity)
		set parity (either odd or even);

	if (we're supposed to set the stop bits (CSTOPB))
		set mode |= STOPBIT2;

	
	now ( finally ) program the chip with the modes we just set up;

	/* if baud rate is zero, drop modem lines and wait for 
	 * other end to drop, or fake it if we're not using modem control 
	 */
	if ( we're hanging up this line )
	{
			if (this is a dial-in line)
				kick the hardware with a initialization cmd;
			else
				signal the line's process group with a SIGHUP;
			return;
	} else {
		/* do these every time, to counteract 'stty 0' */
		tell hardware to set DTR, RTS, and initialize
		}

	/* set the baud rate */
}

/* siointr0 & siointr1 have been moved to siconf.c and dgconf.c,
 * as they differ for the two boards.  If the SIO assembler code is used,
 * it implements siointr0, siointr1, siointr, & sioxintr and directly
 * calls siorintr, siosintr, siosrintr, and sioproc(T_OUTPUT).
 */

/*
** siointr
**      Entry point for all interrupts from all
**      16450 devices. This routine figures out
**      why it was called, and dispatches to
**      the appropriate routine to handle the condition.
*/
siointr( unit )
unsigned int    unit;
{
	unsigned char   reason;	 	/* reason why we are here */
	unsigned int baseport;		/* access to the 16450 */
	int	x;

	for ( ;; )
	{
		reason = ...		/* read from the IIR */

		if ( there isn't an interrupt pending ) {
			switch (reason) {
				case transmit buffer is empty:
					sioxintr( unit );
					break;
				case receive buffer is full:
					siorintr( unit );
					break;
				case modem status needs servicing:
					sioeintr( unit );
					break;
				default: 
					siosrintr( unit );
					break;
			}
		}
		else {
				break;
			}
	}
}
	
/*
** sioxintr
**      Transmit buffer empty interrupt handler
*/
sioxintr( unit )
unsigned int    unit;
{
	register struct tty     *tp;

	ONGUARD;
	sysinfo.xmtint++;

	tp = &sio_tty[ unit ];

	if (we've timed out, or have stopped)
		UNGUARD;
		return;
		}

/* Recode TTXON/OFF to leave start/stop chars at head of queue, 
 * that way 90% of sioxintr disappears!
 */
	if (we are supposed to send an xoff or xon) {
		do it;
	} else {
		/* otherwise, just try to initiate more output */
		tp->t_state &= ~BUSY;
		UNGUARD;
		sioproc( tp, T_OUTPUT );
		return;
	}
	UNGUARD;
}


/*
** sioeintr
**   handle the external status changes to the chip.  These are
**   the Clear To Send status, the Data Terinal Ready status, the 
**   Ring Indicator status, and the Data Carrier Detect status
*/
sioeintr( unit )
unsigned int    unit;
{
	register struct tty     *tp;
	unsigned char		mode;

	sysinfo.mdmint++;

	tp = &sio_tty[ unit ];
	/* clear the interrupt bit in the chip */
	if (Data Carrier Detect) {
	    if (sio_open[unit] & (MODEM_WAIT|MODEM_OPEN)) {
			set carrier on flag;
		    if ( we're waiting for the modem to go ready ) {
			    wakeup((caddr_t) &sio_flag[unit]);
			    }
		    }
	} else {	/* Carrier dropped, we send a hangup signal */
		set carrier off flag;
		signal( tp->t_pgrp, SIGHUP );
	} 

	return;
}

/*
** siorintr
**      Receive character available interrupt handler
*/
siorintr( unit )
unsigned int    unit;
{
	register struct tty     *tp;

	sysinfo.rcvint++;
	tp = &sio_tty[ unit ];

	while ( there are characters present )
	{
		get the character from the port;
		if (we aren't open)  /* nobody to give the char to, discard */
			continue /* continue */;

		/*
		** if we are supposed to do xon/xoff protocol,
		** and the char we got is one of those guys,
		** take care of it.
		*/
		if ( xon/xoff protocol )
		{
			if (we are stopped, and if the char is a xon)
				resume;
			else if ( we should restart on any char) 
				resume;

			if (we got start or stop bits)	
				continue;	/* get more chars */
		}

		/* if we are supposed to strip char to 7 bits, do it */

		if ( we don't have anywhere to put the character )
		{
			return;
		}

		x = TTYSPL();
		/* stuff the char into buffer and signify that we have a char */
		*tp->t_rbuf.c_ptr = c;
		tp->t_rbuf.c_count--;
		( *linesw[ tp->t_line ].l_input )( tp, L_BUF );
		splx(x);
	}
}


/*
** siosrintr
**      Special receive interrupt handler
**	NOTE: asm interface requires that 'unit' be left alone!
*/
siosrintr( unit )
unsigned int    unit;
{
	unsigned char	   reason;
	unsigned char	   lbuf[ 3 ];	/* local char buffer */
	unsigned char	   c;
	unsigned int	    lcnt;	   /* cnt of chars in lbuf */
	unsigned int	    flg;
	register struct tty     *tp;
	caddr_t		 tptr;

	tp = &sio_tty[ unit ];

	flg = tp->t_iflag;

	/* try to find out why we are here */
	switch ( reason )
	{
		case receiver overrun:
			/*
			** if this happens, the best thing that we can
			** do is get out of here as quickly as possible,
			** to avoid another overrun
			*/
			break;

		case PARITY_ERR:	/* parity error */

			get the char;
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
				...
			}

			/*
			** if we are supposed to do xon/xoff protocol,
			** and the char we got is one of those guys,
			** take care of it.
			*/
			if ( we're supposed to xon/xoff protocol )
			{
				if ( we're stopped )
				{
					if ( we should restart ||
						char. is an xon char.) {
						      sioproc( tp, T_RESUME );
					}
				}
				else
				{
					if ( character is a stop character )
						sioproc( tp, T_SUSPEND );
				}
				if ( character is an XON or XOFF character )
					break;
			}
	
			/* if we are supposed to strip char to 7 bits, do it */

			/* stash the char */
			lbuf[ 0 ] = c;

			if (we have no place to put the char)
			{
				break;
			}

			tptr = tp->t_rbuf.c_ptr;
			x = TTYSPL();
			while ( lcnt )
			{
				*tptr++ = lbuf[ --lcnt ];
				tp->t_rbuf.c_count--;
			}
			( *linesw[ tp->t_line ].l_input )( tp, L_BUF );
			splx(x);
			break;
		case BREAKIN:
			/*
			** if this is a break condition, and
			** if we are not ignoring
			** breaks, and if we are supposed to interrupt on break,
			** tell the upper levels to do it.
			*/
			...
			break;
		default:

			break;
	}
}


/*
** sioproc
**      General command routine that initiates action
*/
sioproc( tp, cmd )
register struct tty     *tp;
int		     cmd;
{
	register int    c;      /* char to process */
	extern	  ttrstrt();
	int	     unit, siooutput();
	sd_t	*sdp;

	get device number, control baseport, and pseudo-dma structure;

	/* based on cmd, do various things to the device */
	switch ( cmd )
	{
		case T_TIME:	    
			stop sending a break;
			goto start;

		case T_WFLUSH:	  
			output flush;
			/* FALL THROUGH */

		case T_RESUME:	  
			enable output;
			/* FALL THROUGH */
		
		case T_OUTPUT:	  /* do some output */
start:
		{

			tbuf = &tp->t_tbuf;
		/* Don't need guards, resume only happens when no int pending */

			if (we are stopped, timed-out, or busy) 
			{
				break;
			}

			if ( tbuf->c_ptr == 0 || tbuf->c_count <= 0 )
			{
				if ( tbuf->c_ptr )
				   tbuf->c_ptr -= tbuf->c_size - tbuf->c_count;
				if (!(CPRES&(*linesw[tp->t_line].l_output)(tp)))
				{
					break;
				}
			}
			/* if one of above conditions occurs, 
			 * wait for its interrupt
			 */
			ONGUARD;
			tp->t_state |= BUSY;
			outb( baseport , *tbuf->c_ptr++ );
			tbuf->c_count--;
			UNGUARD;
			break;
		}

		case T_SUSPEND:	 /* block on output */
			
			tp->t_state |= TTSTOP;
			break;

		case T_BLOCK:	   /* block on input */

			/*
			** either we send a xoff right now, or
			** we make sure that the next char sent
			** is an xoff.
			*/
			break;

		case T_RFLUSH:	  /* flush input */
			
			if (we are not blocked on input)
				break;		/* nothing to do */

			/* FALL THROUGH */

		case T_UNBLOCK:	 /* enable input */

			tp->t_state &= ~( TTXOFF | TBLOCK );

			/*
			** Again, if we are not busy, send an xon
			** right now. Otherwise, make sure that the next
			** char out will be an xon
			*/
			if ( tp->t_state & BUSY ) {
				tp->t_state |= TTXON;
			} else
			{/* This code might be a problem, if it restarts dma */
				tp->t_state |= BUSY;
				outb( baseport, CSTART );
			}

			break;

		case T_BREAK:	   
			send a break;
			break;

		case T_SWTCH:	   /* shell layer switch */
			
			/*
			** nothing for us to do. at present, shell
			** layering isn't supported.
			*/
			break;

		case T_PARM:	    /* update parameters */

			sioparam( unit );
			break;

		default:

			break;
	}
}

siooutput(val)
caddr_t val;
{
	sioproc(sio_tty[val], T_OUTPUT);
}

/*
** setbaud
**      Set the baud rate for a particular device
*/
setbaud( unit, select )
int	     unit;
unsigned int    select;
{
	unsigned int    rate;
	unsigned int    x;
	unsigned char   mode;
	
	mode = inb(modeaddress);
	rate = sio_speeds[ select ];
	x = TTYSPL();
	send the appropriate commands for selecting baud rate to the hardware;
	splx( x );
}

/*
** state
**      Change state of device by turning on or off
**      a break condition
**
**      NOTE:
**	      This routine has been modified to only
**	      handle starting and stopping only the break 
*/
state( unit, cond, on )
int	     unit,
		on;
unsigned char   cond;
{
	struct tty      *tp;
	unsigned int    s;

	tp = &sio_tty[ unit ];

	/* first, set s to current line control register value */
	s = inb( line control register address );

	/*
	** now, output the state back to the chip,
	** modifying the state to reflect the condition
	*/
	outb( lcr address, on ? (s | cond) : (s & ~cond) );
}

static 
setints( unit )
{
	if ((sio_open[ unit ] & (MODEM_OPEN | MODEM_WAIT | CTS_OPEN)))
		outb( control address, ENB_ALL_INT );
	else
		outb( control address, ENB_TX_INT | ENB_RX_INT | ENB_LS_INT);		
}

/* timer to compensate for lost interrupts */
sitimer(val) 
caddr_t val;
{
	if (sio_open[ val ]) {
		siointr( val );
		timeout(sitimer, (caddr_t) val, HZ);
		}
}
