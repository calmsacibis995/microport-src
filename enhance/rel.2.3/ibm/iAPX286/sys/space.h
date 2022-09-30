/* uportid = "@(#)space.h	Microport Rev Id  1.3.3 6/18/86" */
/*      Copyright (c) 1985 AT&T */
/*        All Rights Reserved   */

/*      THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T     */
/*      The copyright notice above does not evidence any        */
/*      actual or intended publication of such source code.     */

/* @(#)space.h	1.19 */

/* Modification History:
 * M000 lance Norskog 2-20-86
 * 		removed cfree, buf, and hbuf arrays for dynamic allocation
 * M001 uport!rex 4/21/87
 *		changed wnstat to hdstat for 386 compatibility
 */

#define KERNEL
#include "sys/acct.h"
struct  acct    acctbuf;
struct  inode   *acctp;

#include "sys/tty.h"
struct  cblock  *cfree; 				/* M000 */

#include "sys/buf.h"
struct  buf     bfreelist;      /* head of the free list of buffers */
struct  pfree   pfreelist;      /* Head of physio header pool */
struct  buf     pbuf[NPBUF];    /* Physical io header pool */

struct  hbuf    *hbuf; 		/* buffer hash table M000 */

#include "sys/file.h"
struct  file    file[NFILE];    /* file table */

#include "sys/inode.h"
struct  inode   inode[NINODE];  /* inode table */

#include "sys/proc.h"
struct  proc    proc[NPROC];    /* process table */
struct  proc    *fp_proc;       /* process that is using 80287  */
char            fp_kind;        /* 'kind' of 80287 ( emulated or not )  */

#include "sys/text.h"
struct  text text[NTEXT];       /* text table */

#include "sys/map.h"
struct map swapmap[SMAPSIZ] = {mapdata(SMAPSIZ)};
struct map coremap[CMAPSIZ] = {mapdata(CMAPSIZ)};

#include "sys/callo.h"
struct callo callout[NCALL];

#include "sys/mount.h"
struct mount mount[NMOUNT];

#include "sys/elog.h"
#include "sys/err.h"
struct  err     err = {
        NESLOT,
};

#include "sys/sysinfo.h"
struct sysinfo sysinfo;
struct syswait syswait;
struct syserr syserr;

#include "sys/opt.h"

#include "sys/var.h"
struct var v = {
        NBUF,
        NCALL,
        NINODE,
        (char *)(&inode[NINODE]),
        NFILE,
        (char *)(&file[NFILE]),
        NMOUNT,
        (char *)(&mount[NMOUNT]),
        NPROC,
        (char *)(&proc[1]),
        NTEXT,
        (char *)(&text[NTEXT]),
        NCLIST,
        NSABUF,
        MAXUP,
        SMAPSIZ,
        NHBUF,
        NHBUF-1,
        NPBUF,
        CMAPSIZ,
        AUTOUP
};

#include "sys/init.h"

#ifdef SXT_0
#include "sys/sxt.h"
#define LINKSIZE (sizeof(struct Link) + sizeof(struct Channel)*(MAXPCHAN-1))
char sxt_buf[SXT_0 * LINKSIZE];
#else
char *sxt_buf = 0;      /* so a kernel can link-edit w/out sxt  */
#endif

#ifndef PRF_0
prfintr() {}
#endif
unsigned        prfstat;

#ifndef EMUL_0
emul_init() {}
emul_entry() {}
emul_present() { return( 0 ); }
#else
emul_present() { return( 1 ); }
#endif

#ifndef SIO_0
long sioscan() { return( 0L ); }
#endif

#ifdef  VP_0
#include "sys/vp.h"
#endif

struct iotime hdstat[4];		/* M001 */
struct iotime fpstat[4];

#ifdef  TRACE_0
#include "sys/trace.h"
struct trace trace[TRACE_0];
#endif

#ifdef  CSI_0
#define MAXKMC 16
#include "sys/csi.h"
#include "sys/csihdw.h"
struct csi csi_csi[CSI_0];
int csibnum = CSIBNUM;
struct csibuf *csibpt[CSIBNUM];
#ifndef DMK_0
unsigned short kmc_dmk[MAXKMC];
#endif
#endif

#ifdef  VPM_0
#include "sys/vpmt.h"
struct vpmt vpmt[VPM_0];
struct csibd vpmtbd[VPM_0*(XBQMAX + EBQMAX)];
struct csibuf vpmtbmt =
        { { {0, }, }, 0, 0, 0,VPMNEXUS, };
struct vpminfo vpminfo =
        {XBQMAX, EBQMAX, VPM_0*(XBQMAX + EBQMAX)};
int vpmbsz= VPMBSZ;
#endif

#ifdef  DMK_0
#define MAXKMC 16
#define MAXDMK 16
#include        "sys/dmk.h"
struct dmksave dmksave[MAXDMK];
unsigned short kmc_dmk[MAXKMC];
#endif

#ifdef  X25_0
#include "sys/x25.h"
struct x25slot x25slot[X25_0];
struct x25tab x25tab[X25_0];
struct x25timer x25timer[X25_0];
struct x25link x25link[X25LINKS];
struct x25timer *x25thead[X25LINKS];
struct x25lntimer x25lntimer[X25LINKS];
struct csibd x25bd[X25BUFS];
struct csibuf x25buf=
        { { {0, }, }, 0, 0, 0,X25NEXUS, };
struct x25info x25info =
        {X25_0, X25_0, X25LINKS, X25BUFS, X25BYTES};
#endif

#ifdef BX25S_0
#include "sys/bx25.space.h"
#endif

#ifdef PCL11B_0
#include "sys/pcl.h"
#endif

#if MESG==1
#include        "sys/ipc.h"
#include        "sys/msg.h"

struct map      msgmap[MSGMAP];
struct msqid_ds msgque[MSGMNI];
struct msg      msgh[MSGTQL];
struct msginfo  msginfo = {
        MSGMAP,
        MSGMAX,
        MSGMNB,
        MSGMNI,
        MSGSSZ,
        MSGTQL,
        MSGSEG
};
#else
msgsys()
{
        nosys();
}
#endif

#if SEMA==1
#       ifndef IPC_ALLOC
#       include "sys/ipc.h"
#       endif
#include        "sys/sem.h"
struct semid_ds sema[SEMMNI];
struct sem      sem[SEMMNS];
struct map      semmap[SEMMAP];
struct  sem_undo        *sem_undo[NPROC];
#define SEMUSZ  (sizeof(struct sem_undo) + sizeof(struct undo) * (SEMUME - 1))
int     semu[((SEMUSZ*SEMMNU)+NBPW-1)/NBPW];
union {
        short           semvals[SEMMSL];
        struct semid_ds ds;
        struct sembuf   semops[SEMOPM];
}       semtmp;

struct  seminfo seminfo = {
        SEMMAP,
        SEMMNI,
        SEMMNS,
        SEMMNU,
        SEMMSL,
        SEMOPM,
        SEMUME,
        SEMUSZ,
        SEMVMX,
        SEMAEM
};
#else
semsys()
{
        nosys();
}
#endif

#if SHMEM==1
#       ifndef  IPC_ALLOC
#       include "sys/ipc.h"
#       endif
#include        "sys/shm.h"
struct shmid_ds *shm_shmem[NPROC*SHMSEG];
struct shmid_ds shmem[SHMMNI];  
int             shm_seln[NPROC * SHMSEG];
struct  shminfo shminfo = {
        SHMMAX,
        SHMMIN,
        SHMMNI,
        SHMSEG,
        SHMBRK,
        SHMALL
};
#else
shmsys()
{
        nosys();
}
#endif
#ifdef  NSC_0
#include "sys/nscdev.h"
#endif
#ifdef ST_0
#include "sys/st.h"
struct stbhdr   stihdrb[STIHBUF];
struct stbhdr   stohdrb[STOHBUF];
struct ststat   ststat = {
        STIBSZ, /* input buffer size */
        STOBSZ, /* output buffer size */
        STIHBUF,        /* # of buffer headers */
        STOHBUF,        /* # of buffer headers */
        STNPRNT /* # of printer channels */
};
struct csibuf   stibuf =
        { { {0, }, }, 0, 0, 0,STNEXUS, };
struct csibuf   stobuf =
        { { {0, }, }, 0, 0, 0,STNEXUS, };
#endif

#ifdef EM_0
#include <sys/em.h>
#ifndef EMBHDR
#define EMBHDR  (EM_0 * 6)      /* Default Buffer Headers */
#endif
struct csibd    embd[EMBHDR];   /* Buffer Descriptors */
int embhdr = EMBHDR;            /* No. of Buffer Headers */
int emtbsz = EMTBSZ;            /* Transmit Buffer Space */
int emrbsz = EMRBSZ;            /* Receive Buffer Space */
int emrcvsz = EMRCVSZ;          /* Receive Buffer Size */
struct csibuf embmt =
        { { {0, }, }, 0, 0, 0,EMNEXUS, };
#endif

/* file and record locking */
#include "sys/flock.h"
struct flckinfo flckinfo = {
        FLCKREC,
        FLCKFIL,
        0,
        0,
        0,
        0
} ;

struct filock flox[FLCKREC];            /* lock structures */
struct flino flinotab[FLCKFIL]; /* inode to lock assoc. structures */

