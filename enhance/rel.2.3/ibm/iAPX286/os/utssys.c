static char *uportid = "@(#)utssys.c	Microport Rev Id 1.3.8  10/19/86";
/*      Copyright (c) 1985 AT&T */
/*        All Rights Reserved   */

/*      THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T     */
/*      The copyright notice above does not evidence any        */
/*      actual or intended publication of such source code.     */
/*
 * Modification History:
 *	Broke guts out of uadmin() to allow ctrl-alt-del to perform
 *		A_SHUTDOWN, AD_BOOT at any time
 */

/* @(#)utssys.c	1.5 */
#include "sys/types.h"
#include "sys/sysmacros.h"
#include "sys/param.h"
#include "sys/systm.h"
#include "sys/buf.h"
#include "sys/filsys.h"
#include "sys/mount.h"
#include "sys/dir.h"
#include "sys/errno.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/inode.h"
#include "sys/file.h"
#include "sys/proc.h"
#include "sys/var.h"
#include "sys/utsname.h"
#include "sys/uadmin.h"

utssys()
{
	register i;
	register struct a {
		char	*cbuf;
		int	mv;
		int	type;
	} *uap;
	struct {
		daddr_t	f_tfree;
		ino_t	f_tinode;
		char	f_fname[6];
		char	f_fpack[6];
	} ust;

	uap = (struct a *)u.u_ap;
	switch(uap->type) {

case 0:		/* uname */
	if (copyout(&utsname, uap->cbuf, sizeof(struct utsname)))
		u.u_error = EFAULT;
	return;

/* case 1 was umask */

case 2:		/* ustat */
	for(i=0; i<v.v_mount; i++) {
		register struct mount *mp;

		mp = &mount[i];
		if (mp->m_flags==MINUSE && brdev(mp->m_dev)==brdev(uap->mv)) {
			register struct filsys *fp;

			fp = mp->m_bufp->b_un.b_filsys;
			ust.f_tfree = FsLTOP(mp->m_dev, fp->s_tfree);
			ust.f_tinode = fp->s_tinode;
			bcopy(fp->s_fname, ust.f_fname, sizeof(ust.f_fname));
			bcopy(fp->s_fpack, ust.f_fpack, sizeof(ust.f_fpack));
			if (copyout(&ust, uap->cbuf, 18))
				u.u_error = EFAULT;
			return;
		}
	}
	u.u_error = EINVAL;
	return;

default:
	u.u_error = EFAULT;
	}
}

#ifdef IBMAT
int ualock = 0;		/* 0 - no lock, 1 - in call, 2 - ctrl-alt-del */
/*
 * administrivia system call
 */
uadmin()
{
	register struct a {
		int	cmd;
		int	fcn;
		int	mdep;
	} *uap;
	int	err;

	uap = (struct a *)u.u_ap;
	if (err = kadmin(uap->cmd, uap->fcn, uap->mdep))
		u.u_error = err;
}

/*
 * This is broken out so the atspec.c:kerndebug can use it.
 */

kadmin(cmd, fcn, mdep) 
{
	int	err = 0;
	struct mount *mp;
	struct filsys *fp;
	struct proc *p;
	struct inode iinode;
	extern hz;

	if ((ualock == 1) || ((ualock == 0) && !suser()))	
		return;
	ualock = 1;
	switch(cmd) {

	case A_SHUTDOWN:
		{
			p = &proc[2];

			for (; p < (struct proc *)v.ve_proc; p++) {
				if (p->p_stat == NULL)
					continue;
				if (p != u.u_procp)
					psignal(p, SIGKILL);
			}
		}
		delay(hz);	/* allow other procs to exit */
		{
			
			/* ctrl-alt-del umounts all */
			for(mp = &mount[0]; mp < (struct mount *) v.ve_mount; mp++){
				if(mp->m_flags == MFREE) 
					continue;
				/* I don't understand all the d-structs */
				if ((hiword(mp->m_bufp) == 0) || 
						(hiword(mp->m_bufp->b_un.b_filsys) == 0))
					continue;
				xumount(mp->m_dev);
				update();
				fp = mp->m_bufp->b_un.b_filsys;
				if (fp->s_state == FsACTIVE) {
					iinode.i_mode = IFBLK;
					iinode.i_rdev = mp->m_dev;
					fp->s_time = time;
					fp->s_state = FsOKAY - (long)fp->s_time;
					u.u_error = 0;
					u.u_offset = SUPERBOFF;
					u.u_count = sizeof(struct filsys);
					u.u_base = (caddr_t)fp;
					u.u_segflg = 1;
					u.u_fmode = FWRITE|FSYNC;
					writei(&iinode);
					u.u_error = 0;
				}
				bdwait();
				if (!mdep)
					break;
			}
		}
		delay(hz);	/* allow all I/O to finish */
	case A_REBOOT:
		mdboot(fcn, mdep);

		/* no return expected */
		break;

	case A_REMOUNT:
		{
			mp = &mount[0];
			/* remount root file system */
			iflush(mp->m_dev);
			binval(mp->m_dev);
			srmount(0);
		}
		break;

	default:
		err = EINVAL;
	}
	ualock = 0;
	return err;
}
#endif IBMAT
