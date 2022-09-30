/*      Copyright (c) 1985 AT&T */
/*        All Rights Reserved   */

/*      THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T     */
/*      The copyright notice above does not evidence any        */
/*      actual or intended publication of such source code.     */

/* @(#)shm.h	1.5 */
/*
**	IPC Shared Memory Facility.
*/

/*
**	Implementation Constants.
*/

#define	SHMLBA	ctob((long)stoc(1))	/* segment boundary address multiple */
			/* (SHMLBA must be a power of 2) */

/*
**	Permission Definitions.
*/

#define	SHM_R	0400	/* read permission */
#define	SHM_W	0200	/* write permission */

/*
**	ipc_perm Mode Definitions.
*/

#define	SHM_CLEAR	01000	/* clear segment on next attach */
#define	SHM_DEST	02000	/* destroy segment when # attached = 0 */

/*
**	Message Operation Flags.
*/

#define	SHM_RDONLY	010000	/* attach read-only (else read-write) */
#define	SHM_RND		020000	/* round attach address to SHMLBA */

/*
**	Structure Definitions.
*/

/*
**	There is a shared mem id data structure for each segment in the system.
*/

struct shmid_ds {
	struct ipc_perm	shm_perm;	/* operation permission struct */
	uint		shm_segsz;	/* segment size */
	uint		shm_segpcc;	/* segment 1st physical core click */
	ushort		shm_lpid;	/* pid of last shmop */
	ushort		shm_cpid;	/* pid of creator */
	ushort		shm_nattch;	/* current # attached */
	ushort		shm_cnattch;	/* in memory # attached */
	time_t		shm_atime;	/* last shmat time */
	time_t		shm_dtime;	/* last shmdt time */
	time_t		shm_ctime;	/* last change time */
};

struct	shminfo {
	uint	shmmax;	/* max shared memory segment size */
	int	shmmin,	/* min shared memory segment size */
		shmmni,	/* # of shared memory identifiers */
		shmseg,	/* max attached shared memory segments per process */
		shmbrk,	/* gap (in clicks) used between data and shared memory */
		shmall;	/* max total shared memory system wide (in clicks) */
};
