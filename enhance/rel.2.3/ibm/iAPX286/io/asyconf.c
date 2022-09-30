static char *uportid = "@(#)siconf.c	Microport Rev Id 1.3.3  6/18/86";
/*
** @(#)siconf.c	1.10
**      IBM Serial/Parallel  Adapter driver
** Written for the AT clone machines 9/15/85
** Torn apart into separate config file 2-12-86
** M000 - uport!mark: Added vector/board status register per channel
** 			Doubled to 4 the number of interrupt handlers
** M001 - uport!mark: Default to COM1 & COM2
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
#include "sys/asycnt.h"		/* M000 make SIO_CNT defined in header file */

int	asy_chans = SIO_CNT;

struct   tty	asy_tty [ SIO_CNT ];
int				asy_open[ SIO_CNT ];
int				asy_flag[ SIO_CNT ];/* Stty flags for this line */

#define CHA	0
#define CHB	1
#define CHC	2
#define CHD	3

/* The ctl  array provides a minor device to port number mapping */
unsigned int    ctl[ SIO_CNT ] =	/* 16450 control ports */
{
	0x3f8,			/* M001 default to COM1, COM2 */
	0x2f8,			
	0,/*  zero indicates no serial device there  */
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0
};

/* The intnum array provides a channel to interrupt vector mapping */
unsigned int  intnum[ SIO_CNT ] = 	/* 16450 interrupt vectors */
{
	4,			/*  zero indicates no serial device there  */
	3,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0
};

char sd_array[64 * SIO_CNT];

extern int	asydebug;

/* M000 - Begin additions:=>						*/
/* The vector array is a port to read for the interrupt vector on
 *  multi-port cards and write to for enabling interrupts 
 *  ZERO if no vector port to read
 */

unsigned int vector[ SIO_CNT ] = 	/* only true if necc */
{
	0,		/*  zero indicates not neccessary to read vector*/
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0
};
/* M000 -  END additions   						*/


#undef	DEBUG

#undef	SIASM		/* M000 restore handler configurable interrupts */
#ifndef	SIASM
/*
** asyintr0
**	Entry point for RS232 port 0 interrupt
**
*/
asyintr0( vec )
unsigned int  vec;
{
#ifdef	DEBUG
	debprf("I0");
#endif	/* DEBUG */
/* M000 - Begin additions:=>						*/
	if(vector[4]!=0)
	    {
	    inb(vector[4]);
	    }

#ifdef MP386
	if (asy_open[CHA]) {
		asyintr( CHA );
		}
#else /* !MP386 */
/* M000 -  END additions   						*/
	_asyintr0();
#endif /* !MP386 */

#ifndef MP386
#ifndef	PICFIX1
	/*
	** must turn off all interrupts until the common interrupt
	** exit code because of another 16450 interrupt occurring
	** between INT_PEND not set, and the eoi to the 8259 PIC
	*/
	asm( "  cli" );
	eoi( vec );
#endif /* ! PICFIX1 */ 
#endif /* !MP386 */

}

/* asyintr1
	Entry point for RS232 port 1 interrupt
**
*/
asyintr1( vec )
unsigned int vec;
{
#ifdef	DEBUG
	debprf("I1");
#endif	/* DEBUG */
/* M000 - Begin additions:=>						*/
	if(vector[3]!=0)
	    {
	    inb(vector[3]);
	    }

#ifdef MP386
	if (asy_open[CHB]) {
		asyintr( CHB );
		}
#else /* MP386 */
/* M000 -  END additions   						*/
	_asyintr1();
#endif /* MP386 */

#ifndef MP386
#ifndef	PICFIX1
	/*
	** must turn off all interrupts until the common interrupt
	** exit code because of another 16450 interrupt occurring
	** between INT_PEND not set, and the eoi to the 8259 PIC
	*/
	asm( "  cli" );
	eoi( vec );
#endif /* ! PICFIX1 */ 
#endif /* MP386 */

}

/* asyintr2
	Entry point for RS232 port 2 interrupt
**
*/
asyintr2( vec )
unsigned int vec;
{
#ifdef	DEBUG
	debprf("I2");
#endif	/* DEBUG */
/* M000 - Begin additions:=>						*/
	if(vector[5]!=0)
	    {
	    inb(vector[5]);
	    }

#ifdef MP386
	if (asy_open[CHC]) {
		asyintr( CHC );
		}
#else /* MP386 */
/* M000 -  END additions   						*/
	_asyintr2();
#endif /* MP386 */

#ifndef MP386
#ifndef	PICFIX1
	/*
	** must turn off all interrupts until the common interrupt
	** exit code because of another 16450 interrupt occurring
	** between INT_PEND not set, and the eoi to the 8259 PIC
	*/
	asm( "  cli" );
	eoi( vec );
#endif /* ! PICFIX1 */ 
#endif /* MP386 */

}

/* asyintr3
	Entry point for RS232 port 3 interrupt
**
*/
asyintr3( vec )
unsigned int vec;
{
#ifdef	DEBUG
	debprf("I3");
#endif	/* DEBUG */
/* M000 - Begin additions:=>						*/
	if(vector[7]!=0)
	    {
	    inb(vector[7]);
	    }

#ifdef MP386
	if (asy_open[CHD]) {
		asyintr( CHD );
		}
#else /* MP386 */
/* M000 -  END additions   						*/
	_asyintr3();
#endif /* MP386 */

#ifndef MP386
#ifndef	PICFIX1
	/*
	** must turn off all interrupts until the common interrupt
	** exit code because of another 16450 interrupt occurring
	** between INT_PEND not set, and the eoi to the 8259 PIC
	*/
	asm( "  cli" );
	eoi( vec );
#endif /* ! PICFIX1 */ 
#endif /* MP386 */

}
#endif	/* SIASM */

