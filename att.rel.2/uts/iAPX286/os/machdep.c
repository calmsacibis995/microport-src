/*      Copyright (c) 1985 AT&T */
/*        All Rights Reserved   */

/*      THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T     */
/*      The copyright notice above does not evidence any        */
/*      actual or intended publication of such source code.     */

/* @(#)machdep.c	1.48 */
#include "sys/param.h"
#include "sys/types.h"
#include "sys/sysmacros.h"
#include "sys/systm.h"
#include "sys/map.h"
#include "sys/dir.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/proc.h"
#include "sys/seg.h"
#include "sys/mmu.h"
#include "sys/reg.h"
#include "sys/psl.h"
#include "sys/utsname.h"
#include "sys/8259.h"
#include "sys/file.h"
#include "sys/fp.h"
#include "sys/ppi.h"

#define	STATESZ	(16 * sizeof(int) + sizeof(u.u_fpstate.edata))

/*
 * Machine-dependent startup code
 */
startup()
{

	printf("\nUNIX/%s: %s%s\n", utsname.release, utsname.sysname, utsname.version);
	memsize();
	printf("real mem  = %ld\n", (long)((long)physmem*ctob(1)));
	printf("avail mem = %ld\n", (long)((long)maxmem*ctob(1)));
	mfree(swapmap, nswap, 1);
	swplo--;
}

clkset(oldtime)
time_t	oldtime;
{
	time = oldtime;
}

clkreld(on)
{
	if (on)
		clockon();
	else
		clockoff();
}

/*
 * Send an interrupt to process
 */
sendsig(hdlr, signo, arg)
int	(*hdlr)();
{
	register *usp, *regs;
	register	fpoffset;	/* offset on user stack of
						saved fpstate */

	regs = u.u_ar0;
	if((unsigned)(regs[SP] - STATESZ) > (unsigned)(regs[SP]))

		/* Can't expand user stack to setup for user handler. */
		return(1);
	usp = *(int **)(&regs[SP]);
	grow((unsigned)(regs[SP] - STATESZ));

	/* Save state on user's stack so sigcode can restore it later.
		See sigcode (os/sigcode.c) for further details. */
	/* Saved state includes:
		u.u_fpstate (present only if user has valid fp state)
		sp, cs, ip, ax, cx, dx, bx,
		fp offset or 0, (0 if user does not have valid fp state)
		bp, si, di, ds, es, flags, cs, and ip.  */
	fpsave(FPCHECK);
	if(u.u_fpvalid) {
		usp = (int *)((char *)usp - sizeof(u.u_fpstate.edata));
		fpoffset = loword(usp);
		copyout(u.u_fpstate.edata, usp, sizeof(u.u_fpstate.edata));
		u.u_fpvalid = 0;
	} else
		fpoffset = 0;
	if(u.u_intflg) {
		regs[FLGS] |= PS_T;
		u.u_intflg = 0;
	}
	*--usp = regs[SP];
	*--usp = regs[CS];
	*--usp = regs[IP];
	*--usp = regs[AX];
	*--usp = regs[CX];
	*--usp = regs[DX];
	*--usp = regs[BX];
	*--usp = fpoffset;
	*--usp = regs[BP];
	*--usp = regs[SI];
	*--usp = regs[DI];
	*--usp = regs[DS];
	*--usp = regs[ES];
	*--usp = regs[FLGS];
	*--usp = regs[CS];
	*--usp = regs[IP];

	/* Push the args for the user handler. */
	*--usp = arg;
	*--usp = signo;

	/* Push ret addr for user handler.  (Code at <CODE1_SEL>:0 in crt0.s
		will do an lcall to sigcode to reset registers and return to
		user routine.) */
	if(u.u_model & U_MOD_MTEXT)
		*--usp = (CODE1_SEL << 3) | LDT_TI | USER_RPL;
	*--usp = 0;

	/* Fix up the kernel stack for transfer to user handler. */
	regs[FLGS] &= ~PS_T;
	*(int **)(&regs[SP]) = usp;
	*(int (**)())(&regs[IP]) = hdlr;
	return(0);
}

/*
 * copy count bytes from from to to.
 */
bcopy( from, to, count )
caddr_t		from,
		to;
unsigned	count;
{
	asm( "	mov	14(%bp),%cx" );
	asm( "	lds	6(%bp),%si" );
	asm( "	les	10(%bp),%di" );
	asm( "	rep" );
	asm( "	smovb" );
}

/*
 * Zero sections of memory.
 */
bzero(addr, size)
caddr_t		addr;
unsigned int	size;
{
	asm( "	mov	10(%bp),%cx" );
	asm( "	les	6(%bp),%di" );
	asm( "	xor	%ax,%ax" );
	asm( "	rep" );
	asm( "	sstob" );
}

/*
 * create a duplicate copy of a process
 */
procdup(p)
register struct proc *p;
{
	register ushort a1, a2, n;

	n = p->p_size;
	if ((a2 = malloc(coremap, n)) == NULL)
		return(NULL);

	a1 = p->p_addr;
	p->p_addr = a2;
	while(n--)
		copyseg(a1++, a2++);
	adjustldt(p, p->p_addr);
	return(1);
}

/*
** If the word at addr is readable by the user
** then change its selector's permissions to mode.
*/
chgprot(addr, mode)
caddr_t addr;
unsigned mode;
{
	if (useracc(addr, NBPW, FREAD)) {
		u.u_ldtadv[ston(hiword(addr))].sd_access = mode;
	}
}

chksize(text, data, stack, ldt)
unsigned text, data, stack, ldt;
{
	unsigned long	n;

	n = (unsigned long)USIZE +
		(unsigned long)text +
		(unsigned long)data +
		(unsigned long)stack +
		(unsigned long)ldt;

	if (n > MAXMEM || n > (unsigned long)maxmem) {
		u.u_error = ENOMEM;
		return(-1);
	}
	return(0);
}

#define	MB	( 1 << 20 )	/* define a megabyte to make the code cleaner */

/*
** memsize
**	size memory in click ( 512 bytes ) chunks, and fill
**	the variables maxmem and physmem
** NOTE:
**	If there is at least one 544 present, we jump around
**	the top 128k of each Mb, because the 544 is a 20 bit
**	peripheral, and will respond at every multiple
**	of 20 bits ( yech! )
** NOTE:
**	This code assumes that memory boards are configured
**	on megabyte boundaries
*/
memsize()
{
	paddr_t		paddr;
	paddr_t		pmem;
	paddr_t		physaddr();
	extern void	etext();
	unsigned	i;
	unsigned	clicklimit = 0,	/* base address (in clicks) of 544   */
			tmplimit,
			prevmax = 0;	/* previous value of maxmem	     */
	unsigned long	sioscan(),
			bdaddr;		/* base address (real mode) of 544   */

	/*
	** enable ram address decoding for
	** addresses above 7/8 of a Mb
	** NOTE: The value written could be anything;
	** the port just needs a word write
	*/
	out( PPIPB, 0x1111 );

	/*
	** get the physical address of etext
	*/
	paddr = physaddr( etext );

	/*
	** round up to a click boundary ( just to be nice )
	*/
	paddr += ( NBPC - 1 );
	paddr &= ~( NBPC - 1 );

	/*
	** check to see if there are any 544s out there.
	** if there are, remember base address in clicks
	*/
	if ( ( bdaddr = sioscan() ) != 0L )
	{
		tmplimit = clicklimit = btoc( bdaddr );
	}

	/*
	** step through memory, a click ( 512 bytes ) at a time,
	** writing two values to the first location in the
	** click. if the values come back the same, it must
	** be ram, so increment maxmem
	*/
	for ( i = btoc( paddr ); i < MAXPHYS; i++ )
	{
		/*
		** if there is a 544 out there ( i.e. clicklimit is set )
		** check to see if we have sized up to it. if we have,
		** jump over the memory that it occupies, and continue.
		*/
		if ( clicklimit )
		{
			if ( i == clicklimit )
			{
				paddr = ( paddr + MB ) & ~( MB - 1 );
				/*
				** minus one because when we continue, 
				** the i++ happens
				*/
				i = btoc( paddr ) - 1;

				/*
				** set clicklimit to the next 20 bit boundary
				*/
				clicklimit = btoc( paddr ) + tmplimit;

				/*
				** set maxmem to the largest contiguous
				** section of memory seen so far
				*/
				maxmem = max( maxmem, prevmax );
				prevmax = 0;

				continue;
			}
		}

		spword( paddr, 0xFADE );	/* write first value	*/

		if ( fpword( paddr ) != 0xFADE )
		{
			if ( clicklimit )
			{
				/*
				** we think we have reached the
				** end of ram. Skip up to the next 
				** megabyte, and keep looking.
				*/
				paddr = ( paddr + MB ) & ~( MB - 1 );
				/*
				** minus one because when we continue, 
				** the i++ happens
				*/
				i = btoc( paddr ) - 1;
	
				/*
				** set clicklimit to the next 20 bit boundary
				*/
				clicklimit = btoc( paddr ) + tmplimit;
	
				/*
				** set maxmem to the largest contiguous
				** section of memory seen so far
				*/
				maxmem = max( maxmem, prevmax );
				prevmax = 0;
	
				continue;
			}

			break;
		}

		spword( paddr, 0xBABE );	/* write second value	*/

		if ( fpword( paddr ) != 0xBABE )
		{
			break;
		}

		/*
		** well, we got through the above code, so
		** it must be ram out there.
		** free up the associated page in the map and
		** increment physical address to next click,
		** and add one to mem size
		*/
		mfree( coremap, 1, i );
		pmem = paddr;
		paddr += NBPC;
		++prevmax;
	}

	/*
	** set maxmem to the largest contiguous
	** section of memory seen so far
	*/
	maxmem = max( maxmem, prevmax );

	/*
	** at this point, we know how much usable ram we have.
	** so, set physmem to usable ram + kernel ram
	*/
	physmem = btoc( pmem );
}

/*
** physaddr
**	translate a virtual address into a physical address
*/
paddr_t
physaddr( virt )
char	*virt;
{
	/*
	** union to access the parts of an address
	*/
	union
	{
		struct
		{
			ushort	off;	/* offset part			*/
			ushort	seg;	/* segment part			*/
		} sep;

		char	*full;		/* as a seg-offset pointer	*/
	} laddr;
	paddr_t		paddr;		/* physical address		*/
	struct seg_desc	*sd;

	laddr.full = virt;
	
	/*
	** if the virtual address wants to point to the
	** LDT, use the processes LDT. Otherwise use the GDT
	*/
	if ( ( laddr.sep.seg & SEL_TI ) == LDT_TI )
	{
		/*
		** get a pointer to the LDT resident seg desc
		*/
		sd = &u.u_ldtadv[ ston( laddr.sep.seg ) ];
	}
	else
	{
		/*
		** get a pointer to the GDT resident seg desc
		*/
		sd = &gdt[ ston( laddr.sep.seg ) ];
	}

	paddr = sd->sd_lowbase;	      	/* bits 0-15 of phys base of seg*/
	paddr |= (long)sd->sd_hibase << 16;	/* bits 16-23 of phys base*/
	paddr += laddr.sep.off;		/* physical address of virt     */

	return( paddr );
}

/*
** initpic
**	Initialize the Programmable Interrupt Controller
*/
initpic()
{
	outb( PICMSTAT, MICW1 );
	outb( PICMASTER,MICW2 );
	outb( PICMASTER,MICW3 );
	outb( PICMASTER,MICW4 );

	outb( PICSSTAT, SICW1 );
	outb( PICSLAVE, SICW2 );
	outb( PICSLAVE, SICW3 );
	outb( PICSLAVE, SICW4 );
}

/*
** eoi
**	send a specific End-Of-Interrupt command to the
**	master PIC if the interrupt came from the master; 
**	otherwise, send an End-Of-Interrupt to the slave, then
**	to the master.
*/
eoi( vector )
unsigned int	vector;
{
	/*
	** is vector out of range of the slave and the master?
	*/
	if ( vector > ( SBASE + 7 ) )
		return;

	if ( vector >= SBASE )
	{
		outb( PICSSTAT, EOI );
		outb( PICMSTAT, EOI | SPECIFIC_EOI | 7 );
		return;
	}

	outb( PICMSTAT, EOI | SPECIFIC_EOI | ( vector - MBASE ) );
}


addupc( pc, prof, ticks )
char		*pc;
unsigned int	ticks;
struct
{
	short		*pr_base;	/* buffer base		*/
	unsigned int	pr_size;	/* buffer size		*/
	char 		*pr_off;	/* pc offset		*/
	unsigned int	pr_scale;	/* pc scaling		*/
} *prof;
{
	unsigned int	tip,		/* temp ip		*/
			tcs,		/* temp cs		*/
			r1,		/* tmp reg		*/
			r2;		/* tmp reg		*/
	unsigned long	ltmp;		/* tmp long		*/
	short		*bucketp;	/* bucket pointer	*/

	/*
	** first, calculate the distance between pc and pr_off
	*/
	tip = loword( pc ) - loword( prof->pr_off );
	tcs = ( hiword( pc ) >> 3 ) - ( hiword( prof->pr_off ) >> 3 ) -
		( loword( prof->pr_off ) > loword( pc ) );

	/*
	** second, calculate the bucket subscript as follows:
	**	(((scale * distance) >> 16) + 1) >> 1
	** scale * distance is a 48 bit quantity.
	**	r1 will contain the middle 16 bits,
	**	r2 will contain the high 16 bits,
	**	the low 16 bits are tossed since the 16 bit shift will throw
	**		them away anyway.
	** if r2 is non-zero, the request is out of bounds because the counters
	**	are restricted to residing in a single segment
	** note that the final shift by 1 is delayed until after the bounds
	**	check is completed because pr_size is in bytes not shorts
	*/
	ltmp = tip * (unsigned long)prof->pr_scale;
	r1 = hiword( ltmp );
	ltmp = tcs * (unsigned long)prof->pr_scale;
	r1 += loword( ltmp ) + 1;
	r2 = hiword( ltmp ) + ( r1 < loword( ltmp ) );

	/*
	** check for out of bounds
	*/
	if ( r2 || ( r1 >= prof->pr_size ) )
		return;

	/*
	** find out where to increment, test the location for
	** addressability, and increment the location if ok.
	*/
	bucketp = prof->pr_base + ( r1 >> 1 );
	if ( useracc( bucketp, 2, 0 ) )
	{
		suword( bucketp, fuword( bucketp ) + ticks );
	}
	else
	{
		prof->pr_scale = 0;
	}
}

/*
** validate
**	routine called before the iret in trap.s to validate
**	the users return cs:ip, and ss:sp.
**	Also, validates if the user has set the NT flag in the
**	flags word ( THIS CAN BE DONE -- EXTREMELY DANGEROUS ).
**	If the user has set the NT flag, he is killed immediately,
**	and he has no chance to catch this signal. This routine 
**	never returns under this condition.
**	Returns 1 if addresses are bad, and user is trying 
**	to catch them; returns 0 if addresses ok; doesn't
**	return at all if addresses are bad, and user
**	isn't catching them.
*/
validate( base )
int	*base;
{
	unsigned long	addr;
	int		ret = 0;

	/* 
	** validate if NT flag is set
	*/
	if ( base[ FLGS ] & PS_NT )
	{
		exit( core() ? ( 0200 + SIGBUS ) : SIGBUS );
	}
		
	/*
	** validate cs:ip
	*/
	addr = ( (unsigned long)base[ CS ] << 16 ) |
					(unsigned int)base[ IP ];
	/*
	** if its not readable or it is writable, it is not code
	*/
	if ( useracc( addr, 1, 1 ) == 0 || useracc( addr, 1, 0 ) )
	{
		/*
		** the users return cs:ip is bad, so send ourselves
		** a signal. if issig says we don't have a signal, 
		** the user must be ignoring a segmentation violation,
		** so kill him off. If the user is catching the signal,
		** give him a chance to recover.
		*/
		psignal( u.u_procp, SIGSEGV );
		if ( ! issig() )
		{
			exit( core() ? ( 0200 + SIGSEGV ) : SIGSEGV );
		}
		psig( 0 );
		ret = 1;
	}

	addr = ( (unsigned long)base[ SS ] << 16 ) |
					(unsigned int)base[ SP ];
	/*
	** if not writable, it is bad
	*/
	if ( useracc( addr, 1, 0 ) == 0 )
	{
		psignal( u.u_procp, SIGSEGV );
		if ( ! issig() )
		{
			exit( core() ? ( 0200 + SIGSEGV ) : SIGSEGV );
		}
		psig( 0 );
		ret = 1;
	}
	
	return( ret );
}

/*
** mdboot
** This is a dummy routine on the 286/310.  It is intended on other
** architectures to return to the monitor and/or reboot the UNIX System.
*/
mdboot(fcn, mdep)
{
	spl7();
	printf( "Press reset to reboot\n" );
	while( 1 )
		;
}
