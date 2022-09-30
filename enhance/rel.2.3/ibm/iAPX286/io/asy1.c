static char *uportid = "@(#)asy.c	Microport Rev Id  1.3.5 6/26/86";
/*
(#)asy.c	1.10
**      IBM Serial/Parallel  Adapter driver
** Written for the AT clone machines 9/15/85
** M000 - uport!mark: added vector for ast board, moved UNIT to asycnt.h
** M001 - uport!markc: restore MODEM interrupt control in setints()
** M002 - uport!markc: catch missed Tx Holding Reg Empty Interrupts in 8250"B"s
** M003 - uport!markc: protect open, restore Modem/getty feature
** M004, fredo, Fri Feb  6 10:32:29 PST 1987
** 	Fixed asyopen problem
*/
#include "sys/types.h"
#include "sys/param.h"
#include "sys/systm.h"
#include "sys/conf.h"
#include "sys/dir.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/errno.h"

#ifdef MP386
#include "sys/immu.h"
#include "sys/region.h"
#endif /* MP386 */

#include "sys/proc.h"
#include "sys/file.h"
#include "sys/tty.h"
#include "sys/termio.h"
#include "sys/sysinfo.h"
#include "sys/16450.h"
#include "sys/clock.h"
#include "sys/ioctl.h"
#include "sys/ppi.h"
#include "sys/asycnt.h"		/* M000 */
#ifdef ATMERGE 
#include "sys/realmode.h"
#endif

#define	MODEM		/* Do modem interpretation ! Modem signals don't work! */
#undef	CTSCTL		/* Code not up to date, its not a terminal mode (yet)  */
			/* This doesn't know about PSEUDODMA. */
#define	PSEUDODMA	/* T_OUTPUT into phony dma queue with low-level interrupt */

#ifdef	MODEM
/* Requires that carrier-detect reflect DCD input */
#define	MDMMINOR(dev)	(dev & 0x80)
/* Don't wake up on DCD-up until non-MDM procs give up tty */
#define	INCOMING(dev)	((dev & 0xc0) == 0xc0)

#endif	/* MODEM */

#ifdef	CTSCTL
#define	CTSMINOR(dev)	(dev & 0x20)
#endif	/* CTSCTL */

#define	LOCAL_OPEN		1			/* line owned by callout device */
#define	MODEM_WAIT		2			/* callin device is waiting to own line */
#define	MODEM_OPEN		4			/* callin device using line */
#define	INCOMING_OPEN	8			/* line owned by callin device */
#define	CTS_OPEN		16			/* RST/CTS signalling - not implemented */

#define	ONGUARD	{asm("	pushf"); asm("	cli");}
#define	UNGUARD	{asm("	popf");}

extern	int	asy_chans;

#define ON      1
#define OFF     0

int dotime = 0;

extern struct tty	asy_tty [ ];
extern int		asy_open[ ];
extern int	asy_flag[ ];/* Stty flags for this line */

#ifdef	PSEUDODMA
#define	QLEN	(64-24)				/* Must be power of 2 */
/* If the size of this is changed, update asm and conf files */
struct asydma {
	short		sd_open;			/* is this channel open?  */
	short		sd_ptr;			/* queue[ptr] = next char to be sent */
	unsigned		short sd_base;		/* base I/O port */
	struct tty	*sd_tp;			/* tty struct for this channel */
	short		*sd_state;		/* &sd_tp->t_state, NO struct offsets */
							/* ttout doesn't inform driver on TIMEOUT */
	struct asydma	*sd_sdp;			/* chain to next asydma struct */
	int			*sd_active;		/* pointer to si_active[intnumber] */
	short		sd_unit;			/* unit number of this entry */
	char			sd_queue[QLEN];	/* at end for simpler asm code */
};
typedef struct asydma sd_t;
extern sd_t	sd_array[];
#endif	/* PSEUDODMA */

#define	TTYSPL		spl7

extern unsigned int    ctl[ ];		/* 16450 control ports */
extern unsigned int  intnum[ ]; 	/* 16450 interrupt vectors */

/* counter values for the baud rate generator. */
unsigned int    asy_speeds[] =
{
	0,	  /* 0 baud                   */
	2304,	  /* 50 baud                  */
	1536,	  /* 75 baud                  */
	1047,	  /* 110 baud                 */
	857,	  /* 134.5 baud               */
	768,	  /* 150 baud                 */
	576,	  /* 200 baud                 */
	384,	  /* 300 baud                 */
	192,	  /* 600 baud                 */
	96,	  /* 1200 baud                */
	64,	  /* 1800 baud                */
	48,	  /* 2400 baud                */
	24,	  /* 4800 baud                */
	12,	  /* 9600 baud                */
	6, 	  /* 19200 baud (exta speed)  */
};


#ifdef DEBUG 
#undef DEBUG
#endif

#define	DEBUG
#ifdef	DEBUG
#define	debprf	if (asydebug) printf
#define	debput	if (asydebug) putchar
#endif	/* DEBUG */

int	asydebug = 0;		/* Can't patch, so have special ioctl */
int	si_active[NINT];	/* data for interrupt handlers */
					/* bit 1 means a port was serviced when */
					/* the chain was polled */
extern 	int vector[];		/* M000 - add a vector per interrupt */
int	si_ints[NCHAN] = {4, 3};	/* interrupt numbers for units */
int	mcr[NCHAN] = {0xc, 0xc, 0xc, 0xc, 0xc, 0xc, 0xc, 0xc, 0xc, 0xc, 0xc, 0xc, 0xc, 0xc, 0xc, 0xc, 0xc, 0xc, 0xc, 0xc, 0xc, 0xc, 0xc, 0xc, 0xc, 0xc, 0xc, 0xc, 0xc, 0xc, 0xc, 0xc};	/* modem control register values for units M000 */
struct asydma	*si_chain[NINT] = {0L,0L,0L,0L,0L,0L,0L,0L,0L,0L,0L,0L,0L,0L,0L,0L};	/* head of int chain for each int */

/*
** asyopen
**      Open a device
*/
asyopen( dev, flag )
int     dev,
	flag;
{
	register struct tty     *tp;
	extern		 asyproc();
	int		i, unit, baseport, prflag;
	char 	rmode;
	int		sitimer();
	extern hz;
	int x;		/* M003 */
#ifdef ATMERGE 
	int comport;
#endif /* ATMERGE */

     extern int      (*ivect[])();
     extern int      asyintr0();
     extern int      asyintr1();

#ifdef MP386
     static int      firsttime = 1;		/* M004 */

	 	/* M004 quick fix based on Locus' modifications
		 * to ivect table in conf.c
		 */
     if (firsttime)  {
		firsttime = 0;
 		ivect[4] = asyintr0;   /* IBM COM1 */
		ivect[3] = asyintr1;   /*  "  COM2 */
	}
#endif	/* MP386 */

#ifdef	DEBUG
	debprf("asyopen: dev %d\n", dev);
#endif	/* DEBUG */
	unit = UNIT(dev);

	/* minor device validation 
	 * change this to device validation, 16450 has a quiet loopback mode
	 */
	if ( (unit >= asy_chans ) || (ctl[ unit ] == 0) )
	{
			u.u_error = ENXIO;
			return;
	} 

#ifdef ATMERGE
	/* try to get control of the device */
	if (unit == 0) 
		comport = DEVN_COM0;
	else
		comport = DEVN_COM1;
	if (devclaim(comport)) {
		u.u_error = EBUSY;	/* DOS has the device */
		return;
	}
#endif /* ATMERGE */
	tp = &asy_tty[ unit ];	   /* point to correct tty struct  */
	baseport = ctl[ unit ];

    /* M003 this may screw up an already open comm port, so move within the
	open check M003 */
				
	/*
	** if we are not open, or not waiting for open
	** then we should initialize this tty
	*/
	if ( ! ( tp->t_state & ( ISOPEN | WOPEN ) ) && !asy_open[unit] )/*M003*/
	{
	    /* M003 move diagnostic to within open check M003 */
	    /* Do diagnostic to make sure there's a comm port there */
	    /* Check read/write modem (&int driver) outputs reg */
	    rmode = inb(baseport + 1);
	    rmode = inb(baseport + 1);
	    x = TTYSPL();		/* to prevent spurious interrupts M003 */
	    for(i=0;i<4;i++) {	/* Don't turn on interrupts */
		    outb(baseport + 1, i);
		    if (inb(baseport + 1) != i) {
			    u.u_error = EIO;
			    return;
			    }
		    }
	    outb(baseport + 1, rmode);
	    splx( x );					/* M003 */
	    /* end M003 move */
		tp->t_state = 0;     /* wipe out trash from old opens/closes */
		ttinit( tp );
		mksd(unit);			/* init before interrupts */
		tp->t_proc = asyproc;
		asyparam( unit );
		tp->t_state &= ~ISOPEN;
		tp->t_state |= WOPEN;
		if (dotime)
			timeout(sitimer, (caddr_t) unit, hz);  
				/* half-a-second int timer */
	}
	

#ifdef	MODEM
	/* This should be in the T_INPUT case in asyproc(),
	 * which would require a t_dev element in the tp structure. 
	 */
	if (! MDMMINOR(dev)) {
		/* Can't call out, someone's called us */
		if (asy_open[ unit ] & INCOMING_OPEN) {
			if (tp->t_state & WOPEN)
				tp->t_state &= ~(WOPEN);
			u.u_error = EIO;
			return;
			}
		asy_open[ unit ] |= LOCAL_OPEN;
	} else {
		asy_open[ unit ] |= MODEM_WAIT;
		tp->t_cflag &= ~CLOCAL;
	}
	asy_flag[ unit ] = tp->t_cflag;
	if (MDMMINOR( dev )) {
		if (inb(baseport + 6) & DCD)
			tp->t_state |= CARR_ON;
		else {
			if (!(tp->t_state & ISOPEN)) /* don't foul up other users of port */
				tp->t_state &= ~CARR_ON;
			tp->t_state |= WOPEN;
			/* Incoming modem procs (getty's) wait until local opens are gone */
			prflag = 0;
		    }/* M003 *//* just cause carrier is on don't mean nothing */
			setints( unit );
			while ( !(tp->t_state & CARR_ON) || 
					(INCOMING( dev ) && (asy_open[ unit ] & LOCAL_OPEN))) {
#ifdef	DEBUG
				debprf("SIOopen: dev 0x%x sleeping on DCD\n", dev);
#endif	/* DEBUG */
				prflag = 1;
				/* asyeintr will wake us up */
				sleep((caddr_t) &asy_flag[unit], TTOPRI);
				}
#ifdef	DEBUG
			if (prflag)
				debprf("SIOopen: dev 0x%x arises!\n", dev);
#endif	/* DEBUG */
			asy_open[ unit ] &= ~MODEM_WAIT;
			asy_open[ unit ] |= MODEM_OPEN;
			if (INCOMING(dev))
				asy_open[ unit ] |= INCOMING_OPEN;
	} else 
#endif	/* MODEM */
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
** asyclose
**      Close a device
*/
asyclose( dev )
int     dev;
{
	register struct tty     *tp;
	unsigned int	    baseport; /* to access 16450 base port */
	unsigned char	   	rmode;    /* local mode value */
	unsigned int  		x;    	  /* value for spl */
	int  				unit;    	  /* value for spl */
	extern hz;
#ifdef ATMERGE
	int comport;
#endif /* ATMERGE */

	unit = UNIT(dev);
#ifdef	DEBUG
	debprf("asyclose: dev %d\n", dev);
#endif	/* DEBUG */
	tp = &asy_tty[ unit ];	     /* point to correct tty struct */
	baseport = ctl[ unit ];

	/* execute the common close code for this line discipline */
	( *linesw[ tp->t_line ].l_close )( tp );
				
#ifdef	MODEM
	if (! MDMMINOR(dev)) {
		asy_open[ unit ] &= ~LOCAL_OPEN;
	} else {
		/* kill signal during wait will call close routine, so clear WAIT */
		asy_open[ unit ] &= ~(MODEM_OPEN|MODEM_WAIT);
		if (INCOMING(dev))
			asy_open[ unit ] &= ~INCOMING_OPEN;
	}
	/* if still open, return */
	if (asy_open[unit]) {	/* recode this line for CTS */
	    /* If hangup flag but local waiting, drop DTR/RTS for a second */
	    if (asy_flag[ unit ] & HUPCL) {
		inb( baseport + 6);
		inb( baseport + 6);
		outb( baseport + 4, INTDRIV); 
		delay(hz * 2);
		outb( baseport + 4, INTDRIV | DTR | RTS); 
		}
	    return;
	    }
#endif	/* MODEM */

	/* turn off the interrupt enables for this line 
	** Interrupts are turned off to guard against spurious
	** interrupts.
	*/
	rmode = (unsigned char) 0;
	x = TTYSPL();
	rmsd( unit );			/* remove unit from chain */

	outb( baseport + 1, rmode);	/* disable all device interupts */
	rmode = inb( baseport); 	/* clear the data recv interupts */
	rmode = inb( baseport + 2); 	/* clear the interrupt ID reg */
	rmode = inb( baseport + 5); 	/* clear the Line status reg */
	rmode = inb( baseport + 6); 	/* clear the Mode status reg */
	rmode = (unsigned char) 0;
	outb( baseport + 1, rmode); 	/* disable all interrupt types */
	/* turn off interrupt gate and (possibly) modem lines */
	outb( baseport + 4, (asy_flag[ unit ] & HUPCL) ? 0 : (DTR|RTS) ); 
	splx( x ); 			/* reenable interrupts to the system */

	/* set the init flag to 0 so the next open will re-enable interrupts */
	asy_open[ unit ] = 0;
	asy_flag[ unit ] = 0;
	/* ignore ints from this channel */
	sd_array[ unit ].sd_open = 0;
	tp->t_state = 0;
#ifdef ATMERGE
	if (unit == 0)
	    comport = DEVN_COM0;
	else
	    comport = DEVN_COM1;
        devrelse(comport);
#endif /* ATMERGE */
}


/*
** asyread
**      Read from a device
*/
asyread( dev )
int     dev;
{
	register struct tty     *tp;
	int		x;

#ifdef	DEBUG
	debprf("asyread: dev %d\n", dev);
#endif	/* DEBUG */
	tp = &asy_tty[ UNIT(dev) ];	   /* point to correct tty struct  */

	/* execute the common read code for this line discipline */
	( *linesw[ tp->t_line ].l_read )( tp );

}


/*
** asywrite
**      Write to a device
*/
asywrite( dev )
int     dev;
{
	register struct tty     *tp;
	int		x;

#ifdef	DEBUG
	debprf("asywrite: dev %d\n", dev);
#endif	/* DEBUG */
	tp = &asy_tty[ UNIT(dev) ];	   /* point to correct tty struct  */

	/* execute the common write code for this line discipline */
	( *linesw[ tp->t_line ].l_write )( tp );
}


/*
** asyioctl
**      Call the common ioctl code, and
**      modify the state of the device if necessary
*/
asyioctl( dev, cmd, arg, mode )
unsigned int    dev,
		cmd,
		mode;
union ioctl_arg arg;
{
	int	unit;
	int		x;

	unit = UNIT(dev);
#ifdef	DEBUG
	debprf("asyioctl: dev %d\n", dev);
#endif	/* DEBUG */
	switch ( cmd )
	{
#ifdef	DEBUG
		case	0xffff:
			asydebug = !asydebug;
			return;
#endif	/* DEBUG */
		default:
			/*
			** if the ioctl is one in which the parameters of
			** the chip have to be changed, change them
			*/
			if ( ttiocom( &asy_tty[ unit ], cmd, arg, mode ) )
			{
				asyparam( unit );
			}
		}
}


/*
** asyparam
**      Modify the state of the device as indicated in
**      the tty structure
*/
asyparam( unit )
int     unit;
{
	register struct tty     *tp;
	register int	    flags;	/* to ease access to c_flags */
	unsigned int	    baseport; /* to access 16450 base port */
	unsigned char	   rmode;  /* local mode value */
	unsigned int    	x;	/* value for spl */
	int	outb();

#ifdef	DEBUG
	debprf("asyparam: unit %d\n", unit);
#endif	/* DEBUG */
	tp = &asy_tty[ unit ];	   /* point to correct tty struct */
	baseport = ctl[ unit ];

	/* initialize the mode flag */
	rmode = (unsigned char) 0;
	/*
	** if we have never initialized this particular channel
	** before, init it now
	*/
	if ( asy_open[ unit ] == 0 )
	{

		/*
		** Interrupts are turned off to guard against spurious
		** interrupts.
		*/
#ifdef	DEBUG
		debprf("asyparam: unit %d first init\n", unit);
#endif	/* DEBUG */
		x = TTYSPL();
		outb( baseport + 1, rmode); /* disable all device interupts */
		inb( baseport);     /* clear the data recv interupts */
		inb( baseport + 2); /* clear the interrupt ID reg */
		inb( baseport + 5); /* clear the Line status reg */
		inb( baseport + 6); /* clear the Mode status reg */
		inb( baseport + 6); /* clear the Mode status reg */
		inb( baseport + 6); /* clear the Mode status reg */
		inb( baseport + 2); /* clear the Int  status reg */
		inb( baseport + 2); /* clear the Int  status reg */
		outb( baseport + 4, DTR | RTS | INTDRIV);		
			/* enable modem lines & interrupt gate  */
		/* these are necessary after modem enable to unclog interrupts */
		inb( baseport + 6); /* clear the Mode status reg */
		inb( baseport + 2); /* clear the Int  status reg */
		splx( x ); 		/* reenable interrupts to the system */
	}

	asy_flag[unit] = flags = tp->t_cflag;

	/*
	** set up the mode flags to reflect the
	** current state of t_cflags. At this point
	** in time, the only things that you can modify
	** are : 
	** bits per character, parity enable, type of
	** parity, and stop bits.
	** Later: figure out CREAD, and a few others
	*/

	/* initialize the mode flag */
	rmode = (unsigned char) 0;

	switch( flags & CSIZE ) {
		case CS5:
			rmode = DL_5BIT;	break;
		case CS6:
			rmode = DL_6BIT;	break;
		case CS7:
			rmode = DL_7BIT;	break;
		case CS8:
			rmode = DL_8BIT;	break;
	}
#ifdef	DEBUG
	debprf("Siop: unit 0x%x flags 0%o rmode 0%o ", unit, flags, rmode);
#endif	/* DEBUG */
	if ( flags & PARENB )
	{
		rmode |= ENB_PARITY;
#ifdef	DEBUG
		debprf("0%o ", rmode);
#endif	/* DEBUG */
		if ( ! ( flags & PARODD ) )
		{
			rmode |= EVEN_PAR;
		}
#ifdef	DEBUG
		debprf("0%o ", rmode);
#endif	/* DEBUG */
	}

	if ( flags & CSTOPB )
	{
		rmode |= STOPBIT2;
	}

	/*
	** now ( finally ) program the chip with the modes
	** we just set up.
	*/
	outb( baseport + 3, rmode );    /* line control register */
#ifdef	DEBUG
	debprf("asyparam: unit %d mode byte %x\n", unit, rmode);
#endif	/* DEBUG */

	/* if baud rate is zero, drop modem lines and wait for other end to drop,
	 * or fake it if we're not using modem control */
	if ( ( flags & CBAUD ) == B0 )
	{
#ifdef	MODEM
			if (!(asy_flag[unit] & CLOCAL))
				outb( baseport + 4, INTDRIV);
			else
#endif	/* MODEM */
				signal( tp->t_pgrp, SIGHUP );

			return;
	} else {
		/* do these every time, to counteract 'stty 0' */
		outb( baseport + 4, DTR | RTS | INTDRIV);		
		}

	/* set the baud rate */
	setbaud( unit, flags & CBAUD );

}

/*
** asyintr
**      Entry point for all interrupts from all
**      16450 devices. This routine figures out
**      why it was called, and dispatches to
**      the appropriate routine to handle the condition.
*/
asyintr( unit )
unsigned int    unit;
{
	unsigned char   reason;	 /* reason why we are here */
	unsigned int baseport;		/* access to the 16450 */
	int	x;

	baseport = ctl[unit];

	for ( ;; )
	{
		reason = inb( baseport + 2);	/* read from the IIR */


#ifdef	OLD
		if ( !( reason & INT_PEND) )
		{
			switch ( reason & INTR_BITS )
			{
				/* hardware priority order */
				case LS_STAT:	/* special rcv cond */

					asysrintr( unit );
					break;

				case RX_RDY:	/* receive char avail */

					asyrintr( unit );
					break;

				case TX_EMPTY:	/* xmit buffer empty */

					asyxintr( unit );
					break;

				case MODM_STAT:	/* status change */

					asyeintr( unit );
					break;
				}
		}
#else	/* OLD */
		/* recode order for most probable ints in prep for asm-izing */
		if ( !( reason & INT_PEND) ) {
			x = reason & INTR_BITS;
			if (x == TX_EMPTY)
				asyxintr( unit );
			else if (x == RX_RDY)
				asyrintr( unit );
			else if (x == MODM_STAT)
				asyeintr( unit );
			else 
				asysrintr( unit );
		}
#endif	/* OLD */
		else {
#ifdef	DEBUG 
				debput('!');
#endif	/* DEBUG */
				break;
			}
	}
}
	
/*
** asyxintr
**      Transmit buffer empty interrupt handler
*/
asyxintr( unit )
unsigned int    unit;
{
	register struct tty     *tp;
	unsigned int	    baseport, sps;
	sd_t				*sdp;

	ONGUARD;
#ifndef	PSEUDODMA
#ifdef	DEBUG
	debput('X');
#endif	/* DEBUG */
	sysinfo.xmtint++;

	baseport = ctl[ unit ];
	tp = &asy_tty[ unit ];

	if ( tp->t_state & ( TIMEOUT | TTSTOP ) ) {
		UNGUARD;
		return;
		}

/* Recode TTXON/OFF to leave start/stop chars at head of queue, 
 * that way 90% of asyxintr disappears!
 */
	/* if we are supposed to send an xoff or xon, do it now */
	if ( tp->t_state & TTXON )
	{
		tp->t_state &= ~TTXON;
		tp->t_state |= BUSY;
		outb( baseport, CSTART );
	} else if ( tp->t_state & TTXOFF ) {
		tp->t_state &= ~TTXOFF;
		tp->t_state |= BUSY;
		outb( baseport, CSTOP );
	} else {
		/* otherwise, just try to initiate more output */
		/*	if ( tp->t_state & BUSY )	/* more efficient not to test */
		{
			tp->t_state &= ~BUSY;
		}
		UNGUARD;
		asyproc( tp, T_OUTPUT );
		return;
	}
#else	/* PSEUDODMA */
	sdp = &sd_array[unit];
	if ( *(sdp->sd_state) & ( TIMEOUT | TTSTOP ) ) {
		*(sdp->sd_state) &= ~BUSY;		/* so that ttywait doesn't hang */
		UNGUARD;
		return;
		}
/*	if (!(inb(sdp->sd_base + 5) & TXH_EMPTY))
		return;	/* will empty soon */
	if (sdp->sd_ptr >= 0) 
		outb(sdp->sd_base, sdp->sd_queue[(sdp->sd_ptr)--]);
	else {
		/* otherwise, just try to initiate more output */
		/*	if ( sdp->t_state & BUSY )	/* more efficient not to test */
		{
			*(sdp->sd_state) &= ~BUSY;
		}
		UNGUARD;
		asyproc( sdp->sd_tp, T_OUTPUT );
		return;
	}
#endif	/* PSEUDODMA */
	UNGUARD;
}


/*
** asyeintr
**   handle the external status changes to the chip.  These are
**   the Clear To Send status, the Data Terinal Ready status, the 
**   Ring Indicator status, and the Data Carrier Detect status
**	NOTE: asm interface requires that 'unit' be left alone!
*/
asyeintr( unit )
unsigned int    unit;
{
	register struct tty     *tp;
	unsigned int	    baseport;
	unsigned char		mode;
	unsigned char		msr,delta;

	sysinfo.mdmint++;

	baseport = ctl[ unit ];
	tp = &asy_tty[ unit ];
	/* clear the interrupt bit in the chip */
	/* M003	will only see Delta on first inb()! */
	delta = inb( baseport + 6); /* clear the interrupt internally */
	/* M003	now read until it settles */
	msr = inb( baseport + 6); /* clear the interrupt internally */
	msr = inb( baseport + 6); /* clear the interrupt internally */
	mode = msr | (delta&0xf);
#ifdef	DEBUG
	debprf("E%x %x", unit, mode);
#endif	/* DEBUG */
#ifdef	MODEM
	if (mode & DCD) {
	    if (asy_open[unit] & (MODEM_WAIT|MODEM_OPEN)) {
		    tp->t_state |= CARR_ON;
		    /* Don't use WOPEN, so clean up ISOPEN/WOPEN trash in asyopen() */
#ifdef	DEBUG
		    debprf("Sioextern: about to wake up getty...\n");
#endif	/* DEBUG */
		    if ( asy_open[unit] & MODEM_WAIT ) {
			    wakeup((caddr_t) &asy_flag[unit]);
			    }
		    }
	} else if (mode & DELT_DCD) {/* Carrier dropped, send a hangup signal */
	    if (asy_open[unit] & (MODEM_WAIT|MODEM_OPEN)) {	/* M003 */
#ifdef	DEBUG
		debprf("Sioextern: Carrier drop, send SIGHUP\n");
#endif	/* DEBUG */
		tp->t_state &= ~CARR_ON;
		signal( tp->t_pgrp, SIGHUP );
	    }
	} 
#endif	/* MODEM */
#ifdef	CTSCTL
	if (USECTS(asy_uses[unit]) && ((mode & (CTS|DELT_CTS)) == (CTS|DELT_CTS)))
		asyproc(T_OUTPUT, unit);
#endif	/* CTSCTL */
	/* mode = M003 no need for assignment */
	inb( baseport + 6); /* clear the interrupt internally */
	return;
}

/*
** asyrintr
**      Receive character available interrupt handler
**	NOTE: asm interface requires that 'unit' be left alone!
*/
asyrintr( unit )
unsigned int    unit;
{
	register struct tty     *tp;
	unsigned int	    baseport;
	unsigned char	   c, x, stat;

	/*
	stat = inb( baseport + 5 ) & ( FRAME_ERR|OVERRUN_ERR|PARITY_ERR|DATA_RDY );
	*/
	/* while there are characters present...  */
#ifdef	DEBUG
	debprf("R%x %x", unit, stat);
#endif	/* DEBUG */
	sysinfo.rcvint++;

	baseport = ctl[ unit ];
	tp = &asy_tty[ unit ];

	while ( (inb( baseport + 5 ) & 
		( FRAME_ERR | OVERRUN_ERR | PARITY_ERR | DATA_RDY )) ==  DATA_RDY )
	{
		c = inb( baseport );     /* get the character */
#ifdef	DEBUG
		debput(c&0x7f);
#endif	/* DEBUG */
		/*
		** if we aren't open, nobody to give the char to,
		** so chuck it.
		*/
		if ( ! ( tp->t_state & ( ISOPEN | WOPEN ) ) )
			continue /* continue */;

		/*
		** if we are supposed to do xon/xoff protocol,
		** and the char we got is one of those guys,
		** take care of it.
		*/
		if ( tp->t_iflag & IXON )
		{
			unsigned char   ctmp;

			ctmp = c & 0x7F;

			/*
			** if we are stopped, and if the char is a xon, or if
			** we should restart on any char, resume
			*/
			if ( tp->t_state & TTSTOP )
			{
				if ( ctmp == CSTART || tp->t_iflag & IXANY )
					asyproc( tp, T_RESUME );
			}
			else
			{
				if ( ctmp == CSTOP )
					asyproc( tp, T_SUSPEND );
			}
			if ( ctmp == CSTART || ctmp == CSTOP )
				continue /*continue*/;	/* get more chars */
		}

		/* if we are supposed to strip char to 7 bits, do it */
		if ( tp->t_iflag & ISTRIP )
			c &= 0x7F;
		else
			c &= 0xFF;

		/* if we don't have anywhere to put the char, continue */
		if ( tp->t_rbuf.c_ptr == NULL )
		{
			goto comnrxit;	/* Exit at ONE point please M002 */
			/* M002
			return;
			M002 */
		}

		x = TTYSPL();
		/* stuff the char into buffer and signify that we have a char */
		*tp->t_rbuf.c_ptr = c;
		tp->t_rbuf.c_count--;
		( *linesw[ tp->t_line ].l_input )( tp, L_BUF );
		splx(x);
	}
	/*	else if (stat & DATA_RDY)
			inb( baseport );	/* throw it away, should call next routine */
    /* BEGIN M002 Additions -> */
comnrxit:		/* common exit point makes for easier debug & trace */
    /* this is to "recapture" the lost indication within 8250B (B version only)
       UARTs that the Transmit Holding Register became empty during the
       servicing of a higher priority interrupt.  The important bit is IER 1
       which enables THRE interrupts.  Aparently, re-enabling this bit after
       the condition occurs, will save the condition that otherwise is lost
       upon the eoi().  Thus, this should be done right before the exit of
       these higher priority routines so as to catch all occurances. */
    if ((inb(baseport + 5) & TXH_EMPTY)) {
	outb ((baseport + 1),inb (baseport+1));
    }
    return;	/* explicit returns are nice to indicate NO return value */
    /* END M002 */
}


/*
** asysrintr
**      Special receive interrupt handler
**	NOTE: asm interface requires that 'unit' be left alone!
*/
asysrintr( unit )
unsigned int    unit;
{
	unsigned char	   reason;
	unsigned char	   lbuf[ 3 ];	/* local char buffer */
	unsigned char	   c;
	unsigned int	    lcnt;	   /* cnt of chars in lbuf */
	unsigned int	    flg;
	register struct tty     *tp;
	caddr_t		 tptr;
	unsigned int	    baseport, x;

	baseport = ctl[ unit ];
	tp = &asy_tty[ unit ];

	flg = tp->t_iflag;
	reason = inb( baseport + 5 );
#ifdef	DEBUG
	debprf("S%x %x ", unit, reason);
#endif	/* DEBUG */

	/* try to find out why we are here */
	switch ( reason & 0x1f )
	{
		case OVERRUN_ERR:	/* receiver overrun */
		case OVERRUN_ERR | PARITY_ERR:
		case OVERRUN_ERR | PARITY_ERR | FRAME_ERR:
			
			/*
			** if this happens, the best thing that we can
			** do is get out of here as quickly as possible,
			** to avoid another overrun
			*/
			break;

		case PARITY_ERR:	/* parity error */

			c = inb( baseport );	/* get the char */
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
					/* throw the char away */
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
				unsigned char   ctmp;
	
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
						asyproc( tp, T_RESUME );
				}
				else
				{
					if ( ctmp == CSTOP )
						asyproc( tp, T_SUSPEND );
				}
				if ( ctmp == CSTART || ctmp == CSTOP )
					break;
			}
	
			/* if we are supposed to strip char to 7 bits, do it */
			if ( tp->t_iflag & ISTRIP )
				c &= 0x7F;
			else
				c &= 0xFF;

			/* stash the char */
			lbuf[ 0 ] = c;

			/* if we go no place to put the char, get out */
			if ( tp->t_rbuf.c_ptr == NULL )
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
			if ( ! ( tp->t_iflag & IGNBRK ) )
			{
			     if ( tp->t_iflag & BRKINT )
			     {
				 ( *linesw[ tp->t_line ].l_input )( tp, L_BREAK );
			      }
			 }
			break;
		default:

			break;

	}
    /* BEGIN M002 Additions -> */
comnsxit:
    /* this is to "recapture" the lost indication within 8250B (B version only)
       UARTs that the Transmit Holding Register became empty during the
       servicing of a higher priority interrupt.  The important bit is IER 1
       which enables THRE interrupts.  Aparently, re-enabling this bit after
       the condition occurs, will save the condition that otherwise is lost
       upon the eoi().  Thus, this should be done right before the exit of
       these higher priority routines so as to catch all occurances. */
    if ((inb(baseport + 5) & TXH_EMPTY)) {
	outb ((baseport + 1),inb (baseport+1));
    }
    /* END M002 */
}


/*
** asyproc
**      General command routine that initiates action
*/
asyproc( tp, cmd )
register struct tty     *tp;
int		     cmd;
{
	register int    c;      /* char to process */
	extern	  ttrstrt();
	unsigned int    baseport;
	int	     unit, asyoutput();
	sd_t	*sdp;
	extern hz;

	/* get device number, control baseport, and pseudo-dma structure */
	unit = tp - asy_tty;
	baseport = ctl[ unit ];
	sdp = &sd_array[ unit ];

	/* based on cmd, do various things to the device */
	switch ( cmd )
	{
		case T_TIME:	    /* stop sending a break */
#ifdef	DEBUG
			debprf("asyproc: unit %d T_TIME\n", unit);
#endif	/* DEBUG */

			tp->t_state &= ~TIMEOUT;
			state( unit, SET_BREAK, OFF );
			goto start;

		case T_WFLUSH:	  /* output flush */

#ifdef	DEBUG
			debprf("asyproc: unit %d T_WFLUSH\n", unit);
#endif	/* DEBUG */
			tp->t_tbuf.c_size -= tp->t_tbuf.c_count;
			tp->t_tbuf.c_count = 0;
			ONGUARD;
			sdp->sd_ptr = -1;
			UNGUARD;
			/* FALL THROUGH */

		case T_RESUME:	  /* enable output */

#ifdef	DEBUG
			debprf("asyproc: unit %d T_RESUME\n", unit);
#endif	/* DEBUG */
			tp->t_state &= ~TTSTOP;
			goto start;
			/* FALL THROUGH */
		
		case T_OUTPUT:	  /* do some output */
start:
		{
			register struct ccblock *tbuf;
			int		len, orig, sps, endq;

			tbuf = &tp->t_tbuf;
/* Don't need guards, resume only comes in when no int pending (I think) */
			/*
			** if we are stopped, timed-out, or
			** just plain busy, don't do anything
			** PSEUDODMA: don't bother with high-water/low-water marks
			*/
			if ( tp->t_state & ( TIMEOUT | TTSTOP | BUSY ) ) {
#ifdef	DEBUG
				debprf("asyproc: unit %d T_OUTPUT state %o break\n", 
					unit, tp->t_state);
#endif	/* DEBUG */
/*				splx(sps); /* */
				break;
				}

			if ( tbuf->c_ptr == 0 || tbuf->c_count <= 0 )
			{
				if ( tbuf->c_ptr )
					tbuf->c_ptr -= tbuf->c_size - tbuf->c_count;
				if ( !( CPRES & ( *linesw[ tp->t_line ].l_output )( tp ) ) )
				{
#ifdef	DEBUG
					debprf("asyproc: unit %d T_OUTPUT no chars left\n", unit);
#endif	/* DEBUG */
/*					splx(sps); /* */
					break;
				}
			}
#ifdef	CTSCTL
			if (USECTS(asy_uses[unit])) {
				if ((inb(baseport + 6) & CTS) || 
						(!(inb(baseport + 5) & TXH_EMPTY))) {
/*					splx(sps); /* */
					break;	
					}
			} else {

				/*
				if (!(inb(baseport + 5) & TXH_EMPTY)) {
#ifdef	DEBUG
					debprf("asyproc: unit %d T_OUTPUT chip not ready\n", unit);
#endif	/* DEBUG */
					if (!flag[unit]) {
						timeout(asyoutput, unit, 1);
						flag[unit] = 1;
						}
					break;
					}
				*/
				}
#endif	/* CTSCTL */
			/* if one of above conditions occurs, wait for its interrupt */
			ONGUARD;
			tp->t_state |= BUSY;
#ifndef	PSEUDODMA
			outb( baseport , *tbuf->c_ptr++ );
			tbuf->c_count--;
			UNGUARD;
#else	/* PSEUDODMA */
			/* BUSY means we're doing stuff out of the queue */
			/* or, that we've poked CSTART/CSTOP ourselves */
			/* we can get here after TTSTOP/TIMEOUT finishes */
			/* first, grab next char for later output */
			if (sdp->sd_ptr < 0) {
				c =  *tbuf->c_ptr++;
				tbuf->c_count--;
			} else
				c = sdp->sd_queue[ sdp->sd_ptr-- ];
			/* figure out how many chars we can tack on end of queue */
			if (sdp->sd_ptr >= 0) 
				len = QLEN - (sdp->sd_ptr + 1);
			else
				len = QLEN;
			len = min( tbuf->c_count, len - 1);  /* leave room for START/STOP */
#ifdef	DEBUG
			debprf("\rT_OUTPUT: dma %d chars \n", len);
#endif	/* DEBUG */
			tbuf->c_count -= len;
			orig = sdp->sd_ptr;
			sdp->sd_ptr += len;
			endq = sdp->sd_ptr;
			UNGUARD;
			/* copy existing chars to beginning of queue (this is rare) */
			while(orig >= 0)
				sdp->sd_queue[ endq-- ] = sdp->sd_queue[ orig-- ];
			/* copy new chars to end of queue */
			while(endq >= 0)
				sdp->sd_queue[ endq-- ] = *tbuf->c_ptr++;
			/* start chain */
			outb(baseport, c);
			/* I think none of the above needs to be guarded */
#endif	/* PSEUDODMA */
			break;
		}

		case T_SUSPEND:	 /* block on output */
			
#ifdef	DEBUG
			debprf("asyproc: unit %d T_SUSPEND\n", unit);
#endif	/* DEBUG */
			tp->t_state |= TTSTOP;
			break;

		case T_BLOCK:	   /* block on input */

#ifdef	DEBUG
			debprf("asyproc: unit %d T_BLOCK\n", unit);
#endif	/* DEBUG */
			/*
			** either we send a xoff right now, or
			** we make sure that the next char sent
			** is an xoff.
			*/
			tp->t_state &= ~TTXON;
			tp->t_state |= TBLOCK;
			if ( tp->t_state & BUSY ) {
#ifdef	PSEUDODMA
				/* Put CSTOP at head of queue */
				ONGUARD;
				sdp->sd_queue[ ++(sdp->sd_ptr) ] = CSTOP;
				UNGUARD;
#else	/* PSEUDODMA */
				tp->t_state |= TTXOFF;
#endif	/* PSEUDODMA */
			} else
			{
				tp->t_state |= BUSY;
				outb( baseport, CSTOP );
			}

			break;

		case T_RFLUSH:	  /* flush input */
			
#ifdef	DEBUG
			debprf("asyproc: unit %d T_RFLUSH\n", unit);
#endif	/* DEBUG */
			/*
			** if we are not blocked on input
			** nothing to do
			*/
			if ( ! ( tp->t_state & TBLOCK ) )
				break;

			/* FALL THROUGH */

		case T_UNBLOCK:	 /* enable input */

#ifdef	DEBUG
			debprf("asyproc: unit %d T_UNBLOCK\n", unit);
#endif	/* DEBUG */
			tp->t_state &= ~( TTXOFF | TBLOCK );

			/*
			** Again, if we are not busy, send an xon
			** right now. Otherwise, make sure that the next
			** char out will be an xon
			*/
			if ( tp->t_state & BUSY ) {
#ifdef	PSEUDODMA
				/* Put CSTART at head of queue */
				ONGUARD;
				sdp->sd_queue[ ++(sdp->sd_ptr) ] = CSTART;
				UNGUARD;
#else	/* PSEUDODMA */
				tp->t_state |= TTXON;
#endif	/* PSEUDODMA */
			} else
			{	/* This code might be a problem, if it restarts dma */
				tp->t_state |= BUSY;
				outb( baseport, CSTART );
			}

			break;

		case T_BREAK:	   /* send a break */
			
#ifdef	DEBUG
			debprf("asyproc: unit %d T_BREAK\n", unit);
#endif	/* DEBUG */
			state( unit, SET_BREAK, ON );
			tp->t_state |= TIMEOUT;
			timeout( ttrstrt, tp, hz / 4 );
			break;

		case T_SWTCH:	   /* shell layer switch */
			
#ifdef	DEBUG
			debprf("asyproc: unit %d T_SWTCH\n", unit);
#endif	/* DEBUG */
			/*
			** nothing for us to do
			*/
			break;

		case T_PARM:	    /* update parameters */

#ifdef	DEBUG
			debprf("asyproc: unit %d T_PARM\n", unit);
#endif	/* DEBUG */
			asyparam( unit );
			break;

		default:

			break;
	}
}

	asyoutput(val)
	caddr_t val;	/* 32-bits v.s. 16 bits */
	{
		int	unit;

		unit = (int) val;
		asyproc(asy_tty[unit], T_OUTPUT);
	}

/*
** setbaud
**      Set the baud rate for a particular device
*/
setbaud( unit, select )
int	     unit;
unsigned int    select;
{
	unsigned int    baseport;	/* to ease access to port */
	unsigned int    rate;
	unsigned int    x;
	unsigned char   mode;
	
	baseport = ctl[unit];

	mode = inb(baseport + 3);
	rate = asy_speeds[ select ];
	x = TTYSPL();					/* 7 because of timeout problems */
	outb( baseport + 3, mode | DLAB);
	outb( baseport, (char)rate );
	outb( baseport + 1, (char)( rate >> 8 ) );
	outb( baseport + 3, mode);
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
	unsigned int baseport;
	struct tty      *tp;
	unsigned int    s;

	tp = &asy_tty[ unit ];
	baseport = ctl[ unit ];

	/* first, set s to current line control register value */
	s = inb( baseport + 3);

	/*
	** now, output the state back to the chip,
	** modifying the state to reflect the condition
	*/
	outb( baseport + 3, on ? (s | cond) : (s & ~cond) );
}

static 
setints( unit )
{
/* M000 - Begin additions:=>						*/
	if(vector[si_ints[unit]]!=0)
	    {
#ifdef	DEBUG
	    debprf("setints:  vector %x\n", vector[si_ints[unit]] );
#endif	/* DEBUG */
	    outb(vector[si_ints[unit]],0x80);	/* enable all interrupts  */
#ifdef	DEBUG
	    /*
	    debprf("setints:  value of vector %x\n", inb(vector[si_ints[unit]]) );
	    */
#endif	/* DEBUG */
	    }
/* M000 -  END additions   						*/
	if ((asy_open[ unit ] & (MODEM_OPEN | MODEM_WAIT | CTS_OPEN)))
		outb( ctl[unit] + 1, ENB_ALL_INT );
	else
		outb( ctl[unit] + 1, ENB_TX_INT | ENB_RX_INT | ENB_LS_INT);		
}

/* timer to compensate for lost interrupts */
sitimer(val) 
caddr_t val;
{
	int unit;
	extern hz;

	unit = ( int ) val;
	if (asy_open[ unit ]) {
		asyintr( unit );
		timeout(sitimer, (caddr_t) unit, hz);
		}
}

/*
 * initialize pseudo-dma structure for asy channel x
 * Later: multiple channels on one vector, chained through linked list
 */

mksd(unit)
int	unit;
{
#ifdef	PSEUDODMA
	struct asydma	*sdp;
	int intnum = si_ints[ unit ];

	sdp = &sd_array[unit];
	sdp->sd_unit = unit;
	sdp->sd_open = 1;
	sdp->sd_base = ctl[unit];
	sdp->sd_tp = &asy_tty[unit];
	sdp->sd_state = &asy_tty[unit].t_state;
	sdp->sd_ptr = -1;			/* queue is empty */
	sdp->sd_active = &si_active[ intnum ];  /* common data area */
	/* add to chain at head */
#ifdef	JUNK
	si_chain[ intnum ];
#endif
        asm("   cli");
	sdp->sd_sdp = si_chain[ intnum ];
	si_chain[ intnum ] = sdp;
        asm("   sti");
#endif	/* PSEUDODMA */
}

/* 
 * remove unit from chain. 
 * called from asyclose
 */

rmsd(unit)
int	unit;
{
#ifdef	PSEUDODMA
	register struct asydma	*sdp, **sdpc;

	sdp = &sd_array[unit];
	/* find sdp in chain (IT MUST BE IN IT!) */
	sdpc = &si_chain[ si_ints[ unit ] ];
        asm("   cli");
        if (*sdpc != sdp) {
            do
                sdpc = &((*sdpc)->sd_sdp);
            while( *sdpc != sdp );
            }
        *sdpc = sdp->sd_sdp;
        asm("   sti");
#endif  /* PSEUDODMA */
}

#ifdef ATMERGE
sioreset(devn)
{
}
#endif /* ATMERGE */
