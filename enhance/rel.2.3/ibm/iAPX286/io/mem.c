static char *uportid = "@(#)mem.c	Microport Rev Id 1.3.3  6/18/86";
/*      Copyright (c) 1985 AT&T */
/*        All Rights Reserved   */

/*      THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T     */
/*      The copyright notice above does not evidence any        */
/*      actual or intended publication of such source code.     */

/* @(#)mem.c	1.10 */
/*
 *	Memory special file
 *	minor device 0 is physical memory
 *	minor device 1 is kernel memory
 *	minor device 2 is EOF/NULL
 *
 *  ioctl() is I/O operation interface for user programs. LCN 1-15-86
 *
 *	M001:	Larry!uport	Allow access to top of ROM.
 *	M002:	mike!uport	(rel.138B3)
 *		Added various ioctl's for console driver support
 *	M003:	uport!dwight	Sat Jan 17 22:30:40 PST 1987
 *		Added reboot ioctl.
 *	M004:	uport!rex	Thu Jan 22 1987
 *		Added ioctl commands for login table management.
 *	M005:	uport!rex	Sun Feb  1 16:03:48 PST 1987
 *		Added check that the correct versions of "init" and "getty"
 *		are installed, if not, something terrible should happen.
 *	M006:	mike!uport	Wed Feb 18 15:22:03 PST 1987
 *		Changed outb2, outb21 to use new asm subr's
 *		Cleaned up read/write byte stuff for a little easier reading.
 *	M007:	uport!rexqThu Apr  9 20:34:28 PST 1987
 *		Added ioctl command for returning system information
 *		defined in the structure sys_info in sys/io_op.h.
 */

#include "sys/param.h"
#include "sys/types.h"
#include "sys/sysmacros.h"
#include "sys/dir.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/buf.h"
#include "sys/systm.h"
#include "sys/mmu.h"
#include "sys/seg.h"
#include "sys/io_op.h"
#include "sys/ioctl.h"

/* M001 */
/* definition of top of ROMBIOS address space in phys. memory */
# define max(a,b) 		(a<b ? b : a)
#define ROMTOP 0xfffff
extern	physmem;

int	*nofault;	/* pointer to nofault save area	*/
#define LOGTABSIZ	32	/* M004: big enough for an upgrade  */
short	logintab[LOGTABSIZ];	/* M004: table of getty pid's       */
int	lldb = 0;		/* M005: print some info	    */
static	char	gettychk = 0;	/* M005: 1 = checking getty version */

mmread( dev )
{
	register unsigned	n;
	paddr_t			physaddr();


	switch ( dev )
	{
		case 0:
		{
			struct	seg_desc	*mmp;
			unsigned long		a;
			char			*addr;

		    /* M001 */
			if ( btoc( u.u_offset ) > max(ROMTOP,physmem) )
			{
				u.u_error = ENXIO;
				break;
			}
			mmp = gdt + MM_SEL;
			a = u.u_offset;
			mmp->sd_hibase = lobyte( hiword( a ) );
			mmp->sd_lowbase = loword( a );
			mmp->sd_limit = u.u_count - 1;
			mmp->sd_access = ACC_KDATA;
			addr = (char *)gstokv( MM_SEL );
			if ( copyout( addr, u.u_base, u.u_count ) )
			{
				u.u_error = ENXIO;
			}
			break;
		}

		case 1:
		{
			label_t	fltsav;		/* nofault save area	*/
			int	*oldnofault;	/* old nofault value	*/

			/*
			** the user supplied us with both addresses,
			** so this could fault. So, we implement nofault
			** so it can't hurt us
			*/
			if ( setjmp( fltsav ) )
			{
				nofault = oldnofault;
			}
			else
			{
				oldnofault = nofault;
				nofault = fltsav;
				if ( copyout( u.u_offset, u.u_base,u.u_count ) )
				{
					u.u_error = ENXIO;
				}
			}
			nofault = oldnofault;
			break;
		}

		case 2:
			return;
	}
	u.u_offset += u.u_count;
	u.u_base += u.u_count;
	u.u_count = 0;
}


mmwrite( dev )
{
	register unsigned	n;
	register		c;
	paddr_t			physaddr();


	switch ( dev )
	{
		case 0:
		{
			struct	seg_desc	*mmp;
			unsigned long		a;
			char			*addr;

			if ( btoc( u.u_offset ) > physmem )
			{
				u.u_error = ENXIO;
				break;
			}
			mmp = gdt + MM_SEL;
			a = u.u_offset;
			mmp->sd_hibase = lobyte( hiword( a ) );
			mmp->sd_lowbase = loword( a );
			mmp->sd_limit = u.u_count - 1;
			mmp->sd_access = ACC_KDATA;
			addr = (char *)gstokv( MM_SEL );
			if ( copyin( u.u_base, addr, u.u_count ) )
			{
				u.u_error = ENXIO;
			}
			break;
		}

		case 1:
		{
			label_t	fltsav;		/* nofault save area	*/
			int	*oldnofault;	/* old nofault value	*/

			/*
			** the user supplied us with both addresses,
			** so this could fault. So, we implement nofault
			** so it can't hurt us
			*/
			if ( setjmp( fltsav ) )
			{
				nofault = oldnofault;
			}
			else
			{
				oldnofault = nofault;
				nofault = fltsav;
				if ( copyin( u.u_base, u.u_offset, u.u_count ) )
				{
					u.u_error = ENXIO;
				}
			}
			nofault = oldnofault;
		}
			/* fall through */
		case 2:
			break;
	}
	u.u_offset += u.u_count;
	u.u_base += u.u_count;
	u.u_count = 0;
}

#undef	DEBUG

#ifdef	DEBUGMT
int lastcmd, lastport, lastbyte;
#endif

mmioctl( dev, cmd, arg, mode )
unsigned int dev, cmd, mode;
union ioctl_arg arg;
{
	union s_arg			/* M007 */
	{
	    io_op_t	iop;
	    struct sys_info sis;
	} a;
	long timer;
	extern	daddr_t	cprlmv;		/* M004: patchable login max	*/
	extern	short	logintab[];	/* M004: table of login pid's	*/
	extern	char	initchk;	/* M004: cf/lomem.c		*/
	extern	char	gettychk;	/* M005 */
	int read = 0;			/* M006 */

	extern	int	bdevcnt;	/* M007 */
	extern	int	cdevcnt;	/* M007 */
	extern	dev_t	dumpdev;	/* M007 */
	extern	long	foundmem;	/* M007 */
#ifdef	DEBUG
	printf("mmioctl: sparg = %lx, cmd = %x\n", arg.sparg, cmd);
#endif	DEBUG
	switch(cmd) {			/* M007 begin */
	    case IOCIOP_RB:
	    case IOCIOP_RW:
	    case IOCIOP_WB:
	    case IOCIOP_WW:
	    case IOCIOP_WB2:
	    case IOCIOP_WB21:
	    case IOCIOP_RWB2:
	    case IOCIOP_RWB21:
	    case IOCIOP_RWBM2:
		    if (copyin(arg.sparg, &a, sizeof(a.iop)) == -1) {
			u.u_error = EFAULT;
			return;
		    }
	    }				/* M007 end */
					/* Start M006 and M007 */
/* just to make things a little cleaner */
#define	PORT	a.iop.io_port
#define	PORT2	a.iop.io_port2
#define	BYTE	a.iop.io_byte
#define	BYTE2	a.iop.io_byte2
#define	WORD	a.iop.io_word
					/* End M006 and M007 */
#ifdef	DEBUG
	printf("mmioctl: port = %x, ", PORT);
	if ((cmd == IOCIOP_WB) || (cmd == IOCIOP_WW)) {
	    printf("byte = %x, word = %x\n", BYTE, WORD);
	}
#endif	DEBUG

	/* We don't care about the device number */
	switch(cmd) {
/* Start M006 */
	    case IOCIOP_RB:	BYTE = inb (PORT); read = 1;	break;
	    case IOCIOP_RW:	WORD = in  (PORT); read = 1;	break;
	    case IOCIOP_WB:	outb   (PORT, BYTE);		break;
	    case IOCIOP_WW:	out    (PORT, WORD);		break;
	    case IOCIOP_WB2:	outb2  (PORT, BYTE, BYTE2);	break;
	    case IOCIOP_WB21:	outb21 (PORT, BYTE, BYTE2);	break;
	    case IOCIOP_RWB2:	(void) inb (PORT2);
				outb2  (PORT, BYTE, BYTE2);	break;
	    case IOCIOP_RWB21:	(void) inb (PORT2);
				outb21 (PORT, BYTE, BYTE2);	break;
	    case IOCIOP_RWBM2:	timer = 100000;
				while ((inb (PORT2) & WORD) == 0)
				    if (--timer < 0)
					break;
				outb (PORT, BYTE);
				outb (PORT, BYTE2);		break;
/* End M006 */
/* Start M003 */
	    case IOCIOP_REBOOT:	
				kerndebug();
				break;
/* End M003 */
/* Start M007 */
	    case IOCIOP_SYSINFO:	
			if (copyin(arg.sparg, &a,
					sizeof(struct sys_info)) == -1)
			{
				u.u_error = ENXIO;
				break;
			}
			a.sis.bdevcnt = bdevcnt;
			a.sis.cdevcnt = cdevcnt;
			a.sis.rootdev = rootdev;
			a.sis.pipedev = pipedev;
			a.sis.dumpdev = dumpdev;
			a.sis.swapdev = swapdev;
			a.sis.swplo = swplo;
			a.sis.nswap = nswap;
			a.sis.foundmem = foundmem;
			if ( copyout( &a, arg.sparg,
					sizeof(struct sys_info) ) )
				u.u_error = ENXIO;
			return;
/* End M007 */
	    case IOCIOP_LOGADD:	{	/* Start M004 */
			short	i = 0;
			int	pid;
			if ( copyin( arg.iparg, &pid, sizeof(int) ) ) {
				u.u_error = ENXIO;
				break;
			}
			if (gettychk)	/*	 M005 */
				if (pid == 0) {
					if (lldb) printf("A3");
					gettychk = 0;	/* "getty" check ok */
					break;
				} else
					if (lldb) printf("A0");
					/* 	 M005 */
			for (; i < cprlmv && i < LOGTABSIZ; ++i)
				if (0 == logintab[i]) {
					logintab[i] = (short) pid;
					pid = 0;
					break;
				}
			if ( copyout( &pid, arg.iparg, sizeof(int) ) )
				u.u_error = ENXIO;
			return;
		}
	    case IOCIOP_LOGDEL:	{
			short	i = 0;
			int	pid;
			if ( copyin( arg.iparg, &pid, sizeof(int) ) ) {
				u.u_error = ENXIO;
				break;
			}
			if (initchk) {
				if (lldb) printf("D1");
				initchk = 0;	/* for newproc() */
				break;
			}
			if (pid == 0) {		/* M005 start */
				if (lldb) printf("D2");
				gettychk = 1;	/* for getty check */
				break;
			}
			if (gettychk) {
				if (lldb) printf("D4");
				pid = 0;	/* getty check failed */
			} else			/* M005 end */
				for (; i < cprlmv && i < LOGTABSIZ; ++i)
					if ((short) pid == logintab[i]) {
						logintab[i] = (short) (pid = 0);
						break;
					}
			if ( copyout( &pid, arg.iparg, sizeof(int) ) )
				u.u_error = ENXIO;
			return;
		}			/* End M004 */
	    default: u.u_error = ENXIO; break;
	}

	if (read) {
#ifdef	DEBUG
	    printf("byte = %x, word = %x\n", BYTE, WORD);
#endif	DEBUG
	    if (copyout(&a, arg.sparg, sizeof(a.iop)) == -1) {
		u.u_error = EFAULT;
	    }
	}
#ifdef	DEBUGMT
	lastcmd = cmd;
	lastport = PORT;
	lastbyte = BYTE;
#endif
}
