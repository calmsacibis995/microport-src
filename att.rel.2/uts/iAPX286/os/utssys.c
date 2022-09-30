/*      Copyright (c) 1985 AT&T */
/*        All Rights Reserved   */

/*      THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T     */
/*      The copyright notice above does not evidence any        */
/*      actual or intended publication of such source code.     */

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
	static ualock;

	if (ualock || !suser())
		return;
	ualock = 1;
	uap = (struct a *)u.u_ap;
	switch(uap->cmd) {

	case A_SHUTDOWN:
		{
			register struct proc *p = &proc[2];

			for (; p < (struct proc *)v.ve_proc; p++) {
				if (p->p_stat == NULL)
					continue;
				if (p != u.u_procp)
					psignal(p, SIGKILL);
			}
		}
		delay(HZ);	/* allow other procs to exit */
		{
			register struct mount *mp = &mount[0];
			register struct filsys *fp;
			struct inode iinode;

			xumount(mp->m_dev);
			update();
			fp = mp->m_bufp->b_un.b_filsys;
			if (fp->s_state == FsACTIVE) {
				iinode.i_mode = IFBLK;
				iinode.i_rdev = rootdev;
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
		}
	case A_REBOOT:
		mdboot(uap->fcn, uap->mdep);

		/* no return expected */
		break;

	case A_REMOUNT:
		{
			register struct mount *mp = &mount[0];
	
			/* remount root file system */
			iflush(mp->m_dev);
			binval(mp->m_dev);
			srmount(0);
		}
		break;

	default:
		u.u_error = EINVAL;
	}
	ualock = 0;
}
