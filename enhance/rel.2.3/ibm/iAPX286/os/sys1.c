static char *uportid = "@(#)sys1.c	Microport Rev Id 2.2.5  4/28/87";
/*	Copyright (c) 1985 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/
/*								*/
/*	M000: uport!rex		4/28/87				*/
/*	Added a check in the end of exec() for ownership of	*/
/*	the floating point unit, and now release it if we	*/
/*	owned it						*/

/*      @(#)sys1.c	1.44 - 85/09/13       */
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
#include "sys/buf.h"
#include "sys/reg.h"
#include "sys/file.h"
#include "sys/inode.h"
#include "sys/seg.h"
#include "sys/mmu.h"
#include "sys/acct.h"
#include "sys/sysinfo.h"
#include "sys/var.h"
#include "sys/ipc.h"
#include "sys/shm.h"
#include "sys/text.h"
#ifdef ATMERGE
#include "sys/realmode.h"
#endif /* ATMERGE */


/*
 * exec system call, with and without environments.
 */
struct execa {
	char	*fname;
	/*
	** actually argp & envp are `char **'
	** but that makes dealing with different models harder.
	*/
	char	*argp;
	char	*envp;
};

exec()
{
	((struct execa *)u.u_ap)->envp = NULL;
	exece();
}

#define NBPLP	4
#define NBPSP	2

#if FsTYPE == 2
#define	NCABLK	((NCARGS+NBPC-1)/NBPC)+1
#else
#define	NCABLK	(NCARGS+NBPC-1)/NBPC
#endif
#define	NCAFSBLK	((NCARGS+BSIZE-1)/BSIZE)

exece()
{
	register unsigned nc;
	register char *cp;
	register struct buf *bp;
	register struct execa *uap;
	int na, ne, c;
	long ucp, ap;
	unsigned bno;
	daddr_t swap1k;
	struct inode *ip;
	extern struct inode *gethead();
#ifdef ATMERGE
	int isdosfile;  /* if non-zero, gotta play games with arglist */
#endif /* ATMERGE */
	extern long fuptr();
	int nbpp;
	unsigned sel;
	extern struct proc *fp_proc;

	sysinfo.sysexec++;
#ifndef ATMERGE
	if ((ip = gethead()) == NULL)
#else /* ATMERGE */
	if ((ip = gethead(&isdosfile)) == NULL)   
#endif
		return;
	bp = 0;
	na = nc = ne = 0;
	uap = (struct execa *)u.u_ap;
	/* collect arglist */
	if ((bno = swapalloc(NCABLK, 0)) == 0) {
		printf("No swap space for exec args\n");
		iput(ip);
		u.u_error = ENOMEM;
		return;
	}

	/* Set swap1k to be the actual file system logical block number
	   associated with "swplo + bno" and adjust it to be on a file system
	   logical block boundary. */
#if FsTYPE == 2
	swap1k = (swplo + bno + 1) >> 1;
#else
	swap1k = swplo + bno;
#endif
	if (u.u_model & U_MOD_SSTACK) {
		nbpp = NBPLP;
		sel = STACK_SEL;
	} else {
		nbpp = NBPSP;
		sel = find_stack();
	}
#ifndef ATMERGE
	if (uap->argp) for (;;) {
		ap = NULL;
		if (uap->argp) {
			ap = fuptr((caddr_t)uap->argp, sel);
			uap->argp += nbpp;
		}
#else /* ATMERGE */
	if ((uap->argp) || isdosfile)  for (;;) {
		ap = NULL;
		if (isdosfile) {
			/* pass name of original exec'd file as arg 0 
			*  to execee
			*/
			ap = (long)(uap->fname);
			isdosfile = 0;
		} else {
			if (uap->argp) {
				ap = fuptr((caddr_t)uap->argp, sel);
				uap->argp += nbpp;
			}
		}
#endif /* ATMERGE */
		if (ap==NULL && uap->envp) {
			uap->argp = NULL;
			if ((ap = fuptr((caddr_t)uap->envp, sel)) == NULL)
				break;
			uap->envp += nbpp;
			ne++;
		}
		if (ap==NULL)
			break;
		na++;
		if (ap == -1)
			u.u_error = EFAULT;
		do {
			if (nc >= NCARGS-1)
				u.u_error = E2BIG;
			if ((c = fubyte((caddr_t)ap++)) < 0)
				u.u_error = EFAULT;
			if (u.u_error)
				goto bad;
			if ((nc&BMASK) == 0) {
				if (bp)
					bdwrite(bp);
				bp = getblk(swapdev, swap1k + (nc>>BSHIFT));
				cp = bp->b_un.b_addr;
			}
			nc++;
			*cp++ = c;
		} while (c>0);
	}
	if (bp)
		bdwrite(bp);
	bp = 0;
	nc = (nc + NBPW-1) & ~(NBPW-1);
	/* small doesn't use second arg so we just set it up for large */
	getxfile(ip, nc + NBPLP*na);
	if (u.u_error) {
		psignal(u.u_procp, SIGKILL);
		goto bad;
	}

	/* copy back arglist */

	if (u.u_model & U_MOD_SSTACK) {
		nbpp = NBPLP;
		ucp = (long)lstouv(STACK_SEL) + ctob((long)stoc(1)) - nc - NBPW;
		ap = (long)lstouv(STACK_SEL) + (ushort)ucp - na*NBPLP - 2*NBPLP - NBPW;
	} else {
		nbpp = NBPSP;
		sel = find_stack();
		ucp = (long)lstouv(sel) + u.u_ossize - nc - NBPW;
		ap = (long)lstouv(sel) + (ushort)ucp - na*NBPSP - 2*NBPSP - NBPW;
	}
	u.u_ar0[SS] = (ushort)(ap >> 16);
	u.u_ar0[SP] = (ushort)ap;	/* only need offset portion */
	suword((caddr_t)ap, na-ne);
	ap += NBPW;
	nc = 0;
	for (;;) {
		if (na==ne) {
			ap += nbpp;
		}
		if (--na < 0)
			break;
		suptr((caddr_t)ap, ucp);
		ap += nbpp;
		do {
			if ((nc&BMASK) == 0) {
				if (bp)
					brelse(bp);
				bp = bread(swapdev, swap1k + (nc>>BSHIFT));
				bp->b_flags |= B_AGE;
				bp->b_flags &= ~B_DELWRI;
				cp = bp->b_un.b_addr;
			}
			subyte((caddr_t)ucp++, (c = *cp++));
			nc++;
		} while (c&0377);
	}
	setregs();
						/* start M000 */
	/*
	** if this process owned the floating point unit,
	** now signify that nobody owns it
	*/
	if ( fp_proc == u.u_procp )
		fp_proc = (struct proc *)0;
	u.u_fpvalid = 0;
						/* end M000 */
	if (bp)
		brelse(bp);
	iput(ip);
	mfree(swapmap, NCABLK, bno);
	return;
bad:
	if (bp)
		brelse(bp);
	iput(ip);
	for (nc = 0; nc < NCAFSBLK; nc++) {
		bp = getblk(swapdev, swap1k + nc);
		bp->b_flags |= B_AGE;
		bp->b_flags &= ~B_DELWRI;
		brelse(bp);
	}
	mfree(swapmap, NCABLK, bno);
}

#ifdef ATMERGE 
char *dosexecpgm = "/etc/dosexec";   /* this is the program that gets
                                     ** exec'd when gethead is called on 
                                     ** a file which turns out to be a
                                     ** dos format file.
                                     */
#endif /* ATMERGE */

struct inode *
#ifndef ATMERGE
gethead()
#else /* ATMERGE */
gethead(isdosfile)
int   *isdosfile;      /* non-zero -> special arg 0 treatment on return */
#endif /* ATMERGE */
{
	register struct inode *ip;
	register unsigned ds, ts, ls, ss;
	unsigned i;
	int itscoff = 0;

	struct cofffilehdr {
		unsigned short	f_magic;	/* magic number */
		unsigned short	f_nscns;	/* number of sections */
		long		f_timdat;	/* time & date stamp */
		long		f_symptr;	/* file pointer to symtab */
		long		f_nsyms;	/* number of symtab entries */
		unsigned short	f_opthdr;	/* sizeof(optional hdr) */
		unsigned short	f_flags;	/* flags */
	} ;

#define	COFFSMAGIC	0512
#define	COFFLMAGIC	0522
#define	COFFUHMAGIC	0410

	struct aouthdr {
		short	magic;		/* see magic.h				*/
		short	vstamp;		/* version stamp			*/
		long	tsize;		/* text size in bytes, padded to FW
					   bdry					*/
		long	dsize;		/* initialized data "  "		*/
		long	bsize;		/* uninitialized data "   "		*/
		long	entry;		/* entry pt.				*/
		long	text_start;	/* base of text used for this file	*/
		long	data_start;	/* base of data used for this file	*/
	};

	struct stlfilehdr {
		ushort	fh_magic;
		ushort 	fh_exthdrsiz;
		long	fh_ssiz;
		long	fh_nsisiz;
		long	fh_nsuisiz;
		long	fh_symtblsiz;
		long	fh_relocsiz;
		long	fh_ipoffset;
		char	fh_pspec;
		char	fh_stbltype;
		ushort	fh_flags;
	};

#define STLMAGIC	01006

#define FH_MASK		0x0897	/* FST, FPH, OVERLAY, PURE, SEP, EXEC */
#define FH_VALID	0x0807	/* FST, PURE, SEP, EXEC */
#define FH_HUGE		0x0100
#define FH_NNSS		0x0020
#define FH_NSS		0x0040
#define FH_SEPSTK	0x4000

	struct exthdr {
		struct pre_exthdr {
			long	e_relsiz;
			long	e_nrelsiz;
			long	e_base;
			long	e_nbase;
		} e_pre;
		struct abbr_exthdr {
			long	e_stksiz;
			long	e_fstptr;
			long	e_fstsiz;
			long	e_mdtptr;
			long	e_mdtsiz;
			char	e_mdttyp;
			char	e_blkgran;
			char 	e_ostype;
			char	e_osversion;
			ushort	e_csselector;
		} e_ab;
		char 	reserved[22];
		ushort  e_ngates;
		ushort  e_extldtsiz;
		char	i_reserved[50];
		long	e_symptr; 
		long	e_timdat;
		ushort	e_flags;
	};

	struct sechdr {
		ushort	se_type;	/* text, data or iterated data */
		ushort  se_flags;	/* bss, shareable, expand_down, etc. */
		ushort  se_num;		/* should match ldt entry number */
		ushort  se_nlnno;
		long	se_scnptr;	/* file pointer to body */
		long	se_psize;	/* physical size */
		long	se_vsize;	/* virtual size */
		long	se_relptr;
		long	se_vaddr;     	/* virtual address */
		long	se_lnnoptr;
	};

	char exbuf[32];
#define stlfhp	((struct stlfilehdr *)exbuf)
#define cofffhp	((struct cofffilehdr *)exbuf)
#define aehp	((struct abbr_exthdr *)exbuf)
#define	unixhp	((struct aouthdr *)exbuf)

#ifdef ATMERGE
	extern int schar();
	char *cp;
	int oktype;	/* non-zero means load module type was acceptable */

#define	EXETYPE	1	/* file is executable under unix */
#define	DOSTYPE	2	/* file is executable under dos */
#define	DOSEXEMAGIC	0x5a4d
#define	DOSBLTMAGIC	0x5a4c	/* arbitrary != DOSEXE or I286MAGIC */
	
	*isdosfile = 0;	/* not dos, by default */

tryagain:
	if ((ip = namei((*isdosfile ? schar : uchar), 0)) == NULL)

#else /* -ATMERGE */
	if ((ip = namei(uchar, 0)) == NULL)
#endif /* -ATMERGE */
		return(NULL);
	if (access(ip, IEXEC) ||
	   (ip->i_mode & IFMT) != IFREG ||
	   (ip->i_mode & (IEXEC|(IEXEC>>3)|(IEXEC>>6))) == 0) {
		u.u_error = EACCES;
		goto bad;
	}
	/*
	 * Find out which format the file is in: COFF or STL
	 */
	u.u_base = exbuf;
#ifdef ATMERGE 
	u.u_count = sizeof(struct cofffilehdr);
#else /* -ATMERGE */
	u.u_count = sizeof(unsigned short);
#endif /* -ATMERGE */
	u.u_offset = 0;
	u.u_segflg = 1;
	readi(ip);
#ifdef ATMERGE
	oktype = 0;  /* 0 if not recognized or !0 if valid executable */
#endif /* ATMERGE */
        switch (cofffhp->f_magic) {
          case COFFSMAGIC:
#ifdef ATMERGE 
		oktype = EXETYPE;
#endif /* ATMERGE */
		u.u_exdata.ux_ssize = 0;
		ss = 0;
		itscoff = U_MOD_ISCOFF | U_MOD_SMALL;
		break;
          case COFFLMAGIC:
#ifdef ATMERGE
                oktype = EXETYPE;
#endif /* ATMERGE */
		u.u_exdata.ux_ssize = ctob(SSIZE);
		ss = btoc(ctob(SSIZE) + NCARGS - 1);
		itscoff = U_MOD_ISCOFF | U_MOD_LARGE;
                break;
#ifdef	ATMERGE
	case DOSEXEMAGIC:

		/* it looks like a DOS .exe */
		oktype = DOSTYPE;
		*isdosfile = 1;	/* let exece know its funny */
		break;

	case DOSBLTMAGIC:

		/* encapsulated DOS program */
		oktype = DOSTYPE;
		*isdosfile = 3;
		break;

#endif /* ATMERGE */
        default:
		if (stlfhp->fh_magic == STLMAGIC) {
#ifdef ATMERGE
			oktype = EXETYPE;
#endif /* ATMERGE */
			break;
		}
#ifndef ATMERGE 
noexec:
		u.u_error = ENOEXEC;
		goto bad;
	}       /* end of switch(magic) */
#else /* ATMERGE */
		/* No magic number.
		** Maybe it's a .com or a .bat file.
		** Unfortunately, .com's have no fixed internal format
		** so the following algorithm is intended to
		** distinguish between shell files (where exec
		** should fail) and .com files (where magic stuff
		** is supposed to happen).  .bat's are recognized
		** by the \r before the first \n in the file.
		** .bat file recognition should be done in the
		** shell, not here, but such are `requirements'.
		*/

		cp = &exbuf[0];
		for (i=sizeof (struct cofffilehdr) - u.u_count;i;i--) {
			if (*cp++&0200) {
				/* high order bit on, assume this isn't
				** a shell file
				*/
				oktype = DOSTYPE; 
				*isdosfile = 2;	  /* flag as dos file */
				goto endmagic;
			}
		}

		/* Now look for the .bat files */
#define MAXDOSLINE 129	/* 127+CR+NL */
		{
		    int wascr=0, n=0;
		    i = sizeof(struct cofffilehdr) - u.u_count; /* read above */
		    while (1) {
			cp = &exbuf[0];
			while (i--) {
				if (*cp == '\n') {
					if (wascr) {
						oktype = DOSTYPE;
						*isdosfile = 4;
					}
					goto endmagic;	/* found \n, so done */
				} else
					wascr = (*cp++ == '\r');
				n++;
			}
			if (u.u_count || n>=MAXDOSLINE)
				break;		/* EOF or real long line */
			u.u_base = exbuf;
			u.u_count = sizeof(exbuf);
			u.u_offset = n;
			u.u_segflg = 1;
			readi(ip);
			i = sizeof(exbuf) - u.u_count;
			if (n+i>MAXDOSLINE)
				i = MAXDOSLINE-n;
		    }
		}
		break;
	}	/* end of switch(magic) */
endmagic:

	if (!oktype) {
noexec:
		u.u_error = ENOEXEC;
		goto bad;
	}
	if (oktype == DOSTYPE) {
		/* current file is of DOS type */
		u.u_segflg = 0;
		iput(ip);	/* give old inode back */
		u.u_dirp = dosexecpgm;
		goto tryagain;	/* exec through interpreter */
	}

#endif	/* ATMERGE */

	if (itscoff) {
		/* read in coff filehdr */
		u.u_base = exbuf;
		u.u_count = sizeof(struct cofffilehdr);
		u.u_offset = 0;
		readi(ip);
		u.u_exdata.ux_flags = itscoff;
		u.u_exdata.ux_ldtend = cofffhp->f_nscns + CODE1_SEL;
		u.u_exdata.ux_lsize = u.u_exdata.ux_ldtend
					* sizeof(struct seg_desc);
		u.u_base = exbuf;
		u.u_count = sizeof(struct aouthdr);
		u.u_offset = sizeof(struct cofffilehdr);
		readi(ip);
		if (unixhp->magic != COFFUHMAGIC)
			goto noexec;
		u.u_exdata.ux_tsize = unixhp->tsize;
		u.u_exdata.ux_dsize = unixhp->dsize;
		u.u_exdata.ux_bsize = unixhp->bsize;
		u.u_exdata.ux_ip = loword(unixhp->entry);
		u.u_exdata.ux_cs = hiword(unixhp->entry);
		u.u_exdata.ux_fstoff = sizeof(struct cofffilehdr)
					+ sizeof(struct aouthdr);
		ts = btoc(u.u_exdata.ux_tsize);
		ds = btoc(u.u_exdata.ux_dsize + u.u_exdata.ux_bsize);
	} else {
		/*
		 * read in first few bytes of file for segment sizes
		 *  1006 is STL object
		 */
		u.u_base = exbuf;
		u.u_count = sizeof(struct stlfilehdr);
		u.u_offset = 0;
		u.u_segflg = 1;
		readi(ip);
		/* squirrel away info */
		u.u_exdata.ux_tsize = stlfhp->fh_ssiz;
		u.u_exdata.ux_dsize = stlfhp->fh_nsisiz;
		u.u_exdata.ux_ip = (ushort)stlfhp->fh_ipoffset;
		u.u_exdata.ux_flags = stlfhp->fh_flags;
		u.u_exdata.ux_bsize = (u.u_exdata.ux_flags & FH_SEPSTK) ?
							stlfhp->fh_nsuisiz : 0L;
		/* read in next hunk */

		u.u_base = exbuf;
		u.u_count = sizeof(struct abbr_exthdr);
		u.u_offset = sizeof(struct stlfilehdr) + sizeof(struct pre_exthdr);
		readi(ip);
		u.u_exdata.ux_ssize = aehp->e_stksiz;
		i = aehp->e_fstsiz / sizeof(struct sechdr) + CODE1_SEL;
		u.u_exdata.ux_lsize = i * sizeof(struct seg_desc);
		u.u_exdata.ux_cs = aehp->e_csselector;
		u.u_exdata.ux_fstoff = aehp->e_fstptr;
		u.u_exdata.ux_ldtend = i;
		 if ((u.u_exdata.ux_flags & FH_MASK) != FH_VALID)
			goto noexec;
		ts = btoc(u.u_exdata.ux_tsize);

		/* Check for seperate stack */
		if (u.u_exdata.ux_flags & FH_SEPSTK) {
			ds = btoc(u.u_exdata.ux_dsize+u.u_exdata.ux_bsize);
			ss = btoc(u.u_exdata.ux_ssize + NCARGS - 1);
		} else {
			ds = btoc(u.u_exdata.ux_dsize + u.u_exdata.ux_bsize);
			ss = 0;
		}
		i = 0;
		if (u.u_exdata.ux_flags & FH_HUGE)
			i |= U_MOD_HARRAY;
		if (u.u_exdata.ux_flags & FH_NNSS)
			i |= U_MOD_MDATA;
		if (u.u_exdata.ux_flags & FH_NSS)
			i |= U_MOD_MTEXT;
		if (u.u_exdata.ux_flags & FH_SEPSTK)
			i |= U_MOD_SSTACK;
		u.u_exdata.ux_flags = i;
	}
	if (u.u_count != 0) {
		goto noexec;
	}
	/* note that currently all processes are shared text */
	if ((ip->i_flag&ITEXT)==0 && ip->i_count!=1) {
		register struct file *fp;

		for (fp = file; fp < (struct file *)v.ve_file; fp++)
			if (fp->f_count && fp->f_inode == ip &&
			    (fp->f_flag&FWRITE)) {
				u.u_error = ETXTBSY;
				goto bad;
			}
	}
	ls = btoc(u.u_exdata.ux_lsize);

	chksize(ts, ds, ss, ls);
bad:
	u.u_segflg = 0;
	if (u.u_error) {
		iput(ip);
		ip = NULL;
	}
	return(ip);
}

/*
 * Read in and set up memory for executed file.
 */
getxfile(ip, nargc)
register struct inode *ip;
{
	register int (**execptr)();
	extern int (*dev_exec[])();
	register i;		/* segment # */
	register unsigned ts, ds, ss, ls;	/* in clicks */
	unsigned first;		/* first click to be cleared */
	unsigned limit;		/* limit of clearing */
	unsigned segoff;	/* virtual offset in segment */
	long secoff;		/* section header offset in file
				 */
	struct proc *p;
	struct seg_desc *ldt, *lap;
	int what;		/* type of section being processed
				 */
	long a;
	ushort base;		/* click # of start of contiguous clicks
				 * for the region (text or data)
				 */
	ushort offset;		/* offset (in clicks) in region
				 */
	ushort b;		/* number of clicks allocated in region
				 */

	/* STL section format */
	struct sechdr {
		ushort	se_type;	/* text, data or iterated data */
		ushort  se_flags;	/* bss, shareable, expand_down, etc. */
		ushort  se_num;		/* should match ldt entry number */
		ushort  se_nlnno;
		long	se_scnptr;	/* file pointer to body */
		long	se_psize;	/* physical size */
		long	se_vsize;	/* virtual size */
		long	se_relptr;
		long	se_vaddr;     	/* virtual address */
		long	se_lnnoptr;
	};

	/* COFF section format */
	struct scnhdr {
		char		s_name[8];	/* section name */
		long		s_paddr;	/* physical address */
		long		s_vaddr;	/* virtual address */
		long		s_size;		/* section size */
		long		s_scnptr;	/* file ptr to raw data for section */
		long		s_relptr;	/* file ptr to relocation */
		long		s_lnnoptr;	/* file ptr to line numbers */
		unsigned short	s_nreloc;	/* number of relocation entries */
		unsigned short	s_nlnno;	/* number of line number entries */
		long		s_flags;	/* section flags */
	};
#define	STYP_TEXT	0x20
#define	STYP_DATA	0x40
#define	STYP_BSS	0x80


#define S_TEXT		1
#define S_DATA		2
#define S_ITER		64

#define FSA_BSS		0x0004

	struct sechdr exbuf;
	struct scnhdr coffbuf;


	for (execptr = &dev_exec[0]; *execptr; execptr++)
		(**execptr)();

	punlock();
	xfree();

	u.u_prof.pr_scale = 0;

	ts = btoc(u.u_exdata.ux_tsize);
	if (u.u_exdata.ux_flags & U_MOD_SSTACK) {
		ds = btoc(u.u_exdata.ux_dsize + u.u_exdata.ux_bsize);
		ss = btoc(u.u_exdata.ux_ssize + nargc);
	} else {
		ds = btoc(u.u_exdata.ux_dsize + u.u_exdata.ux_bsize);
		ss = 0;
	}
	ls = btoc(u.u_exdata.ux_lsize);

	u.u_ossize = u.u_exdata.ux_ssize;
	u.u_dsize = ds;
	u.u_ssize = ss;
	u.u_lsize = ls;
	u.u_tsize = ts;
	i = USIZE + ls + ds + ss;	/* since we always have shared text */

	/* we fudge things so expand() does little work if we grab core */
	p = u.u_procp;
	p->p_ssize = 0;
	p->p_lsize = ls;
	(u.u_ldtadv + CODE1_SEL)->sd_access = 0;
	expand(i);
	p->p_ssize = ss;

	/* set up memory model */
	u.u_model = u.u_exdata.ux_flags;

	/* make ldt */
	ldt = u.u_ldtadv;
	lap = (struct seg_desc *)((char *)u.u_ldtadv + ctob(ls));

	/* make sure unused descriptors are null */
	for (i = 0; i < UPAGE_SEL; ldt++, i++)
		ldt->sd_access = 0;
	/* upage descriptor is okay (it was before) */
	ldt++;
	for ( ; ldt < lap; ldt++)
		ldt->sd_access = 0;
	lap = u.u_ldtadv + u.u_exdata.ux_ldtend;

	ldt = u.u_ldtadv + UPAGE_SEL + 1;

	if (u.u_model & U_MOD_SSTACK) {

		/* ignore the section header !! */

		a = ctob((long)p->p_addr + p->p_size - stoc(1));
		ldt->sd_hibase = lobyte(hiword(a));
		ldt->sd_lowbase = loword(a);
		ldt->sd_limit =
		    (ctob((long)stoc(1)) - ctob((long)p->p_ssize)) - 1;
		ldt->sd_access = ACC_UDATA | SD_EXPND_DN;

		/* clear the separate stack for exece */
		limit = p->p_addr + p->p_size;
		first = limit - p->p_ssize;
		while (first < limit)
			clearseg(first++);
	}
	ldt++;
	i = 9;

	/* try to find the shared text inode & always get the memory */
	/* -1: fail,  0: loaded,  1:need to load,  2:not shared */
	if ((what = xalloc(ip)) < 0) {
		u.u_error = EFAULT;
		return;
	}

	/* set up to scan the LDT from the first code segment */
	first = p->p_textp->x_caddr;
	base = first;
	offset = 0;
	b = ts;
	secoff = u.u_exdata.ux_fstoff;
	p->p_flag |= SLOCK;

	for ( ; ldt < lap; ldt++) {

		/* read in section header */
		u.u_offset = secoff;
		if (u.u_model & U_MOD_ISCOFF) {
			u.u_base = (caddr_t)&coffbuf;
			u.u_count = sizeof(struct scnhdr);
			secoff += sizeof(struct scnhdr);
		} else {
			u.u_base = (caddr_t)&exbuf;
			u.u_count = sizeof(struct sechdr);
			secoff += sizeof(struct sechdr);
		}
		u.u_segflg = 1;
		readi(ip);
		if (u.u_model & U_MOD_ISCOFF) {
			/* for now, fit COFF back into STL */
			exbuf.se_type = 0;
			exbuf.se_flags = 0;
			if (coffbuf.s_flags & STYP_TEXT)
				exbuf.se_type = S_TEXT;
			if (coffbuf.s_flags & STYP_DATA)
				exbuf.se_type = S_DATA;
			if (coffbuf.s_flags & STYP_BSS) {
				exbuf.se_type = S_ITER;
				exbuf.se_flags = FSA_BSS;
			}
		}

		/* now change gears if we have changed types
		 * of sections that are being processed.
		 */
		switch (what) {
		case 0:		/* text image already loaded */
		case 1:		/* shared text needs to be loaded */
		case 2:		/* non-shared text (not implemented) */
			if (exbuf.se_type == S_TEXT)
				break;
			/* we should be at the first data segment now */
			p->p_flag &= ~SLOCK;
			xunlock(p->p_textp);
			u.u_ar0[DS] = (ldt - u.u_ldtadv)<<3 | LDT_TI|USER_RPL;
			first = p->p_addr + USIZE + p->p_lsize;
			b = ds;
			offset = 0;
			what = 3;
			/* fall through */
		case 3:		/* data &/or bss */
			base = p->p_addr + USIZE + p->p_lsize;
			if (exbuf.se_type == S_DATA
			    || (exbuf.se_type == S_ITER
			    && (exbuf.se_flags & FSA_BSS)))
				break;
			/* fall through */
		default:	/* if we get here we've read all we can */
			goto done;
		}
		if (u.u_model & U_MOD_ISCOFF) {
			if (exbuf.se_type == S_DATA &&
			    (!(u.u_model & U_MOD_LARGE))) {
				u.u_exdata.ux_ssize = loword(coffbuf.s_vaddr);
				u.u_ossize = u.u_exdata.ux_ssize;
			}
			exbuf.se_num = ston(hiword(coffbuf.s_vaddr));
			/*se_nlnno not used*/
			exbuf.se_scnptr = coffbuf.s_scnptr;
			exbuf.se_psize = (exbuf.se_flags & FSA_BSS)
						? 0 : coffbuf.s_size;
			segoff = loword(coffbuf.s_vaddr);
			exbuf.se_vsize = (segoff + coffbuf.s_size + ctob(1)-1)
					& (~(long)(ctob(1)-1));
			/*se_relptr not used*/
			/*se_vaddr not used*/
			/*se_lnnoptr not used*/
		} else {
			segoff = 0;
			if (!((u.u_model & U_MOD_MODEL) == U_MOD_LARGE)) {
				if (exbuf.se_type == S_DATA)
					segoff = u.u_exdata.ux_ssize;
			}
			if (exbuf.se_vsize & (ctob(1) - 1))
				goto giveup;
		}

		/*
		 * don't need to check psize too big or vsize > 64K
		 * as those get caught when we try to read.
		 */
		if (exbuf.se_num < CODE1_SEL) {
giveup:
			u.u_error = EFAULT;
			return;
		}

		/* now adjust ldt as needed so it can be used.
		 * This is needed to handle COFF files
		 * which can have both a data and bss section
		 * in the same segment.
		 */
		if (exbuf.se_num > i) {
			if (++i != exbuf.se_num) {
				goto giveup;
			}
			a = ctob((long)(base + offset));
			ldt->sd_hibase = lobyte(hiword(a));
			ldt->sd_lowbase = loword(a);
			if (segoff) {
				/* assumes shared text sections always
				 * have segoff == 0.
				 */
				offset += btoc(segoff);
				limit = base + offset;
				while (first < limit)
					clearseg(first++);
			}
		} else {
			if (i != exbuf.se_num) {
				goto giveup;
			}
			--ldt;
			--lap;
			if (btoc(segoff) != btoc(ldt->sd_limit))
				goto giveup;
		}
		ldt->sd_limit = exbuf.se_vsize ? exbuf.se_vsize - 1 : 0;
		if (what == 0) {
			ldt->sd_access = ACC_UCODE;
			offset += btoct(exbuf.se_vsize);
			continue;
		}

		first = (base + offset) + btoct(exbuf.se_psize);
		offset += btoc(exbuf.se_vsize) - btoc(segoff);
		if (offset > b)
			goto giveup;
		limit = base + offset;
		/* clear any clicks that will not be fully overwritten */
		while (first < limit)
			clearseg(first++);

		if (ldt->sd_limit)
			ldt->sd_access = ACC_UDATA;
		if (exbuf.se_psize == 0)
			continue;

		/* read in segment.  may need to split in two pieces !? */
		u.u_base = lstouv(ldt - u.u_ldtadv) + segoff;
		u.u_offset = exbuf.se_scnptr;
		u.u_segflg = 0;
		if (exbuf.se_psize >= ctob((long)stoc(1))) {
			u.u_count = ctob(1);
			areadi(ip);
			u.u_count = ctob(stoc(1) - 1);
		} else
			u.u_count = exbuf.se_psize;
		areadi(ip);
		if (u.u_count!=0) {
			/* read failures are another good reason to die */
			u.u_error = EFAULT;
			break;
		}
		/* turn code into code */
		if (what <= 2)
			ldt->sd_access = ACC_UCODE;
	}
done:

	/*
	 * set SUID/SGID protections, if no tracing
	 */
	if ((u.u_procp->p_flag&STRC)==0) {
		if (ip->i_mode&ISUID)
			u.u_uid = ip->i_uid;
		if (ip->i_mode&ISGID)
			u.u_gid = ip->i_gid;
		u.u_procp->p_suid = u.u_uid;
	} else
		psignal(u.u_procp, SIGTRAP);
}

/*
 * Clear registers on exec
 */
setregs()
{
	register int (**rp)();
	register i;

	for (rp = &u.u_signal[0]; rp < &u.u_signal[NSIG]; rp++)
		if (*rp != SIG_IGN)
			*rp = SIG_DFL;
	u.u_ar0[BP] = 0;
	u.u_ar0[SCCS] = u.u_exdata.ux_cs;
	u.u_ar0[SCIP] = u.u_exdata.ux_ip;
	u.u_ar0[ES] = 0;
	u.u_ar0[DS] = 0;
	for (i=0; i<NOFILE; i++) {
		if ((u.u_pofile[i]&EXCLOSE) && u.u_ofile[i] != NULL) {
			closef(u.u_ofile[i]);
			u.u_ofile[i] = NULL;
		}
	}
	/*
	 * Remember file name for accounting.
	 */
	u.u_acflag &= ~AFORK;
	bcopy((caddr_t)u.u_dent.d_name, (caddr_t)u.u_comm, DIRSIZ);
}

/*
 * exit system call:
 * pass back caller's arg
 */
rexit()
{
	register struct a {
		int	rval;
	} *uap;

	uap = (struct a *)u.u_ap;
	exit((uap->rval & 0377) << 8);
}

/*
 * Release resources.
 * Enter zombie state.
 * Wake up parent and init processes,
 * and dispose of children.
 */
exit(rv)
{

	register int (**exitptr)();
	extern int (*dev_exit[])();
	register int i;
	register struct proc *p, *q;
	struct seg_desc *sdp, *ep;
	extern struct proc *fp_proc;

	p = u.u_procp;
	p->p_flag &= ~(STRC);
	p->p_clktim = 0;
	for (i=0; i<NSIG; i++)
		u.u_signal[i] = SIG_IGN;
	if ((p->p_pid == p->p_pgrp)
	 && (u.u_ttyp != NULL)
	 && (*u.u_ttyp == p->p_pgrp)) {
		*u.u_ttyp = 0;
		signal(p->p_pgrp, SIGHUP);
	}
	p->p_pgrp = 0;
	for (i=0; i<NOFILE; i++) {
		if (u.u_ofile[i] != NULL)
			closef(u.u_ofile[i]);
	}
	punlock();
	plock(u.u_cdir);
	iput(u.u_cdir);
	if (u.u_rdir) {
		plock(u.u_rdir);
		iput(u.u_rdir);
	}
	xfree();


	for (exitptr = &dev_exit[0]; *exitptr; exitptr++)
		(**exitptr)();

	acct(rv);

	p->p_stat = SZOMB;
	((struct xproc *)p)->xp_xstat = rv;
	((struct xproc *)p)->xp_utime = u.u_cutime + u.u_utime;
	((struct xproc *)p)->xp_stime = u.u_cstime + u.u_stime;
	for (q = &proc[1]; q < (struct proc *)v.ve_proc; q++) {
		if (p->p_pid == q->p_ppid) {
			q->p_ppid = 1;
			if (q->p_stat == SZOMB)
				psignal(&proc[1], SIGCLD);
			if (q->p_stat == SSTOP)
				setrun(q);
		} else
		if (p->p_ppid == q->p_pid)
			psignal(q, SIGCLD);
		if (p->p_pid == q->p_pgrp)
			q->p_pgrp = 0;
	}

	/*
	** if this process owned the floating point unit,
	** now signify that nobody owns it
	*/
	if ( p == fp_proc )
		fp_proc = (struct proc *)0;

	dispatch(&proc[0]);
	/* no deposit, no return */
}

/*
 * Wait system call.
 * Search for a terminated (zombie) child,
 * finally lay it to rest, and collect its status.
 * Look also for stopped (traced) children,
 * and pass back status from them.
 */
wait()
{
	register f;
	register struct proc *p;

loop:
	f = 0;
	for (p = &proc[1]; p < (struct proc *)v.ve_proc; p++)
	if (p->p_ppid == u.u_procp->p_pid) {
		f++;
		if (p->p_stat == SZOMB) {
			freeproc(p, 1);
			return;
		}
		if (p->p_stat == SSTOP) {
			if ((p->p_flag&SWTED) == 0) {
				p->p_flag |= SWTED;
				u.u_rval1 = p->p_pid;
				u.u_rval2 = (fsig(p)<<8) | 0177;
				return;
			}
			continue;
		}
	}
	if (f) {
		sleep((caddr_t)u.u_procp, PWAIT);
		goto loop;
	}
	u.u_error = ECHILD;
}

/*
 * Remove zombie children from the process table.
 */
freeproc(p, flag)
register struct proc *p;
{

	if (flag) {
		register n;

		n = u.u_procp->p_cpu + p->p_cpu;
		if (n > 80)
			n = 80;
		u.u_procp->p_cpu = n;
		u.u_rval1 = p->p_pid;
		u.u_rval2 = ((struct xproc *)p)->xp_xstat;
	}
	u.u_cutime += ((struct xproc *)p)->xp_utime;
	u.u_cstime += ((struct xproc *)p)->xp_stime;
	p->p_stat = NULL;
	p->p_pid = 0;
	p->p_ppid = 0;
	p->p_sig = 0L;
	p->p_flag = 0;
	p->p_wchan = 0;
}

/*
 * fork system call.
 */
fork()
{
	register a;

	sysinfo.sysfork++;
	/*
	 * Disallow if
	 *  No processes at all;
	 *  not su and too many procs owned; or
	 *  not su and would take last slot; or
	 *  not su and no space on swap.
	 * Part of check done in newproc().
	 */
	if (u.u_uid && u.u_ruid) {
		if ((a = malloc(swapmap, ctod(MAXMEM/4))) == 0) {
			u.u_error = ENOMEM;
			goto out;
		}
		mfree(swapmap, ctod(MAXMEM/4), a);
	}
	switch( newproc(1) ) {
		case 1: /* child  -- successful newproc */
			u.u_rval1 = u.u_procp->p_ppid;
			u.u_rval2 = 1;  /* child */
			u.u_start = time;
			u.u_ticks = lbolt;
			u.u_mem = u.u_procp->p_size;
			u.u_ior = u.u_iow = u.u_ioch = 0;
			u.u_cstime = 0;
			u.u_stime = 0;
			u.u_cutime = 0;
			u.u_utime = 0;
			u.u_acflag = AFORK;
			return;
		case 0: /* parent -- successful newproc */
			/* u.u_rval1 = pid-of-child; */
			break;
		default: /* unsuccessful newproc */
			u.u_error = EAGAIN;
			break;
	}
out:
	u.u_rval2 = 0; /* parent */
}

#define BRK	0

/*
 * break system call.
 *  -- bad planning: "break" is a dirty word in C.
 */
sbreak()
{
	struct a {			/* input parameters */
		int mode;
		union {
			int incr;
			char *addr;
		} arg;
	};
	register struct a *ap;		/* ptr to parameters */
	unsigned a;			/* address (clicks) */
	long al;			/* address (bytes) */
	unsigned n, d, l;		/* sizes (clicks) */
	int new;			/* flag used by bigger */
	int si, sf, sh;			/* selector indices */
	struct proc *p;			/* local for udot proc ptr */
	struct seg_desc *sp;		/* one past last LDT data entry */
	struct seg_desc *hp;		/* arg.addr LDT entry (may not exist) */
	struct seg_desc *ep;		/* one past last usable LDT entry */

	ap = (struct a *)u.u_ap;
	p = u.u_procp;

	/*
	 * find first & last data selectors.
	 * also, save data to turn large model back brk to an sbrk
	 */
	/* ep points to last possible ldt data descriptor */
	if (p->p_smbeg)
		ep = u.u_ldtadv + p->p_smbeg;
	else
		ep = (struct seg_desc *)
		    ((char *)u.u_ldtadv + ctob((long)u.u_lsize));

	/* hp is the "selector" of the brk (do not reference through it!) */
	if (ap->mode == BRK)
		hp = u.u_ldtadv + ston(hiword(ap->arg.addr));
	else
		hp = ep;

	/* start at the first unknown selector */
	sp = u.u_ldtadv + CODE1_SEL + 1;

	/* skim over the text */
	if(u.u_model & U_MOD_MTEXT)
		while (sp < ep && (sp->sd_access & (DSC_SEG|SD_CODE))
		    == (DSC_SEG|SD_CODE))
			sp++;

	/* save start of data */
	sf = sp - u.u_ldtadv;
	for(;sp < ep;sp++) {
		if((sp->sd_access & (DSC_PRESENT | DSC_SEG | SD_CODE)) !=
			(DSC_PRESENT | DSC_SEG))
			break;

		/* remember how much we passed so we can 'sbrk' it */
		if (sp < hp)
			continue;
		else if (sp == hp) {
			/* this might be negative but we catch that later */
			d = btoc(sp->sd_limit) - btoc(loword(ap->arg.addr));
		} else {
			d += btoc(sp->sd_limit);
		}
	} 
	/* save end of data */
	si = (sp - u.u_ldtadv) - 1;


	/*
	 * Now check special conditions, decide what to do
	 * and compute stuff we need.
	 */
	if (ap->mode == BRK) {
		sh = ston(hiword(ap->arg.addr));
		if (sh == si) {
		long	nl,	/* current limit */
			dl;	/* desired limit */

			/* brk in current segment */
			nl = (long)((sp - 1)->sd_limit) + 1;
			dl = loword(ap->arg.addr);

			/* round request up to click boundary */
			dl = (dl + NBPC - 1) & ~((long)(NBPC - 1));
			if (dl == nl) {
				/* brk to current brk location */
				u.u_rptr = lstouv(si+1);
			} else if (dl > nl) {
				/* brk more in current segment */
				d = btoc(dl - nl);
				l = 0;
				new = 0;
				u.u_rptr = lstouv(si) + n;
				goto bigger;
			} else /* (dl < nl) */ {
				/* brk less in current segment */
				d = btoc(nl - dl);
				goto smaller;
			}
		} else if (!(u.u_model & U_MOD_SSTACK)) {
			u.u_error = ENOMEM;
			return;
		} else if (sh > si) {
			/* brk to new segment */
			d = (sh - (si + 1)) * stoc(1)
			    + btoc(loword(ap->arg.addr));
			if ((u.u_model & U_MOD_MODEL) != U_MOD_HUGE
			    && si+1 != sh) {
				u.u_error = ENOMEM;
				return;
			} else if (hp < ep) /* enough free slots */
				l = 0;
			else if (!p->p_smbeg)	/* room to expand */
				l = btoc((sh << 3) + 1) - u.u_lsize;
			else {				/* no room left */
				u.u_error = ENOMEM;
				return;
			}
			new = 1;
			u.u_rptr = lstouv(si+1);
			goto bigger;
		} else /* (sh < si) */ {
			/* brk to previous segment */
			if (sh < sf || loword(ap->arg.addr) >
				(long)hp->sd_limit + 1) {
				u.u_error = ENOMEM;
				return;
			}
			/* 'd' was set up while we looked for si */
			goto smaller;
		}
	} else /* SBRK */ {
		if (u.u_model & U_MOD_SSTACK) {
			u.u_rptr = lstouv(si+1);
		} else {
			u.u_rptr = lstouv(si) + ctob((long)u.u_dsize);
		}

		if (ap->arg.incr == 0) {
			/* sbrk zero */
			return;
		} else if (ap->arg.incr > 0) {
			/* sbrk more */
			d = btoc(ap->arg.incr);
			if (u.u_model & U_MOD_SSTACK) {
				if (sp < ep)		/* a free slot */
					l = 0;
				else if (!p->p_smbeg && u.u_lsize < stoc(1))
					/* room to expand */
					l = 1;
				else {			/* no room left */
					u.u_error = ENOMEM;
					return;
				}
				new = 1;
			} else {
				if (u.u_dsize + d > stoc(1)) {
					u.u_error = ENOMEM;
					return;
				}
				l = 0;
				new = 0;
			}
			goto bigger;
		} else /* (ap->arg.incr < 0) */ {
			/* sbrk less */
			d = btoc(-ap->arg.incr);
			goto smaller;
		}
	}
	return;


	/*
	 * expand the data space
	 *
	 * d = amount to expand data by (clicks)
	 * l = amount to expand ldt by  (clicks)
	 * new = whether expansion occurs in a new segment
	 * si = index of segment to start expansion at
	 */
bigger:
	if (chksize(u.u_tsize, u.u_dsize + d, u.u_ssize, u.u_lsize + l))
		return;
	p->p_lsize += l;	/* so adjustldt in expand will do right */
	expand(p->p_size + l + d);
	u.u_lsize = p->p_lsize;
	a = p->p_addr + p->p_size;
	n = u.u_ssize;
	while (n--) {
		a--;
		copyseg(a - d - l, a);
	}
	n = u.u_dsize;
	u.u_dsize += d;
	sf = d;
	while (sf--)
		clearseg(--a);
	if (l) {
		while (n--) {
			a--;
			copyseg(a-l, a);
		}
		while (l--)
			clearseg(--a);
	}

	/* now we fix up the LDT */
	sp = u.u_ldtadv + si;
	al = ((long)sp->sd_hibase << 16) + sp->sd_lowbase;
	if (new) {
		al += sp->sd_limit + 1L;
		sp++;
		l = 0;
	} else {
		l = btoc(sp->sd_limit);
	}
	while (d > 0) {
		if (d > stoc(1) - l) {
			sp->sd_limit = ctob((long)stoc(1)) - 1;
			n = stoc(1) - l;
		} else {
			sp->sd_limit = ctob((long)d+l) - 1;
			n = d;
		}
		sp->sd_hibase = lobyte(hiword(al));
		sp->sd_lowbase = loword(al);
		sp->sd_access = ACC_UDATA;
		d -= n;
		al += ctob((long)n);
		l = 0;
		sp++;
	}
	return;


	/*
	 * shrink the data space
	 *
	 * d = amount of space to release (clicks)
	 * si = index of last data segment
	 */
smaller:
	if (u.u_model & U_MOD_SSTACK) {
		if (d > u.u_dsize) {
			u.u_error = ENOMEM;
			return;
		}
	} else {
		if (ctob((long)d) > ctob((long)u.u_dsize) - (u.u_ossize + 1)) {
			u.u_error = ENOMEM;
			return;
		}
	}
	u.u_dsize -= d;
	a = p->p_addr + p->p_size - u.u_ssize;
	n = u.u_ssize;
	while (n--) {
		copyseg(a, a-d);
		a++;
	}
	expand(p->p_size - d);

	/* now we fix up the LDT */
	if (p->p_ssize) {	/* only fix stack if seperate */
		sp = u.u_ldtadv + STACK_SEL;
		al = ctob((long)p->p_addr + p->p_size - stoc(1));
		sp->sd_hibase = lobyte(hiword(al));
		sp->sd_lowbase = loword(al);
	}
	sp = u.u_ldtadv + si;
	while (d > 0) {
		if (d >= btoc(sp->sd_limit)) {
			sp->sd_access = 0;
			d -= btoc(sp->sd_limit);
			sp--;
		} else {
			sp->sd_limit -= ctob((long)d);
			d = 0;
		}
	}
}
