static char *uportid = "@(#)machdep.c	Microport Rev Id 1.3.8  10/19/86";
/* (#)machdep.c	1.48 */
/*		Copyright 1985 Microport Systems
 *		All Rights Reserved.
 *
 *	Modification History:
 * 	General upgrade to the IBM AT. Involves:
 *		1) Removing 544 references (sioscan, memsizing).
 *		2) Changed eoi to use SSLAVE_HERE
 *		3) Teaching memsize to skip around 0x9FE00, to 1 meg
 *		4) Having memsize to handle the first hole it comes across
 *		5) Add reboot.
 *  		6) Changed memsize() to be table-driven, and added separate
 *  		7) init-time functions to allocate (getmem) and mfree() 
 *		8) Changed initpic to set the slave to lowest priority. 
 *		   This is in conjunction with fixed sio dropping of characters.
 *		9) mdboot now distinguishes between a halt and a reboot.
 *		10) Moved the allocation of swapmap(nswap) and swplo-- out of 
 *			here and into main.c. The reason for this is because 
 *			these two variables now can get picked up via wnopen 
 *			for the IBM AT. Ifdef'ed on IBMAT.
 */

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
#ifdef IBMAT
#include "sys/uadmin.h"
#endif IBMAT
#ifdef ATMERGE
#include "sys/realmode.h"

extern	struct	sw_data	usw_data;
#endif ATMERGE

#define	STATESZ	(16 * sizeof(int) + sizeof(u.u_fpstate.edata))

#ifdef IBMAT
long foundmem;
#endif IBMAT

/*
 * Machine-dependent startup code
 */
startup()
{
	printf("\nSYSTEM V/AT release %s: %s\n", 
		utsname.release, utsname.sysname);
	memsize();
	printf("total mem  = %ldK\n", foundmem/1024);
#ifdef	IBMAT
#else	IBMAT
	mfree(swapmap, nswap, 1);
	swplo--;
#endif	IBMAT
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
#ifdef	LCCSIO
	asm("	pushf ");
	asm("	cld ");
#endif	LCCSIO
	asm( "	mov	14(%bp),%cx" );
	asm( "	lds	6(%bp),%si" );
	asm( "	les	10(%bp),%di" );
	asm( "	rep" );
	asm( "	smovb" );
#ifdef	LCCSIO
	asm("	popf");
#endif	LCCSIO
}

/*
 * copy count words from from to to.
 */
wcopy( from, to, count )
caddr_t		from,
		to;
unsigned	count;
{
#ifdef	LCCSIO
	asm("	pushf ");
	asm("	cld ");
#endif	LCCSIO
	asm( "	mov	14(%bp),%cx" );
	asm( "	lds	6(%bp),%si" );
	asm( "	les	10(%bp),%di" );
	asm( "	rep" );
	asm( "	smov" );
#ifdef	LCCSIO
	asm("	popf");
#endif	LCCSIO
}

/*
 * Zero sections of memory.
 */
bzero(addr, size)
caddr_t		addr;
unsigned int	size;
{
#ifdef	LCCSIO
	asm("	pushf ");
	asm("	cld ");
#endif	LCCSIO
	asm( "	mov	10(%bp),%cx" );
	asm( "	les	6(%bp),%di" );
	asm( "	xor	%ax,%ax" );
	asm( "	rep" );
	asm( "	sstob" );
#ifdef	LCCSIO
	asm("	popf");
#endif	LCCSIO
}

/*
 * Zero sections of memory.
 */
wzero(addr, size)
caddr_t		addr;
unsigned int	size;
{
#ifdef	LCCSIO
	asm("	pushf ");
	asm("	cld ");
#endif	LCCSIO
	asm( "	mov	10(%bp),%cx" );
	asm( "	les	6(%bp),%di" );
	asm( "	xor	%ax,%ax" );
	asm( "	rep" );
	asm( "	ssto" );
#ifdef	LCCSIO
	asm("	popf");
#endif	LCCSIO
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

#ifdef IBMAT
/* Following is memory table stuff */
/* configured to act as before, i.e. it expects contiguous memory */
#define	NRANGES 2

struct memtab {
	unsigned long	Base, End;
};

struct memtab maybemem[NRANGES] = {		/* check these areas for memory */
	0x000000, 0x0a0000,
	0x100000, 0x1000000, 
};
struct memtab ismem[NRANGES];			/* record areas with memory */
#endif IBMAT
	

/*
** memsize
**	size memory in click ( 512 bytes ) chunks, and fill
**	the variables maxmem and physmem
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
	unsigned long	bdaddr;		/* base address (real mode) of 544   */
	int		indx;

#ifdef ATMERGE
	extern unsigned int fulpage, lulpage, fuepage;

	paddr = maybemem[0].Base = ((paddr_t) fulpage) << 9;
	bdaddr = ((paddr_t) lulpage) << 9;
	if (bdaddr < maybemem[0].End)
	    maybemem[0].End = bdaddr;
	bdaddr = ((paddr_t) fuepage) << 9;
	if (bdaddr > maybemem[1].Base)
	    maybemem[1].Base = bdaddr;

	/* start foundmem such that it will end up as total installed
	   memory. */
	if (bdaddr > 0x100000L)
	    foundmem = bdaddr - 0x100000L;	/* normal case */
	else
	    /* can't run merge - kernel in low mem */
	    foundmem = (physaddr (etext) + NBPC - 1) & ~(NBPC - 1);

#ifdef ATPROBE
  /* this is temporary debugging code - avoids overwriting probe code */
	maybemem[1].End = 0xd00000L;
  /* end of temporary debugging code */
#endif /* ATPROBE */

#else /* -ATMERGE */
	/*
	/*
	** get the physical address of etext
	*/
	paddr = physaddr( etext );

	/*
	** round up to a click boundary ( just to be nice )
	*/
	paddr += ( NBPC - 1 );
	paddr &= ~( NBPC - 1 );
	foundmem = paddr;
#endif /* ATMERGE */

	indx = 0;
	if ((paddr < maybemem[0].Base) || (paddr >= maybemem[0].End))
		panic("Memsize: First ram range is incorrect\n");

	ismem[ indx ].Base = ismem[ indx ].End = 0L;
	physmem = prevmax = maxmem = 0;
	/*
	** step through memory, a click ( 512 bytes ) at a time,
	** writing a value to the first location in the
	** click. if the value comes back the same, it must
	** be ram, so increment maxmem
	** Try to find each
	*/
	for ( ; ; )
	{
		/* There is memory up to end of this range, so mark end 
		 * and advance to next range.
		 */

		if (paddr >= maybemem[indx].End) {
	nextrange:
			if ( ismem[ indx ].Base == 0L) {
						 /* this range never started, */
				ismem[ indx ].Base = paddr;	
						 /* thus its totally empty */
			}

			ismem[ indx ].End = paddr;
			indx++;
			if (indx == NRANGES)
				break;
			paddr = maybemem[ indx ].Base;
			ismem[ indx ].Base = ismem[ indx ].End = 0L;
			maxmem = max( maxmem, prevmax);
			prevmax = 0;
		}

		spword( paddr, 0xBABE );	/* write value	*/
		if ( fpword( paddr ) != 0xBABE ) {
				goto nextrange;
		}
		spword( paddr, ~0xBABE );	/* write value	*/
		if ( fpword( paddr ) != ~0xBABE ) {
				goto nextrange;
		}

		/*
		** well, we got through the above code, so
		** it must be ram out there.
		** If this is the first hit in this range,
		** save this address to mark beginning of known
		** ram in this range.
		** and add one to mem size
		*/
		if (ismem[ indx ].Base == 0L )
			ismem[ indx ].Base = paddr;
		pmem = paddr;
		paddr += NBPC;
		++prevmax;
		/*
		** set maxmem to the largest contiguous
		** section of memory seen so far, and physmem
		** to highest physical address seen so far.
		*/
		maxmem = max( maxmem, prevmax );
		physmem = btoc( paddr );
		foundmem += NBPC;
	}
}

/* allocate a piece of physical memory that (optionally) doesn't straddle */
/* a page boundary */
/* This plucks memory directly out of the ismem array; may only be used */
/* during kernel startup by init routines, between "startup" and "freemem" */
/* in main():main.c */
/* Allows lots of small hunks like clists and suchlike to be allocated, */
/* also can optionally avoid DMA page straddles */
#ifdef ATMERGE
/* For ATMERGE, must allocate top down! */
getmem(hunk, paddr, pageflg)
unsigned long hunk, *paddr;	/* length to alloc, repository for pointer */
int pageflg;				/* throw away page straddles if one */
{
	int	ret_val = 1;
	static indx = NRANGES - 1;
	long start;

	/* if this range is too small, skip to next one */
	do {
		if ((ismem[ indx ].End - ismem[ indx ].Base) < hunk) {
			ret_val = 0;
			if (indx-- == 0)
				panic("Getmem: Ran out of memory\n");
			continue;
			}
		/* if region straddles a DMA page, throw away problem area */
		if (pageflg)
		{
			start = ismem[indx].End - hunk;
			if (((start & 0x0FFFFL) + hunk) > 0x10000L)
			{
				ismem[ indx ].End &= 0xffff0000;
				ret_val = 0; /* non-contiguous with last call */
				continue;
			}
		}
		break;
	} while(1);
	/* found some memory! */
	ismem[ indx ].End -= hunk;
	*paddr = ismem[ indx ].End;

	/* return 1 if memory is contiguous to most recent allocation. 
	 * 0 if skip 
	 */
	return ret_val;
}

#else /* ! ATMERGE */
getmem(hunk, paddr, pageflg)
unsigned long hunk, *paddr;	/* length to alloc, repository for pointer */
int pageflg;				/* throw away page straddles if one */
{
	int	ret_val = 1;
	static indx = 0;

	/* if this range is too small, skip to next one */
	do {
		if ((ismem[ indx ].End - ismem[ indx ].Base) < hunk) {
			ret_val = 0;
			if (++indx == NRANGES)
				panic("Getmem: Ran out of memory\n");
			continue;
			}
		/* if region straddles a DMA page, throw away problem area */
		if (pageflg && 
			(((ismem[ indx ].Base & 0x0FFFFL) + hunk) > 0x10000L)) {
			ismem[ indx ].Base += 0x10000L;
			ismem[ indx ].Base &= 0xffff0000;
			ret_val = 0;	/* non-contiguous with last call */
			continue;
			}
		break;
	} while(1);
	/* found some memory! */
	*paddr = ismem[ indx ].Base;
	ismem[ indx ].Base += hunk;

	/* return 1 if memory is contiguous to most recent allocation. 
	 * 0 if skip 
	 */
	return ret_val;
}
#endif /* ATMERGE */

/*
 * Freemem gives all usable memory to the standard UNIX malloc/mfree routines 
 * after the system init routines have allocated their stuff
 */
freemem() {
	int	i, j;
	long first, last, usrmem = 0L;

	maxmem = 0;
	for(i = 0; i < NRANGES; i++)  {
#ifdef LCCFIX
		ismem[i].End &= ~(NBPC-1);
#endif /* LCCFIX */
		first = ismem[ i ].Base;  last = ismem[ i ].End;
		if (first & (NBPC-1)) 
			first = (first + NBPC) & ~(NBPC-1);
#ifdef ATMERGE
		/* Do not free click 0 !! */
		if (first == 0)
			first = NBPC;
#endif /* ATMERGE */
		if (last - first > 0) {
			mfree(coremap, (int) btoc(last - first), (int) btoc(first)); 
			usrmem += last - first;
			/* largest contig: */
			maxmem = max(maxmem, btoc(last - first)); 
			}
		}
	printf("avail mem = %ldK\n", usrmem/1024);
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

#ifndef LCCFIX
	/* Can't do this with MERGE, because DOS and DOS programs will assume
	   that, for example, if they have not EOI'd the keyboard, serial
	   interrupts will not happen. */
#ifdef IBMAT
	outb( PICMSTAT, ROTATE|SPECIFIC_EOI|2 );
#endif IBMAT
#endif /* !LCCFIX */

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
#ifndef LCCFIX

	/*
	** is vector out of range of the slave and the master?
	*/
	if ( vector > ( SBASE + 7 ) )
		return;

	if ( vector >= SBASE )
	{
		outb( PICSSTAT, EOI );
		outb( PICMSTAT, EOI | SPECIFIC_EOI | SSLAVE_HERE );
		return;
	}

	outb( PICMSTAT, EOI | SPECIFIC_EOI | ( vector - MBASE ) );
#endif /* LCCFIX */
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
** The timer is needed because some AT clones have screwy hardware, and need
** some breathing space in order to allow their bus to completely reset.
**
*/
mdboot(fcn, mdep)
int fcn, mdep;
{
	int x;
	unsigned long timer;
	extern ualock, startsd;

	x = spl7();
#ifdef ATMERGE
	/* clear the shutdown byte in the cmos */
	outb(0x70, 0x8f);
	outb(0x71, 0);
#endif /* ATMERGE */
	printf("The system has stopped");
	startsd = 0;
	switch(fcn) {
		case AD_HALT:
			splx(x);
			printf(", and can be rebooted\n(via control-alt-del");
			printf(" or powering off and on)");
			for(;;);
			break;
		case AD_BOOT:
		case AD_IBOOT:
		default:
			splx(x);
			do {
				timer = 0x1FFFF;	/* 1 sec at 8 Mhz */
				outb(0x64,0xFE);	/* resets the AT */
				while(timer--)
					;
			} while(1);
			break;
	}
}

#ifdef IBMAT
/* 
 * Mapin - 
 * create a kernel address for given physical address using given descriptor
 */

long
mapin(addr, selector)
long	addr;
int		selector;
{
	extern struct seg_desc gdt[];

	gdt[selector].sd_lowbase = addr & 0xFFFF;
	gdt[selector].sd_hibase =  (addr>>16) & 0xFF;
	gdt[selector].sd_limit = 0xFFFF;
	gdt[selector].sd_access = ACC_KDATA | DSC_PRESENT;
	return (long) gstokv( ( long ) selector);
}
#endif IBMAT
