static char *uportid = "@(#)sys3.c	Microport Rev Id 1.3.3  6/18/86";
/*      Copyright (c) 1985 AT&T */
/*        All Rights Reserved   */

/*      THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T     */
/*      The copyright notice above does not evidence any        */
/*      actual or intended publication of such source code.     */

/* @(#)sys3.c	1.7 */
#include "sys/param.h"
#include "sys/types.h"
#include "sys/sysmacros.h"
#include "sys/systm.h"
#include "sys/mount.h"
#include "sys/ino.h"
#include "sys/buf.h"
#include "sys/filsys.h"
#include "sys/dir.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/inode.h"
#include "sys/file.h"
#include "sys/conf.h"
#include "sys/stat.h"
#include "sys/ttold.h"
#include "sys/var.h"
#include "sys/ioctl.h"
#include "sys/flock.h"

/*
** Union for use by all fcntl locking routines.
*/
union fcntl_arg
{
	struct flock	*sfparg;	/* ptr to flock struct */
	int		iarg;		/* integer */
};


/*
 * the fstat system call.
 */
fstat()
{
	register struct file *fp;
	register struct a {
		int	fdes;
		struct stat *sb;
	} *uap;

	uap = (struct a *)u.u_ap;
	fp = getf(uap->fdes);
	if(fp == NULL)
		return;
	stat1(fp->f_inode, uap->sb);
}

/*
 * the stat system call.
 */
stat()
{
	register struct inode *ip;
	register struct a {
		char	*fname;
		struct stat *sb;
	} *uap;

	uap = (struct a *)u.u_ap;
	ip = namei(uchar, 0);
	if(ip == NULL)
		return;
	stat1(ip, uap->sb);
	iput(ip);
}

/*
 * The basic routine for fstat and stat:
 * get the inode and pass appropriate parts back.
 */
stat1(ip, ub)
register struct inode *ip;
struct stat *ub;
{
	register struct dinode *dp;
	register struct buf *bp;
	struct stat ds;

	if(ip->i_flag&(IACC|IUPD|ICHG))
		iupdat(ip, &time, &time);
	/*
	 * first copy from inode table
	 */
	ds.st_dev = brdev(ip->i_dev);
	ds.st_ino = ip->i_number;
	ds.st_mode = ip->i_mode;
	ds.st_nlink = ip->i_nlink;
	ds.st_uid = ip->i_uid;
	ds.st_gid = ip->i_gid;
	ds.st_rdev = (dev_t)ip->i_rdev;
	ds.st_size = ip->i_size;
	/*
	 * next the dates in the disk
	 */
	bp = bread(ip->i_dev, FsITOD(ip->i_dev, ip->i_number));
	dp = bp->b_un.b_dino;
	dp += FsITOO(ip->i_dev, ip->i_number);
	ds.st_atime = dp->di_atime;
	ds.st_mtime = dp->di_mtime;
	ds.st_ctime = dp->di_ctime;
	brelse(bp);
	if (copyout((caddr_t)&ds, (caddr_t)ub, sizeof(ds)) < 0)
		u.u_error = EFAULT;
}

/*
 * the dup system call.
 */
dup()
{
	register struct file *fp;
	register i;
	register struct a {
		int	fdes;
	} *uap;

	uap = (struct a *)u.u_ap;
	fp = getf(uap->fdes);
	if(fp == NULL)
		return;
	if ((i = ufalloc(0)) < 0)
		return;
	u.u_ofile[i] = fp;
	fp->f_count++;
}

/*
 * the file control system call.
 */
fcntl()
{
	register struct file *fp;
	register struct a {
		int		fdes;
		int		cmd;
		union fcntl_arg	arg;
	} *uap;
	struct flock bf;
	register i;

	uap = (struct a *)u.u_ap;
	fp = getf(uap->fdes);
	if (fp == NULL)
		return;
	switch(uap->cmd) {
	case 0:
		i = uap->arg.iarg;
		if (i < 0 || i > NOFILE) {
			u.u_error = EINVAL;
			return;
		}
		if ((i = ufalloc(i)) < 0)
			return;
		u.u_ofile[i] = fp;
		fp->f_count++;
		break;

	case 1:
		u.u_rval1 = u.u_pofile[uap->fdes];
		break;

	case 2:
		u.u_pofile[uap->fdes] = uap->arg.iarg;
		break;

	case 3:
		u.u_rval1 = fp->f_flag+FOPEN;
		break;

	case 4:
		fp->f_flag &= (FREAD|FWRITE);
		fp->f_flag |= (uap->arg.iarg-FOPEN) & ~(FREAD|FWRITE);
		break;

	case 5:
		/* get record lock */
		if (copyin(uap->arg.sfparg, &bf, sizeof bf))
			u.u_error = EFAULT;
		else if ((i=getflck(fp, &bf)) != 0)
			u.u_error = i;
		else if (copyout(&bf, uap->arg.sfparg, sizeof bf))
			u.u_error = EFAULT;
		break;

	case 6:
		/* set record lock and return if blocked */
		if (copyin(uap->arg.sfparg, &bf, sizeof bf))
			u.u_error = EFAULT;
		else if ((i=setflck(fp, &bf, 0)) != 0)
			u.u_error = i;
		break;

	case 7:
		/* set record lock and wait if blocked */
		if (copyin(uap->arg.sfparg, &bf, sizeof bf))
			u.u_error = EFAULT;
		else if ((i=setflck(fp, &bf, 1)) != 0)
			u.u_error = i;
		break;

	default:
		u.u_error = EINVAL;
	}
}


/*
 * character special i/o control
 */
ioctl()
{
	register struct file *fp;
	register struct inode *ip;
	register struct a {
		int	fdes;
		int	cmd;
		union ioctl_arg	arg;
	} *uap;
	register dev_t dev;

	uap = (struct a *)u.u_ap;
	if ((fp = getf(uap->fdes)) == NULL)
		return;
	ip = fp->f_inode;
	if ((ip->i_mode&IFMT) != IFCHR) {
		u.u_error = ENOTTY;
		return;
	}
	dev = (dev_t)ip->i_rdev;
	(*cdevsw[major(dev)].d_ioctl)(minor(dev),uap->cmd,uap->arg,fp->f_flag);
}

/*
 * old stty and gtty
 */
stty()
{
	register struct a {
		int		fdes;
		struct sgttyb	*arg;
	} *uap;
	register struct o {
		int		fdes;
		int		cmd;
		union ioctl_arg	arg;
	} *uop;
	struct sgttyb	*save;

	uap = (struct a *)u.u_ap;
	uop = (struct o *)uap;
	save = uap->arg;
	uop->cmd = TIOCSETP;
	uop->arg.sparg = (struct Generic *)save;
	ioctl();
}

gtty()
{
	register struct a {
		int		fdes;
		struct sgttyb	*arg;
	} *uap;
	register struct o {
		int		fdes;
		int		cmd;
		union ioctl_arg	arg;
	} *uop;
	struct sgttyb	*save;

	uap = (struct a *)u.u_ap;
	uop = (struct o *)uap;
	save = uap->arg;
	uop->cmd = TIOCGETP;
	uop->arg.sparg = (struct Generic *)save;
	ioctl();
}

/*
 * Common code for mount and umount.
 * Checks block special argument and returns inode pointer.
 */
struct inode *
mgetdev()
{
	register dev_t dev;
	register struct inode *ip;

	ip = namei(uchar, 0);
	if (ip == NULL)
		return(ip);
	if ((ip->i_mode&IFMT) != IFBLK)
		u.u_error = ENOTBLK;
	else {
		dev = (dev_t)ip->i_rdev;
		if (bmajor(dev) >= bdevcnt)
			u.u_error = ENXIO;
	}
	if (u.u_error) {
		iput(ip);
		ip = NULL;
	}
	return(ip);
}

/*
 * the mount system call.
 */
smount()
{
	dev_t dev;
	register struct inode *ip;
	register struct mount *mp;
	struct mount *smp;
	register struct filsys *fp;
	struct inode *bip;
	register struct a {
		char	*fspec;
		char	*freg;
		int	ronly;
	} *uap;

	uap = (struct a *)u.u_ap;
	if(!suser())
		return;
	bip = mgetdev();
	if (u.u_error)
		return;
	dev = (dev_t)bip->i_rdev;
	u.u_dirp = (caddr_t)uap->freg;
	ip = namei(uchar, 0);
	if(ip == NULL) {
		iput(bip);
		return;
	}
	if ((ip->i_mode&IFMT) != IFDIR) {
		u.u_error = ENOTDIR;
		goto out;
	}
	if (ip->i_count != 1)
		goto out;
	if (ip->i_number == ROOTINO)
		goto out;
	smp = NULL;
	for(mp = &mount[0]; mp < (struct mount *)v.ve_mount; mp++) {
		if(mp->m_flags != MFREE) {
			if (brdev(dev) == brdev(mp->m_dev))
				goto out;
		} else
		if(smp == NULL)
			smp = mp;
	}
	mp = smp;
	if(mp == NULL)
		goto out;
	mp->m_flags = MINTER;
	mp->m_dev = brdev(dev);
#ifdef LCCFIX
	(*bdevsw[bmajor(dev)].d_open)(minor(dev), !uap->ronly, 1);
#else
	(*bdevsw[bmajor(dev)].d_open)(minor(dev), !uap->ronly);
#endif /* ! LCCFIX */
	if(u.u_error)
		goto out1;
	mp->m_bufp = geteblk();
	fp = mp->m_bufp->b_un.b_filsys;
	u.u_offset = SUPERBOFF;
	u.u_count = sizeof(struct filsys);
	u.u_base = (caddr_t)fp;
	u.u_segflg = 1;
	readi(bip);
	if (u.u_error)
		goto out2;
	fp->s_fmod = 0;
	fp->s_ilock = 0;
	fp->s_flock = 0;
	fp->s_ninode = 0;
	fp->s_inode[0] = 0;
	fp->s_ronly = uap->ronly & 1;
	if (fp->s_magic != FsMAGIC) {
		u.u_error = EINVAL;
		goto out2;
	}
	if (!fp->s_ronly) {
		if ((fp->s_state + (long)fp->s_time) == FsOKAY) {
			fp->s_state = FsACTIVE;
			u.u_offset = SUPERBOFF;
			u.u_count = sizeof(struct filsys);
			u.u_base = (caddr_t)fp;
			u.u_segflg = 1;
			u.u_fmode = FWRITE|FSYNC;
			writei(bip);
			if (u.u_error) {
				u.u_error = EROFS;
				goto out2;
			}
		} else {
			u.u_error = ENOSPC;
			goto out2;
		}
	}
	if (fp->s_type == Fs2b) {
		binval(mp->m_dev);
		mp->m_dev |= Fs2BLK;
	}
	if(brdev(pipedev) == brdev(mp->m_dev))
		pipedev = mp->m_dev;
	if (mp->m_mount = iget(mp->m_dev, ROOTINO))
		prele(mp->m_mount);
	else
		goto out2;
	mp->m_inodp = ip;
	mp->m_flags = MINUSE;
	ip->i_flag |= IMOUNT;
	iput(bip);
	prele(ip);
	return;

out2:
#ifdef LCCFIX
	(*bdevsw[bmajor(dev)].d_close)(minor(dev), !uap->ronly, 1);
#else
	(*bdevsw[bmajor(dev)].d_close)(minor(dev), !uap->ronly);
#endif /* ! LCCFIX */
	brelse(mp->m_bufp);
out1:
	mp->m_flags = MFREE;
out:
	iput(bip);
	if (u.u_error == 0)
		u.u_error = EBUSY;
	iput(ip);
}

/*
 * the umount system call.
 */
sumount()
{
	dev_t dev, odev;
	register struct inode *ip, *bip;
	register struct mount *mp;
	register struct filsys *fp;

	if(!suser())
		return;
	bip = mgetdev();
	if (u.u_error)
		return;
	dev = (dev_t)bip->i_rdev;
	for(mp = &mount[0]; mp < (struct mount *)v.ve_mount; mp++)
		if(mp->m_flags == MINUSE && brdev(dev) == brdev(mp->m_dev))
			goto found;
	iput(bip);
	u.u_error = EINVAL;
	return;

found:
	odev = dev;
	dev = mp->m_dev;
	xumount(dev);	/* remove unused sticky files from text table */
	if (mp->m_mount) {
		plock(mp->m_mount);
		iput(mp->m_mount);
		mp->m_mount = NULL;
		u.u_error = 0;
	}
	if (iflush(dev) < 0) {
		iput(bip);
		u.u_error = EBUSY;
		return;
	}
	/* at this point there should be no active files
	 * on the file system, the super block should not be locked.
	 * Break the connections.
	 */
	mp->m_flags = MINTER;
	ip = mp->m_inodp;
	ip->i_flag &= ~IMOUNT;
	plock(ip);
	iput(ip);
	fp = mp->m_bufp->b_un.b_filsys;
	if (!fp->s_ronly) {
		bflush(dev);
		fp->s_time = time;
		fp->s_state = FsOKAY - (long)fp->s_time;
		u.u_error = 0;
		u.u_offset = SUPERBOFF;
		u.u_count = sizeof(struct filsys);
		u.u_base = (caddr_t)fp;
		u.u_segflg = 1;
		u.u_fmode = FWRITE|FSYNC;
		writei(bip);
		u.u_error = 0;
	}
	iput(bip);
#ifdef LCCFIX
	(*bdevsw[bmajor(dev)].d_close)(minor(dev), 0, 1);
#else
	(*bdevsw[bmajor(dev)].d_close)(minor(dev), 0);
#endif /* ! LCCFIX */
	binval(dev);
	if (dev != odev)
		binval(odev);
	brelse(mp->m_bufp);
	mp->m_bufp = NULL;
	mp->m_flags = MFREE;
}

/*
 * mount root file system
 * iflg is 1 if called from iinit, 0 if from uadmin/remount
 */
srmount(iflg)
{
	register struct mount *mp = &mount[0];
	register struct buf *cp;
	register struct filsys *fp;
	struct inode iinode;

	if (iflg) {	/* initial call from iinit */
		cp = geteblk();
		mp->m_bufp = cp;
		fp = cp->b_un.b_filsys;
	} else {		/* for remount */
		cp = mp->m_bufp;
		fp = cp->b_un.b_filsys;
		if (fp->s_state == FsACTIVE) {
			u.u_error = EINVAL;
			return;
		}
	}
	iinode.i_mode = IFBLK;
	iinode.i_rdev = rootdev;
	u.u_error = 0;
	u.u_offset = SUPERBOFF;
	u.u_count = sizeof(struct filsys);
	u.u_base = (caddr_t)fp;
	u.u_segflg = 1;
	readi(&iinode);
	if(u.u_error)
		panic("cannot mount root");
	mp->m_flags = MINUSE;
	mp->m_dev = brdev(rootdev);
	fp->s_fmod = 0;
	fp->s_ilock = 0;
	fp->s_flock = 0;
	fp->s_ninode = 0;
	fp->s_inode[0] = 0;
	fp->s_ronly = 0;
	if (fp->s_magic != FsMAGIC)
		panic("not a valid root");
	if ((fp->s_state + (long)fp->s_time) == FsOKAY)
		fp->s_state = FsACTIVE;
	else
		fp->s_state = FsBAD;
	u.u_offset = SUPERBOFF;
	u.u_count = sizeof(struct filsys);
	u.u_base = (caddr_t)fp;
	u.u_segflg = 1;
	u.u_fmode = FWRITE|FSYNC;
	writei(&iinode);
	if (u.u_error) {
		fp->s_state = FsBAD;
		u.u_error = 0;
	}
	if (fp->s_type == Fs2b) {
		binval(mp->m_dev);
		mp->m_dev |= Fs2BLK;
	}
	rootdev = mp->m_dev;
	if(brdev(pipedev) == brdev(rootdev))
		pipedev = rootdev;
	if (iflg)
		clkset(fp->s_time);
}
