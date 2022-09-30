/*      Copyright (c) 1985 AT&T */
/*        All Rights Reserved   */

/*      THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T     */
/*      The copyright notice above does not evidence any        */
/*      actual or intended publication of such source code.     */

/* @(#)text.c	1.18 */
#include "sys/param.h"
#include "sys/types.h"
#include "sys/sysmacros.h"
#include "sys/systm.h"
#include "sys/map.h"
#include "sys/dir.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/proc.h"
#include "sys/text.h"
#include "sys/inode.h"
#include "sys/buf.h"
#include "sys/seg.h"
#include "sys/mmu.h"
#include "sys/var.h"
#include "sys/sysinfo.h"
#include "sys/fp.h"

/*
 * Swap out process p.
 * The ff flag causes its core to be freed--
 * it may be off when called to create an image for a
 * child process in newproc.
 * Os is the old size  of the process,
 * and is supplied during core expansion swaps.
 */

xswap(p, ff, os)
register struct proc *p;
{
	register a;
	register int *ip;

	p->p_flag |= SLOCK;
	if (ff && (p == u.u_procp))
		printf("xswap %x\n",p);

	/*
	** if the person we want to swap currently
	** owns the floating point unit, save his
	** fp registers
	*/
	if (p == fp_proc)
		fpsave(FPNOCHECK);

	if (os == 0)
		os = p->p_size;
	a = swapalloc(ctod(p->p_size), 1);

	if (p->p_textp)
		xccdec(p->p_textp, p);
	swap(p, a, p->p_addr, os, B_WRITE);
	p->p_flag &= ~SLOAD;
	if (ff) {
		mfree(coremap, os, p->p_addr);
		zapprocslots(p);
		if (runin) {	/* questionable: in pdp but not vax */
			runin = 0;
			setrun(&proc[0]);
		}
	}
	p->p_addr = a;
	p->p_flag &= ~SLOCK;
	p->p_time = 0;
	if (runout) {
		runout = 0;
		wakeup((caddr_t)&runout);
	}
}

/*
 * relinquish use of the shared text segment
 * of a process.
 */
xfree()
{
	register struct text *xp;
	register struct inode *ip;
	register struct proc *p = u.u_procp;
	unsigned int x;

	if ((xp = p->p_textp) == NULL)
		return;
	xlock(xp);
	xp->x_flag &= ~XLOCK;
	x = spl7();
	p->p_textp = NULL;
	splx( x );
	p->p_flag &= ~STEXT;
	ip = xp->x_iptr;
	if (--xp->x_count==0 && (ip->i_mode&ISVTX)==0) {
		xp->x_iptr = NULL;
		if (xp->x_daddr)
			mfree(swapmap, ctod(xp->x_size), xp->x_daddr);
		mfree(coremap, xp->x_size, xp->x_caddr);
		ip->i_flag &= ~ITEXT;
		if (ip->i_flag&ILOCK)
			ip->i_count--;
		else
			iput(ip);
	} else
		xccdec(xp, p);
}

/*
 * Attach to a shared text segment.
 * If there is no shared text, just return 2.
 * If there is, hook up to it:
 * if it is not currently being used, it has to be read
 * in from the inode (ip); the written bit is set to force it
 * to be written out as appropriate & return 1.
 * If it is being used, but is not currently in core,
 * a swap has to be done to get it back & return 0.
 * Otherwise, return -1.
 * NOTE:
 * 	The reason that there are spl7()s around the setting
 *	of textp, is that a pointer load on the 286 is not an
 *	atomic operation, e.g. between a segment register load
 *	and an offset load there could be an interrupt. In clock(),
 *	textp is examined, and, if non-zero, is used. If the textp
 *	is not fully loaded, the kernel would panic. Therefore,
 *	we disallow clock interrupts for the duration of the setting
 *	of textp.
 */
xalloc(ip)
register struct inode *ip;
{
	register struct text *xp;
	register ts;
	register struct text *xp1;
	register struct proc *p = u.u_procp;
	unsigned int x;

	if (u.u_exdata.ux_tsize == 0)
		/* non-shared text */
		return(2);
	xp1 = NULL;
loop:
	for (xp = &text[0]; xp < (struct text *)v.ve_text; xp++) {
		if (xp->x_iptr == NULL) {
			if (xp1 == NULL)
				xp1 = xp;
			continue;
		}
		if (xp->x_iptr == ip) {
			xlock(xp);
			xp->x_count++;
			x = spl7();
			p->p_textp = xp;
			splx( x );
			if (xp->x_ccount == 0)
				xexpand(xp);
			else {
				xp->x_ccount++;
				p->p_flag |= STEXT;
				if (xp->x_caddr == 0)
					panic("lost text");
			}
			/* shared: memory is loaded */
			return(0);
		}
	}
	if ((xp=xp1) == NULL) {
		printf("out of text\n");
		syserr.textovf++;
		if (xumount(NODEV))
			goto loop;
		psignal(p, SIGKILL);
		/* out of luck */
		return(-1);
	}
	xp->x_flag = XLOAD|XWRIT|XLOCK;
	xp->x_count = 1;
	xp->x_ccount = 0;
	xp->x_iptr = ip;
	ip->i_flag |= ITEXT;
	ip->i_count++;
	ts = btoc(u.u_exdata.ux_tsize);
	xp->x_size = ts;
	xp->x_daddr = 0;	/* defer swap alloc til later */
	x = spl7();
	p->p_textp = xp;
	splx( x );
	xexpand(xp);
	/* shared: memory needs to be loaded */
	return(1);
}

/*
 * Assure core for text segment
 * Text must be locked to keep someone else from
 * freeing it in the meantime.
 * x_ccount must be 0.
 */
xexpand(xp)
register struct text *xp;
{
	register struct proc *p = u.u_procp;

	if ((xp->x_caddr = malloc(coremap, xp->x_size)) != NULL) {
		if ((xp->x_flag&XLOAD)==0)
			swap(p, xp->x_daddr, xp->x_caddr, xp->x_size, B_READ);
		xp->x_ccount++;
		p->p_flag |= STEXT;
		adjusttext(p, xp->x_caddr);
		xunlock(xp);
		return;
	}
	xque(p, SXTXT, 0);
}

/*
 * Lock and unlock a text segment from swapping
 */
xlock(xp)
register struct text *xp;
{

	while(xp->x_flag&XLOCK) {
		xp->x_flag |= XWANT;
		sleep((caddr_t)xp, PSWP);
	}
	xp->x_flag |= XLOCK;
}

xunlock(xp)
register struct text *xp;
{

	if (xp->x_flag&XWANT)
		wakeup((caddr_t)xp);
	xp->x_flag &= ~(XLOCK|XWANT);
}

/*
 * Decrement the in-core usage count of a shared text segment.
 * When it drops to zero, free the core space.
 */
xccdec(xp, p)
register struct text *xp;
register struct proc *p;
{

	if (xp==NULL || xp->x_ccount==0)
		return;
	xlock(xp);
	p->p_flag &= ~STEXT;
	if (--xp->x_ccount==0) {
		if (xp->x_flag&XWRIT) {
			if (xp->x_daddr == 0)
			{
				xp->x_daddr = swapalloc(ctod(xp->x_size), 1);
			}
			xp->x_flag &= ~XWRIT;
			swap(p, xp->x_daddr, xp->x_caddr, xp->x_size, B_WRITE);
		}
		mfree(coremap, xp->x_size, xp->x_caddr);
		xp->x_flag &= ~XLOAD;
	}
	xunlock(xp);
}

/*
 * free the swap image of all unused saved-text text segments
 * which are from device dev (used by umount system call).
 */
xumount(dev)
register dev_t dev;
{
	register struct inode *ip;
	register struct text *xp;
	register count = 0;

	for (xp = &text[0]; xp < (struct text *)v.ve_text; xp++) {
		if ((ip = xp->x_iptr) == NULL)
			continue;
		if (dev != NODEV && dev != ip->i_dev)
			continue;
		if (xuntext(xp))
			count++;
	}
	return(count);
}

/*
 * remove a shared text segment from the text table, if possible.
 */
xrele(ip)
register struct inode *ip;
{
	register struct text *xp;

	if ((ip->i_flag&ITEXT) == 0)
		return;
	for (xp = &text[0]; xp < (struct text *)v.ve_text; xp++)
		if (ip==xp->x_iptr)
			xuntext(xp);
}

/*
 * remove text image from the text table.
 * the use count must be zero.
 */
xuntext(xp)
register struct text *xp;
{
	register struct inode *ip;

	xlock(xp);
	if (xp->x_count) {
		xunlock(xp);
		return(0);
	}
	ip = xp->x_iptr;
	xp->x_flag &= ~XLOCK;
	xp->x_iptr = NULL;
	if (xp->x_daddr)
	{
		mfree(swapmap, ctod(xp->x_size), xp->x_daddr);
	}
	ip->i_flag &= ~ITEXT;
	if (ip->i_flag&ILOCK)
		ip->i_count--;
	else
		iput(ip);
	return(1);
}

struct {
	int	x_w;
	int	x_c;
	struct proc *x_h, *x_t;
} xsp;

/*
 * xsched handles swapping out processes that need to grow.
 * It also handles writing out old delayed write system buffers
 */
xsched()
{
	register struct proc *p;
	register arg;
	int s;

	bcopy("xsched", u.u_comm, 7);
	xqupd();
	for (;;) {
		if (xsp.x_h == NULL)
			bdflush();
		s = splhi();
		if ((p = xsp.x_h) == NULL) {
			xsp.x_w++;
			sleep(&xsp.x_w, PSWP);
			splx(s);
			continue;
		}
		splx(s);
		xsp.x_c++;
		arg = p->p_arg;
		switch(p->p_stat) {
		case SXBRK:
			xswap(p, 1, arg);
			break;
		case SXSTK:
			xswap(p, 1, arg);
			break;
		case SXTXT:
			xswap(p, 1, 0);
			xunlock(p->p_textp);
			break;
		}
		xsp.x_h = p->p_link;
		p->p_link = 0;
		p->p_arg = 0;
		if (p == xsp.x_t)
			xsp.x_t = 0;
		setrun(p);
	}
}

xque(p, st, arg)
register struct proc *p;
{
	/*
	** save the floating point registers
	*/
	fpsave( FPCHECK );

	p->p_stat = st;
	p->p_arg = arg;
	p->p_link = 0;
	if (xsp.x_t)
		xsp.x_t->p_link = p;
	else
		xsp.x_h = p;
	xsp.x_t = p;
	if (xsp.x_w) {
		xsp.x_w = 0;
		wakeup(&xsp.x_w);
	}
	swtch();
}

/*
 * periodically wake up xsched proc to call bdflush
 */
xqupd()
{
	if (xsp.x_w) {
		xsp.x_w = 0;
		wakeup((caddr_t)&xsp.x_w);
	}
	timeout(xqupd, (caddr_t)0, HZ/2);
}

/*
 * allocate swap blocks, freeing and sleeping as necessary
 */
swapalloc(size, sflg)
{
	register addr;

	for (;;) {
		if (addr = malloc(swapmap, size))
			return(addr);
		if (swapclu()) {
			printf("\nWARNING: swap space running out\n");
			printf("  needed %d blocks\n", size);
			continue;
		}
		printf("\nDANGER: out of swap space\n");
		printf("  needed %d blocks\n", size);
		if (sflg) {
			mapwant(swapmap)++;
			sleep((caddr_t)swapmap, PSWP);
		} else
			return(0);
	}
}

/*
 * clean up swap used by text
 */
swapclu()
{
	register struct text *xp;
	register ans = 0;

	for (xp = text; xp < (struct text *)v.ve_text; xp++) {
		if (xp->x_iptr == NULL)
			continue;
		if (xp->x_flag&XLOCK)
			continue;
		if (xp->x_daddr == 0)
			continue;
		if (xp->x_count) {
			if (xp->x_ccount) {
				mfree(swapmap, ctod(xp->x_size), xp->x_daddr);
				xp->x_flag |= XWRIT;
				xp->x_daddr = 0;
				ans++;
			}
		} else {
			xuntext(xp);
			ans++;
		}
	}
	return(ans);
}
