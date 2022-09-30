/*      Copyright (c) 1985 AT&T */
/*        All Rights Reserved   */

/*      THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T     */
/*      The copyright notice above does not evidence any        */
/*      actual or intended publication of such source code.     */

/* @(#)main.c	1.26 */
#include "sys/param.h"
#include "sys/types.h"
#include "sys/sysmacros.h"
#include "sys/systm.h"
#include "sys/dir.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/filsys.h"
#include "sys/mount.h"
#include "sys/proc.h"
#include "sys/inode.h"
#include "sys/seg.h"
#include "sys/mmu.h"
#include "sys/conf.h"
#include "sys/buf.h"
#include "sys/iobuf.h"
#include "sys/tty.h"
#include "sys/var.h"
#include "sys/clock.h"
#include "sys/utsname.h"

/*
 * Initialization code.
 * Called from cold start routine as
 * soon as a stack and segmentation
 * have been established.
 * Functions:
 *	clear and free user core
 *	turn on clock
 *	hand craft 0th process
 *	call all initialization routines
 *	fork - process 0 to schedule
 *	     - process 1 execute bootstrap
 *
 * loop at low address in user mode -- /etc/init
 *	cannot be executed.
 */

struct inode *rootdir;
int	maxmem, physmem;
char	**bufstart;
struct buf *sbuf;

long
main()
{

	register int (**initptr)();
	extern int (*dev_init[])();
	extern int (*init_tbl[])();
	extern icode[], szicode;
	extern struct user p0u;
	paddr_t physaddr();

	startup();

	printf( "UNIX System V Release %s %s Version %s\n",
		utsname.release, utsname.machine, utsname.version );
	printf( "%s\nCopyright (c) 1985 AT&T\nAll Rights Reserved\n",
		utsname.nodename );

	/*
	 * set up system process
	 */

	proc[0].p_stat = SRUN;
	proc[0].p_flag |= SLOAD|SSYS;
	proc[0].p_nice = NZERO;
	/*
	** The following assignment to p_addr only works if the
	** proc0 u-struct is on a click boundary ( which it is )
	*/
	proc[0].p_addr = btoc( physaddr( &p0u ) );
	proc[0].p_size = USIZE + 1;
	proc[0].p_lsize = 1;
	u.u_procp = &proc[0];
	u.u_ldtadv = (struct seg_desc *)gstokv(&pslot[0].pa_ldt_ad - gdt);
	u.u_cmask = CMASK;
	u.u_limit = CDLIMIT;

	curproc = &proc[0];


	/*
	 * initialize system tables
	 */

	for (initptr= &init_tbl[0]; *initptr; initptr++) (**initptr)();

	for (initptr= &dev_init[0]; *initptr; initptr++) (**initptr)();


	rootdir = iget(rootdev, ROOTINO);
	rootdir->i_flag &= ~ILOCK;
	u.u_cdir = iget(rootdev, ROOTINO);
	u.u_cdir->i_flag &= ~ILOCK;
	u.u_rdir = NULL;
	u.u_start = time;

	/*
	 * create initial processes
	 * start scheduling task
	 */
	if (newproc(0)) {
		struct proc *p;
		struct seg_desc *ldt;
		long a;
		register unsigned i;

		p = u.u_procp;
		/* if i ever gets >= stoc(1) then we'ld be in trouble */
		i = btoc(szicode) + 1;
		expand(p->p_size + i);
		u.u_dsize = i;
		a = ctob((long)p->p_addr + USIZE + 1);
		i = ctob(i);
		/* set up the code & data descriptors for process 1 */
		ldt = &u.u_ldtadv[CODE1_SEL];
		ldt->sd_limit = i - 1;
		ldt->sd_lowbase = loword(a);
		ldt->sd_hibase = lobyte(hiword(a));
		ldt->sd_access = ACC_UCODE;
		ldt++;
		ldt->sd_limit = i - 1;
		ldt->sd_lowbase = loword(a);
		ldt->sd_hibase = lobyte(hiword(a));
		ldt->sd_access = ACC_UDATA;
		copyout((caddr_t)icode, lstouv(CODE1_SEL + 1), szicode);
		return((long)i & 0xFFFF);
	}

	if (newproc(0)) {
		register struct proc *p;
		extern	xsched();

		p = u.u_procp;
		/*
		** must take p_size * 2 because we want
		** maxmem to be the maximum CONTIGUOUS
		** ram
		*/
		maxmem -= (p->p_size + 1) * 2;
		p->p_flag |= SLOAD|SSYS;
		p->p_pid = 0;
		return((long)xsched);
	}
	sched();
}

/*
 * iinit is called once (from main) very early in initialization.
 * opens rootdev, pipedev, swapdev. Then calls srmount to mount
 * the root superblock and initialize the current date.
 */
iinit()
{
	(*bdevsw[bmajor(rootdev)].d_open)(minor(rootdev), 1);
	(*bdevsw[bmajor(pipedev)].d_open)(minor(pipedev), 1);
	(*bdevsw[bmajor(swapdev)].d_open)(minor(swapdev), 1);
	srmount(1);
}

/*
 * Initialize clist by freeing all character blocks.
 */
struct chead cfreelist;
cinit()
{
	register n;
	register struct cblock *cp;

	for(n = 0, cp = &cfree[0]; n < v.v_clist; n++, cp++) {
		cp->c_next = cfreelist.c_next;
		cfreelist.c_next = cp;
	}
	cfreelist.c_size = CLSIZE;
}

/*
 * Initialize the buffer I/O system by freeing
 * all buffers and setting all device hash buffer lists to empty.
 */
binit()
{
	extern struct buf	buf[];
	extern char		buffers[][SBUFSIZE];
	register struct buf	*bp;
	register struct buf	*dp;
	register unsigned	i;

	dp = &bfreelist;
	dp->b_forw = dp->b_back =
	    dp->av_forw = dp->av_back = dp;

	if (bp == NULL || buffers == NULL)
		panic("binit");
	bufstart = buffers;

	bp = &buf[0];

	for (i=0; i<v.v_buf; i++,bp++) {
		bp->b_dev = NODEV;
		bp->b_un.b_addr = buffers[i];
		bp->b_back = dp;
		bp->b_forw = dp->b_forw;
		dp->b_forw->b_back = bp;
		dp->b_forw = bp;
		bp->b_flags = B_BUSY;
		bp->b_bcount = 0;
		brelse(bp);
	}
	pfreelist.av_forw = bp = pbuf;
	for (; bp < &pbuf[v.v_pbuf-1]; bp++)
		bp->av_forw = bp+1;
	bp->av_forw = NULL;
	for (i=0; i < v.v_hbuf; i++)
		hbuf[i].b_forw = hbuf[i].b_back = (struct buf *)&hbuf[i];
}
