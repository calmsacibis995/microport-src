static char *uportid = "@(#)shm.c	Microport Rev Id 1.3.3  6/18/86";
/*      Copyright (c) 1985 AT&T */
/*        All Rights Reserved   */

/*      THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T     */
/*      The copyright notice above does not evidence any        */
/*      actual or intended publication of such source code.     */

#undef	SHMDEBUG

/* @(#)shm.c	1.9 */
#include "sys/types.h"
#include "sys/param.h"
#include "sys/sysmacros.h"
#include "sys/dir.h"
#include "sys/errno.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/seg.h"
#include "sys/ipc.h"
#include "sys/shm.h"
#include "sys/proc.h"
#include "sys/systm.h"
#include "sys/mmu.h"
#include "sys/map.h"

extern struct shmid_ds	shmem[];	/* shared memory headers */
extern struct shmid_ds	*shm_shmem[];	/* ptrs to attached segments */
extern int		shm_seln[];	/* segment attach points */
extern struct shminfo	shminfo;	/* shared memory info structure */
int	shmtot;		/* total shared memory currently used */

extern time_t		time;			/* system idea of date */

struct	shmid_ds	*ipcget(),
			*shmconv();

/*
**	shmat - Shmat system call.
*/

shmat()
{
	register struct a {
		int	shmid;
		char	*addr;
		int	flag;
	}	*uap = (struct a *)u.u_ap;
	register int			seln;	/* LDT selector number */
	register struct shmid_ds	*sp;	/* shared memory header ptr */
	register struct seg_desc	*lp;	/* ptr into LDT */
	register int	shmn;
	register int	i;
	register int			j;	/* loop control */
	paddr_t				paddr;	/* segment physical address */

	if((sp = shmconv(uap->shmid, SHM_DEST)) == NULL)
		return;
	if(ipcaccess(&sp->shm_perm, SHM_R))
		return;
	if((uap->flag & SHM_RDONLY) == 0)
		if(ipcaccess(&sp->shm_perm, SHM_W))
			return;
	for(shmn = 0;shmn < shminfo.shmseg;shmn++)
		if(shm_shmem[(u.u_procp - proc)*shminfo.shmseg+shmn] == NULL)
			break;
	if(shmn >= shminfo.shmseg) {
		u.u_error = EMFILE;
		return;
	}

	/* Adjust shmn to be actual index into shm_shmem and shm_seln. */
	shmn += (u.u_procp - proc) * shminfo.shmseg;

	/* Round user supplied address down to segment boundary, if
		requested by user. */
	if(uap->flag & SHM_RND)
		uap->addr = (char *)((long)uap->addr & ~(SHMLBA - 1));

	/* Check for legitimate attach address. */
	if(uap->addr != NULL)
		if((i = ston(hiword(uap->addr))) <= CODE1_SEL + 1 ||
			uap->addr != lstouv(i & 0xff)) {
			u.u_error = EINVAL;
			return;
		}

/*
 * An address of 0 places the shared memory into a first fit location
 */
	if (uap->addr == NULL) {

		/* Calculate current end of allocated LDT. */
		i = u.u_lsize * (ctob(1) / sizeof(struct seg_desc));

		/* If shared memory allocation has been checked before,
			look for available free slot around that point. */
		if(seln = u.u_procp->p_smend) {
			lp = u.u_ldtadv + seln;
			for(;seln < i;seln++, lp++)
				if(lp->sd_access == 0)
					break;
			if(seln >= i)

				/* We didn't find anything, reset to look for
					new spot. */
				seln = 0;
		}

		/* If we haven't found a usable selector and shared memory is
			currently attached, allocate a selector in the next
			LDT click. */
		if(seln == 0 && u.u_procp->p_smbeg) {
			 u.u_procp->p_smend = seln = i - shminfo.shmseg +
				(ctob(1) / sizeof(struct seg_desc));
			lp = u.u_ldtadv + seln;
		}

		/* If we still haven't found a usable selector, perform initial
			check for a selector. */
		if(seln == 0) {

			/* Calculate # of open segments required and see if
				that many are available. */
			for(j = shminfo.shmseg + shminfo.shmbrk,
				lp = u.u_ldtadv + j;j;j--, lp--)
				if(lp->sd_access)
					break;

			/* Adjust new LDT size by a click and shrink amount
				required until it fits. */
			while(j > 0) {
				i += ctob(1) / sizeof(struct seg_desc);
				j -= ctob(1) / sizeof(struct seg_desc);
			}

			/* Set up assigned selector and save it for later
				attaches. */
			u.u_procp->p_smend = seln = i - shminfo.shmseg;
			lp = u.u_ldtadv + seln;
		}
	} else {
/*
 * Check to make sure segment does not overlay any valid segments
 */
		seln = ston(hiword(uap->addr));
		lp = u.u_ldtadv + seln;
		if((uint)(btoc((long)seln * sizeof(struct seg_desc)) <=
			u.u_lsize) && lp->sd_access) {
			u.u_error = ENOMEM;
			return;
		}
	}

	/* Grow the LDT, if necessary. */
	/* Calculate # of additional LDT clicks needed. */
	i = (int)((btoc((long)seln * sizeof(struct seg_desc))) - u.u_lsize);
	if(i > 0) {
	register int	clicks;	/* # of clicks to copy */

		/* Verify that process can grow. */
		if(chksize(u.u_tsize, u.u_dsize, u.u_ssize, u.u_lsize + i))
			return;

		/* Expand the process. */
		u.u_procp->p_lsize += i;
		expand(u.u_procp->p_size + i);
		u.u_lsize += i;

		/* Reassemble the process image. */
		/* Set up physical click # of end of process and
			# of clicks to move. */
		j = u.u_procp->p_addr + u.u_procp->p_size;

		/* u.u_tisize is not checked here because we don't have any
			non-shared text. */
		clicks = u.u_dsize + u.u_ssize;
		while(clicks--) {

			/* Move the text, data, and stack. */
			j--;
			copyseg(j - i, j);
		}

		/* Clear the newly allocated LDT entries. */
		while(i--)
			clearseg(--j);
	}

	/* Set up the segment in the LDT. */
	paddr = ctob((long)sp->shm_segpcc);
	lp->sd_lowbase = loword(paddr);
	lp->sd_hibase = (unsigned char)hiword(paddr);
	lp->sd_limit = sp->shm_segsz - 1;
	lp->sd_access = (uap->flag & SHM_RDONLY) ? ACC_UCODE : ACC_UDATA;
#ifdef	SHMDEBUG
	printf("SHMAT: Paddr = %lx, limit = %x, access = %x, selector = %d\n",
		paddr, lp->sd_limit, lp->sd_access, seln);
#endif	SHMDEBUG
/*
 * Clear segment on first attach
 */
	if (sp->shm_perm.mode & SHM_CLEAR) {
		i = (int)btoc((long)sp->shm_segsz);
		while(--i >= 0)
			clearseg(sp->shm_segpcc + i);
		sp->shm_perm.mode &= ~SHM_CLEAR;
	}
	shm_shmem[shmn] = sp;
	shm_seln[shmn] = seln;
	if(u.u_procp->p_smbeg == 0 || u.u_procp->p_smbeg > seln)
		u.u_procp->p_smbeg = seln;
	sp->shm_nattch++;
	sp->shm_cnattch++;
	u.u_rptr = lstouv(seln);
	sp->shm_atime = time;
	sp->shm_lpid = u.u_procp->p_pid;
}

/*
**	shmconv - Convert user supplied shmid into a ptr to the associated
**		shared memory header.
*/

struct shmid_ds *
shmconv(s, flg)
register int	s;	/* shmid */
int		flg;	/* error if matching bits are set in mode */
{
	register struct shmid_ds	*sp;	/* ptr to associated header */

	sp = &shmem[s % shminfo.shmmni];
	if(!(sp->shm_perm.mode & IPC_ALLOC) || sp->shm_perm.mode & flg
		|| s / shminfo.shmmni != sp->shm_perm.seq) {
		u.u_error = EINVAL;
		return(NULL);
	}
	return(sp);
}

/*
**	shmctl - Shmctl system call.
*/

shmctl()
{
	register struct a {
		int		shmid,
				cmd;
		struct shmid_ds	*arg;
	}	*uap = (struct a *)u.u_ap;
	register struct shmid_ds	*sp;	/* shared memory header ptr */
	struct shmid_ds			ds;	/* hold area for IPC_SET */

	if((sp = shmconv(uap->shmid, (uap->cmd == IPC_STAT) ? 0 : SHM_DEST)) ==
		NULL)
		return;
	u.u_rval1 = 0;
	switch(uap->cmd) {

	/* Remove shared memory identifier. */
	case IPC_RMID:
		if(u.u_uid != sp->shm_perm.uid && u.u_uid != sp->shm_perm.cuid
			&& !suser())
			return;
		sp->shm_ctime = time;
		sp->shm_perm.mode |= SHM_DEST;

		/* Change key to private so old key can be reused without
			waiting for last detach.  Only allowed accesses to
			this segment now are shmdt() and shmctl(IPC_STAT).
			All others will give bad shmid. */
		sp->shm_perm.key = IPC_PRIVATE;

		/* Adjust counts to counter shmfree decrements. */
		sp->shm_nattch++;
		sp->shm_cnattch++;
		shmfree(sp);
		return;

	/* Set ownership and permissions. */
	case IPC_SET:
		if(u.u_uid != sp->shm_perm.uid && u.u_uid != sp->shm_perm.cuid
			&& !suser())
			return;
		if(copyin(uap->arg, &ds, sizeof(ds))) {
			u.u_error = EFAULT;
			return;
		}
		sp->shm_perm.uid = ds.shm_perm.uid;
		sp->shm_perm.gid = ds.shm_perm.gid;
		sp->shm_perm.mode = (ds.shm_perm.mode & 0777) |
			(sp->shm_perm.mode & ~0777);
		sp->shm_ctime = time;
		return;

	/* Get shared memory data structure. */
	case IPC_STAT:
		if(ipcaccess(&sp->shm_perm, SHM_R))
			return;
		if(copyout(sp, uap->arg, sizeof(*sp)))
			u.u_error = EFAULT;
		return;

	default:
		u.u_error = EINVAL;
		return;
	}
}

/*
**	shmdt - Shmdt system call.
*/

shmdt()
{
	struct a {
		char	*addr;
	}	*uap = (struct a *)u.u_ap;
	register struct shmid_ds	*sp, **spp;
	register int			*ap;	/* ptr to attach point data */
	register i, j;
	register struct proc *p;
	register struct seg_desc	*lp;	/* ptr to LDT entry */

	/* Find the segment attached at the address supplied by the user. */
	ap = &shm_seln[((p = u.u_procp) - proc) * shminfo.shmseg];
	for(i = 0;i < shminfo.shmseg;i++, ap++)
		if(uap->addr == lstouv(*ap))
			break;
	if(i >= shminfo.shmseg) {
		u.u_error = EINVAL;
		return;
	}

	/* Clear the LDT entry for the segment. */
	lp = &u.u_ldtadv[*ap];
	lp->sd_limit = 0;
	lp->sd_lowbase = 0;
	lp->sd_hibase = 0;
	lp->sd_access = 0;

	/* Free the segment, update time and pid, clear the shm_shmem
		and shm_seln entries for the segment, and reset the
		shmem beginning and ending fields in the proc table. */
	spp = &shm_shmem[ap - shm_seln];
	sp = *spp;
	shmfree(sp);
	sp->shm_dtime = time;
	sp->shm_lpid = p->p_pid;
	*spp = NULL;
	*ap = 0;
	p->p_smbeg = 0;
	ap = &shm_seln[(p - proc) * shminfo.shmseg];
	for(j = 0;j < shminfo.shmseg;j++, ap++) {
		if(i = *ap) {
			if (p->p_smbeg) {
				if (p->p_smbeg > i)
					p->p_smbeg = i;
			} else {
				p->p_smbeg =  i;
			}
		}
	}
	u.u_rval1 = 0;
}

/*
**	shmexec - Called by setregs(os/sys1.c) to handle shared memory exec
**		processing.
*/

shmexec()
{
	register struct shmid_ds	**spp;	/* ptr to ptr to header */
	register int			*sppp;	/* ptr to attach point data */
	register int			i;	/* loop control */

	if (u.u_procp->p_smbeg == 0)
		return;
	/* Detach any attached segments. */
	sppp = &shm_seln[i = (u.u_procp - proc)*shminfo.shmseg];
	u.u_procp->p_smbeg = 0;
	u.u_procp->p_smend = 0;
	spp = &shm_shmem[i];
	for(i = 0; i < shminfo.shmseg; i++,spp++,sppp++) {
		if(*spp == NULL)
			continue;
		shmfree(*spp);
		*spp = NULL;
		*sppp = 0;
	}
}

/*
**	shmexit - Called by exit(os/sys1.c) to clean up on process exit.
*/

shmexit()
{
	/* Same processing as for exec. */
	shmexec();
}

/*
**	shmfork - Called by newproc(os/slp.c) to handle shared memory fork
**		processing.
*/

shmfork(cp, pp)
struct proc	*cp,	/* ptr to child proc table entry */
		*pp;	/* ptr to parent proc table entry */
{
	register struct shmid_ds	**cpp,	/* ptr to child shmem ptrs */
					**ppp;	/* ptr to parent shmem ptrs */
	register int			*cppp,	/* ptr to child attach pts */
					*pppp;	/* ptr to parent attach pts */
	register int			i;	/* loop control */

	if (pp->p_smbeg == 0)
		return;
	/* Copy ptrs and update counts on any attached segments. */
	cpp = &shm_shmem[(cp - proc)*shminfo.shmseg];
	ppp = &shm_shmem[(pp - proc)*shminfo.shmseg];
	cppp = &shm_seln[(cp - proc)*shminfo.shmseg];
	pppp = &shm_seln[(pp - proc)*shminfo.shmseg];
	cp->p_smbeg = pp->p_smbeg;
	cp->p_smend = pp->p_smend;
	for(i = 0;i < shminfo.shmseg; i++, cpp++, ppp++, cppp++, pppp++) {
		if(*cpp = *ppp) {
			(*cpp)->shm_nattch++;
			(*cpp)->shm_cnattch++;
			*cppp = *pppp;
		}
	}
}

/*
**	shmfree - Decrement counts.  Free segment, if indicated.
*/

shmfree(sp)
register struct shmid_ds	*sp;	/* shared memory header ptr */
{
	register int	size;		/* clicks to be freed */

	if(sp == NULL)
		return;
	sp->shm_nattch--;
	if(--(sp->shm_cnattch) == 0 && sp->shm_perm.mode & SHM_DEST) {
		if (!(sp->shm_perm.mode & SHM_PHYS)) {
			mfree(coremap, size = btoc((long)sp->shm_segsz),
				sp->shm_segpcc);

			/* adjust maxmem for amount freed */
			maxmem += size;
			shmtot -= size;
#ifdef	SHMDEBUG
			printf("SHMfree: Paddr = %lx mfree'd\n", ctob(sp->shm_segpcc));
#endif	SHMDEBUG
			}
#ifdef	SHMDEBUG
		else printf("SHMfree: Paddr = %lx not mfree'd\n", ctob(sp->shm_segpcc));
#endif	SHMDEBUG
		sp->shm_perm.mode = 0;
		if(((int)(++(sp->shm_perm.seq)*shminfo.shmmni + (sp - shmem))) < 0)
			sp->shm_perm.seq = 0;
	}
}

/*
**	shmget - Shmget system call.
*/

shmget()
{
	register struct a {
		key_t	key;
		uint	size;
		int	shmflg;
		long	shmphys;
	}	*uap = (struct a *)u.u_ap;
	register struct shmid_ds	*sp;	/* shared memory header ptr */
	int				s;	/* ipcget status */
	int				size;	/* clicks needed */

#ifdef	SHMDEBUG
	printf("SHMGET:Enter\n");
#endif	SHMDEBUG
	if((sp = ipcget(uap->key, uap->shmflg, shmem, shminfo.shmmni, sizeof(*sp),
		&s)) == NULL)
		return;
	if(s) {
#ifdef	SHMDEBUG
		printf("SHMGET:New segment flag %x\n", uap->shmflg);
#endif	SHMDEBUG

		/* This is a new shared memory segment.  Allocate memory and
			finish initialization. */
		if(uap->size < shminfo.shmmin || uap->size > shminfo.shmmax) {
			u.u_error = EINVAL;
			sp->shm_perm.mode = 0;
			return;
		}
		size = btoc((long)uap->size);
		if (uap->shmflg & IPC_SHMPHYS) {
			if (!suser()) {
				u.u_error = EPERM;
				sp->shm_perm.mode = 0;
				return;
				}
#ifdef	SHMDEBUG
			printf("SHMGET:Physaddr %lx, %d\n",uap->shmphys,btoc(uap->shmphys));
#endif	SHMDEBUG
			sp->shm_segsz = uap->size;
			sp->shm_segpcc = btoc(uap->shmphys);
			sp->shm_perm.mode &= ~SHM_CLEAR;	/* Don't clear existing area! */
			sp->shm_perm.mode |= SHM_PHYS;		/* Don't mfree() it on shmdt! */
		} else {
			if (shmtot + size > shminfo.shmall) {
				u.u_error = ENOMEM;
				sp->shm_perm.mode = 0;
				return;
			}
			sp->shm_segsz = uap->size;
			sp->shm_segpcc = malloc(coremap, size);
			sp->shm_perm.mode |= SHM_CLEAR;
			if(sp->shm_segpcc == NULL) {
				u.u_error = ENOMEM;
				sp->shm_perm.mode = 0;
				return;
			}

			/* Adjust maxmem for the segment. */
			maxmem -= size;
			shmtot += size;
			}

		sp->shm_nattch = sp->shm_cnattch = 0;
		sp->shm_atime = sp->shm_dtime = 0;
		sp->shm_ctime = time;
		sp->shm_lpid = 0;
		sp->shm_cpid = u.u_procp->p_pid;
	} else
		if(uap->size && uap->size > sp->shm_segsz) {
			u.u_error = EINVAL;
			return;
		}
	u.u_rval1 = sp->shm_perm.seq * shminfo.shmmni + (sp - shmem);
#ifdef	SHMDEBUG
	printf("SHMGET:Exit\n");
#endif	SHMDEBUG
}

/*
**	shmsys - System entry point for shmat, shmctl, shmdt, and shmget
**		system calls.
*/

shmsys()
{
	register struct a {
		uint	id;
	}	*uap = (struct a *)u.u_ap;
	int		shmat(),
			shmctl(),
			shmdt(),
			shmget();
	static int	(*calls[])() = {shmat, shmctl, shmdt, shmget};

	if(uap->id > 3) {
		u.u_error = EINVAL;
		return;
	}
	u.u_ap++;
	(*calls[uap->id])();
}
