static char *uportid = "@(#)main.c	Microport Rev Id 1.3.8  11/24/86";
/* (#)main.c	1.26 */
/*		Copyright 1985 Microport Systems
 *		All Rights Reserved.
 *
 * Modification History:
 *	The general upgrade to run on the AT involves:	
 *	1) Includes mem-size-select tables for number of buffers and clists 
 *	2) Move initpic to beginning of everything, to solve crt problems.
 *	3) Moved the mfree allocation of swapmap from startup() to end of iinit.
 *
 * M000:	uport!dwight Mon Nov 24 16:52:59 PST 1986
 *	Made ulimit patchable.
 * M001:	uport!rex	Thu Jun 25 01:03:22 PDT 1987
 *	NBUF and NCLIST now override the table lookup if they are defined
 *	in "dfile" as non-zero.  If they are zero in dfile, then the number
 *	of buffers and clists is taken from the memory tables as always.
 * M002:	uport!rex	Tue Sep  1 18:49:01 PDT 1987
 *	Call open for root device with mount flag set if not fixed disk.
 *
 * Not backwards compatible with the generic release.
 */


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
#include "sys/map.h"
#ifdef ATMERGE
#include "sys/realmode.h"
#endif /* ATMERGE */

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
#ifdef IBMAT
extern long foundmem;
#endif IBMAT
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
	extern daddr_t ulpatch;					/* M000 */
#ifdef ATMERGE
	extern int topalloc;
	topalloc = 1;            /* allocate memory from top down for now */
#endif /* ATMERGE */

	initpic();	spl7();	

	asydbginit ();	/* init console and debugger I/O */
	startup();

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
	u.u_limit = ulpatch;			/* M000 */

	curproc = &proc[0];

	/*
	 * initialize system tables
	 */


	for (initptr= &init_tbl[0]; *initptr; initptr++) {
		(**initptr)();
	}

	asm(" sti");				/* turn on irq's	*/
	spl0();					/* enable all irq's	*/

	for (initptr= &dev_init[0]; *initptr; initptr++)  {
		(**initptr)();
	}

	/* Free all available memory */
	freemem();
	printf( "\nMicroport's System V/AT Release %s %s Version %s\n",
		utsname.release, utsname.machine, utsname.version );
	printf("Copyright (c) 1985 AT&T             - All Rights Reserved\n");
	printf("Copyright (c) 1985, 1986 Microport  - All Rights Reserved\n");


	rootdir = iget(rootdev, ROOTINO);
	rootdir->i_flag &= ~ILOCK;
	u.u_cdir = iget(rootdev, ROOTINO);
	u.u_cdir->i_flag &= ~ILOCK;
	u.u_rdir = NULL;
	u.u_start = time;

#ifdef ATMERGE
	topalloc = 0;       /* done with initialization, allocate normally */
#endif /* ATMERGE */

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
#ifdef ATMERGE
	topalloc = 1;     /* allocate memory top down for xsched _u */
#endif /* ATMERGE */

	if (newproc(0)) {
		register struct proc *p;
		extern	xsched();

		p = u.u_procp;
#ifdef ATMERGE
                /* with topalloc we don't lose process 1 _u to fragmentation */
                /* so we only need to take away the space for process 2 _u */
                maxmem -= p->p_size;
#else /* -ATMERGE */
		/*
		** must take p_size * 2 because we want
		** maxmem to be the maximum CONTIGUOUS
		** ram
		*/
		maxmem -= (p->p_size + 1) * 2;
#endif /* ATMERGE */
		p->p_flag |= SLOAD|SSYS;
		p->p_pid = 0;
		return((long)xsched);
	}
#ifdef ATMERGE
	topalloc = 0;   /* all done forking xsched, allocate normally */
#endif /* ATMERGE */
	sched();
}

/*
 * iinit is called once (from main) very early in initialization.
 * opens rootdev, pipedev, swapdev. Then calls srmount to mount
 * the root superblock and initialize the current date.
 */
iinit()
{
#ifdef LCCFIX
	int	omnt = ((rootdev & 0xfff) != 0);	/* M002 */
	(*bdevsw[bmajor(rootdev)].d_open)(minor(rootdev), 1, omnt);
	(*bdevsw[bmajor(pipedev)].d_open)(minor(pipedev), 1, 0);
	(*bdevsw[bmajor(swapdev)].d_open)(minor(swapdev), 1, 0);
#else
	(*bdevsw[bmajor(rootdev)].d_open)(minor(rootdev), 1);
	(*bdevsw[bmajor(pipedev)].d_open)(minor(pipedev), 1);
	(*bdevsw[bmajor(swapdev)].d_open)(minor(swapdev), 1);
#endif /* ! LCCFIX */
	srmount(1);
#ifdef	IBMAT
	mfree(swapmap, nswap, 1);		/* parameters picked up */
	swplo--;				/* by wnopen		*/
#endif	IBMAT
}

char * valloc();

/* how many clists for <1M, 1-2M, etc. */
int nclist_tab[16] = {
	40,
	45,
	50,
	60,
	70,
	100,
	100,
	100,
	100,
	100,
	100,
	100,
	100,
	100,
	100,
	100
};

/*
 * Initialize clist by freeing all character blocks.
 */
struct chead cfreelist;
cinit()
{
	register n;
	register struct cblock *cp;
	extern struct cblock *cfree;

	if (v.v_clist == 0)			/* M001 */
		v.v_clist = nclist_tab[ ctob((long) physmem) >>20];
						/* 0x100000,200000,etc*/
	printf("clists = %d\n", v.v_clist);
	/* CMATCH macro in tty.h assumes they are in one array */
	cfree = (struct cblock *) valloc(sizeof (struct cblock) * v.v_clist, 0);
	for(n = 0, cp = &cfree[0]; n < v.v_clist; n++, cp++) { /* */
		cp->c_next = cfreelist.c_next;
		cfreelist.c_next = cp;
	}
	cfreelist.c_size = CLSIZE;
}

/* how many buffers for <1M, 1-2M, etc. */
struct nbuf_tab {
	long	size;
	int		nbuf;
} nbuf_tab[12] = {
	0x80000, 	25,		/* 512K = 25 buffers */
	0xa0000, 	45,
	0x100000, 	100,
	0x140000, 	200,
	0x180000, 	300,
	0x200000, 	500,
	0x300000, 	800,
	0x400000, 	1100,
	0x600000, 	1400,
	0x800000, 	1700,
	0x1000000, 	2000,
	0x10000000,	0,		/* can't possibly get here */
};


/*
 * Initialize the buffer I/O system by freeing
 * all buffers and setting all device hash buffer lists to empty.
 */

binit()
{
	register struct buf	*bp;
	register struct buf	*dp;
	register unsigned	i;

	dp = &bfreelist;
	dp->b_forw = dp->b_back =
	    dp->av_forw = dp->av_back = dp;

	if (v.v_buf == 0)			/* M001 */
		for(i=0; nbuf_tab[i].size <= foundmem; i++)
			v.v_buf = nbuf_tab[ i ].nbuf;
	printf("buffers = %dK\n", v.v_buf);
	for (i=0; i< v.v_buf; i++ ) {
		/* alloc bp on the fly allows more than 1200 buffers  */
		bp = (struct buf *) valloc( sizeof (struct buf), 0);  
		bp->b_dev = NODEV;
		/* 1 here means may not straddle 64K */
		bp->b_un.b_addr = (caddr_t) valloc(SBUFSIZE, 1);	
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
	/* alloc hbuf on fly, too */
	for(i=0; (i < 11) && ((1<<i) < v.v_buf) ; i++)
		continue;	/* calc appropriate hashing size, max 2048 */
	v.v_hbuf  = 1 << i;
	v.v_hmask = v.v_hbuf - 1;	/* mask for mod operation (see buf.h) */
	/* must be one big hunk, ok if crosses DMA page */
	hbuf = (struct hbuf *) valloc(sizeof (struct hbuf) * v.v_hbuf, 0);
	for (i=0; i < v.v_hbuf; i++)
		hbuf[i].b_forw = hbuf[i].b_back = (struct buf *)&hbuf[i];

}

/* Allocate physical memory and map in descriptors 
   Must not be called after other memory allocation has taken place */

#define	NVSEL	40

/*
 * This assumes that successive calls to malloc get successive physical memory.
 * Memsize should just build tables of available memory ranges, and
 * this should grab out of those tables and change them.
 * After initialization, then a separate vallocend() 
 * function should free up the remaining memory.
 */

#ifdef ATMERGE
/* Since getmem() under ATMERGE allocates top-down, valloc must account
   for that in allocating selectors. */
char * 
valloc(size, pageflg)		
unsigned size; /* Size in bytes */
{
	unsigned long phys_mem;
	static selnum = -1;
	char *ret_val;
	static unsigned long mem_avail;  /* Memory allocated thus far */
	extern struct seg_desc gdt[];
		/* offsets in gdt of selectors */
	extern struct seg_desc vall_sel[], vall_selend[]; 
	int i, j, first;
	struct buf b;
	unsigned long hunk;

	hunk = size;
	/* if first time through get offset into get */
	if (selnum == -1) {
		selnum = vall_sel - gdt;
		selnum--;		/* hack: see ++selnum */
		first = 1;
		mem_avail = 0x10000L;
	} else first = 0;
	
/*		phys_mem  = ctob((unsigned long) malloc(coremap,size)); /* */
	
	if ((!getmem(hunk, &phys_mem, pageflg)) || first || 
			(mem_avail < hunk)) {
		/* Start a new selector because getmem gave us new 
		 * physical memory or this is the first time through 
		 * or we overran our selector window 
		 */
		if (++selnum == (vall_selend - gdt))
			panic("Valloc: Ran out of selectors!");
		/* fill in new selector base such that the phys address
		   just allocated rams up against the very end of a
		   segment. */
		phys_mem = phys_mem + hunk - 0x10000L;
		gdt[selnum].sd_lowbase = phys_mem & 0xFFFF;
		gdt[selnum].sd_hibase =  (phys_mem>>16) & 0xFF;
		mem_avail = 0x10000L - hunk;
		gdt[selnum].sd_limit = mem_avail - 1;
		gdt[selnum].sd_access = ACC_KDATA | DSC_PRESENT | SD_EXPND_DN;
		ret_val = &(gstokv(selnum))[mem_avail];
	} else {
		/* both physical addr and logical addr are contig with last valloc */
		mem_avail -= hunk;
		ret_val = &(gstokv(selnum))[ mem_avail ];
		gdt[selnum].sd_limit -= hunk;
	}
	return ret_val;
}

#else /* ! ATMERGE */

char * 
valloc(size, pageflg)		
unsigned size; /* Size in bytes */
{
	unsigned long phys_mem;
	static selnum = -1;
	char *ret_val;
	static unsigned long mem_used;  /* Memory allocated thus far */
	extern struct seg_desc gdt[];
		/* offsets in gdt of selectors */
	extern struct seg_desc vall_sel[], vall_selend[]; 
	int i, j, first;
	struct buf b;
	unsigned long hunk;

	hunk = size;
	/* if first time through get offset into get */
	if (selnum == -1) {
		selnum = vall_sel - gdt;
		selnum--;		/* hack: see ++selnum */
		first = 1;
		mem_used = 0;
	} else first = 0;
	
/*		phys_mem  = ctob((unsigned long) malloc(coremap,size)); /* */
	
	if ((!getmem(hunk, &phys_mem, pageflg)) || first || 
			(mem_used+hunk > 0x0FFFFL)) {
		/* Start a new selector because getmem gave us new 
		 * physical memory or this is the first time through 
		 * or we overran our selector window 
		 */
		if (++selnum == (vall_selend - gdt))
			panic("Valloc: Ran out of selectors!");
		/* fill in new selector at phys address allocated */
		gdt[selnum].sd_lowbase = phys_mem & 0xFFFF;
		gdt[selnum].sd_hibase =  (phys_mem>>16) & 0xFF;
#ifdef LCCFIX 
		gdt[selnum].sd_limit = (mem_used = hunk) - 1;
#else /* !LCCFIX */
		gdt[selnum].sd_limit = mem_used = hunk;
#endif /* LCCFIX */
		gdt[selnum].sd_access = ACC_KDATA | DSC_PRESENT;
		ret_val = gstokv(selnum);
	} else {
		/* both physical addr and logical addr are contig with last valloc */
		ret_val = &(gstokv(selnum))[ mem_used ];
		gdt[selnum].sd_limit = (mem_used += hunk) - 1;
	}
	return ret_val;
}
#endif /* ATMERGE */
