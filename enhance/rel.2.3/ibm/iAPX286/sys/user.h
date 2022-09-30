/* uportid = "@(#)user.h	Microport Rev Id  1.3.3 6/18/86" */
/*      Copyright (c) 1985 AT&T */
/*        All Rights Reserved   */

/*      THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T     */
/*      The copyright notice above does not evidence any        */
/*      actual or intended publication of such source code.     */

/* @(#)user.h	1.19 - 85/08/09 */
#include "sys/tss.h"
/*
 * The user structure.
 * One allocated per process.
 * Contains all per process data that doesn't need to be referenced
 * while the process is swapped.
 * The user block is USIZE*click bytes long; resides at virtual kernel
 * loc 0x7ffff800(vax), 0140000(11/70), floating(11/34), sel 8(286);
 * contains the system stack per user; is cross referenced
 * with the proc structure for the same process.
 */
 
struct	user
{
	int	u_stack[KSTACKSZ];
					/* kernel stack per user
					 * extends from u + ctob(USIZE/2)
					 * backward not to reach here
					 */
	struct tss u_tss;		/* TSS - automatic save on switch */

	label_t	u_qsav;			/* label variable for quits and interrupts */
	label_t	u_ssav;			/* label variable for swapping */
	char	u_segflg;		/* IO flag: 0:user D; 1:system; 2:user I */
	char	u_error;		/* return error code */
	ushort	u_uid;			/* effective user id */
	ushort	u_gid;			/* effective group id */
	ushort	u_ruid;			/* real user id */
	ushort	u_rgid;			/* real group id */
	struct proc *u_procp;		/* pointer to proc structure */
	int	*u_ap;			/* pointer to arglist */
	union {				/* syscall return values */
		struct	{
			int	r_val1;
			int	r_val2;
		}r_reg;
		off_t	r_off;
		time_t	r_time;
		char	*r_ptr;
	} u_r;
	caddr_t	u_base;			/* base address for IO */
	unsigned u_count;		/* bytes remaining for IO */
	off_t	u_offset;		/* offset in file for IO */
	short	u_fmode;		/* file mode for IO */
	ushort	u_pbsize;		/* bytes in block for IO */
	ushort	u_pboff;		/* offset in block for IO */
	dev_t	u_pbdev;		/* real device for IO */
	daddr_t	u_rablock;		/* read ahead block addr */
	short	u_errcnt;		/* syscall error count */
	struct inode *u_cdir;		/* current directory of process */
	struct inode *u_rdir;		/* root directory of process */
	caddr_t	u_dirp;			/* pathname pointer */
	struct direct u_dent;		/* current directory entry */
	struct inode *u_pdir;		/* inode of parent directory of dirp */
	struct file *u_ofile[NOFILE];	/* pointers to file structures of open files */
	char	u_pofile[NOFILE];	/* per-process flags of open files */
	unsigned u_tsize;		/* text size (clicks) */
	unsigned u_dsize;		/* data size (clicks) */
	unsigned u_ssize;		/* stack size (clicks) */
	unsigned u_lsize;		/* LDT size (clicks) */
	struct seg_desc *u_ldtadv;	/* LDT alias descriptor virtual addr */
	int	(*u_signal[NSIG])();	/* disposition of signals */
	time_t	u_utime;		/* this process user time */
	time_t	u_stime;		/* this process system time */
	time_t	u_cutime;		/* sum of childs' utimes */
	time_t	u_cstime;		/* sum of childs' stimes */
	int	*u_ar0;			/* address of users saved R0 */
	struct {			/* profile arguments */
		short	*pr_base;	/* buffer base */
		unsigned pr_size;	/* buffer size */
		caddr_t	pr_off;		/* pc offset */
		unsigned pr_scale;	/* pc scaling */
	} u_prof;
	char	u_intflg;		/* catch intr from sys */
	char	u_sep;			/* flag for I and D separation */
	short	*u_ttyp;		/* pointer to pgrp in "tty" struct */
	dev_t	u_ttyd;			/* controlling tty dev */
	struct exdata {		/* info from header of executable file */
		long	ux_tsize;	/* size of text (bytes) */
		long	ux_dsize;	/* size of data (bytes) */
		long	ux_bsize;	/* size of bss (bytes) */
		long	ux_ssize;	/* size of stack (bytes) */
		long	ux_lsize;	/* size of ldt (bytes) */
		ushort	ux_cs;		/* entry point CS */
		ushort	ux_ip;		/* entry point IP */
		long	ux_fstoff;	/* file offset of FST */
		ushort	ux_flags;	/* contains model & other magic */
		ushort	ux_ldtend;	/* number of fst entries in file */
	} u_exdata;
	ushort	u_ossize;		/* fixed stack size (in bytes) */
	char	u_model;		/* memory model flags */
	char	u_comm[DIRSIZ];
	time_t	u_start;
	time_t	u_ticks;
	long	u_mem;
	long	u_ior;
	long	u_iow;
	long	u_iosw;
	long	u_ioch;
	char	u_acflag;
	short	u_cmask;		/* mask for file creation */
	daddr_t	u_limit;		/* maximum write address */
	short	u_lock;			/* process/text locking flags */
/* floating point stuff */
	char	u_fpvalid;		/* flag if saved state is valid	*/
	struct	fp_state		/* saved state			*/
	{
		char	edata[146];	/* 146 bytes needed to save 287 state */
		char	emuldata[274];	/* scratch data needed by fp emul     */
	} u_fpstate;
};

extern struct user u;

#define	u_rval1	u_r.r_reg.r_val1
#define	u_rval2	u_r.r_reg.r_val2
#define	u_roff	u_r.r_off
#define	u_rtime	u_r.r_time
#define	u_rptr	u_r.r_ptr

/* ioflag values: Read/Write, User/Kernel, Ins/Data */
#define	U_WUD	0
#define	U_RUD	1
#define	U_WKD	2
#define	U_RKD	3
#define	U_WUI	4
#define	U_RUI	5

#define	EXCLOSE	01

/* Memory model flag definitions. */
#define	U_MOD_HARRAY	0x01	/* huge arrays allowed */
#define	U_MOD_MDATA	0x02	/* multiple data segments allowed */
#define	U_MOD_MTEXT	0x04	/* multiple text segments allowed */
#define	U_MOD_SSTACK	0x08	/* stack is separate from data segment */
#define U_MOD_ISCOFF    0x80    /* set if COFF file, clear if STL file */

/* Memory models. */
#define U_MOD_MODEL     0x0F    /* model flag field mask */
#define	U_MOD_SMALL	0
#define	U_MOD_COMPACT	(U_MOD_SSTACK | U_MOD_MDATA)
#define	U_MOD_MIDDLE	U_MOD_MTEXT
#define	U_MOD_LARGE	(U_MOD_MDATA | U_MOD_MTEXT | U_MOD_SSTACK)
#define	U_MOD_HUGE	(U_MOD_LARGE | U_MOD_HARRAY)

/* u_intflg bit definitions. */
#define	TR_OFF		0x01	/* tracing turned off by trap */
