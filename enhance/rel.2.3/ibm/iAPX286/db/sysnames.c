/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident "@(#)sysnames.c	1.1"

/*
 * Table of syscall names
 */


#ifdef GDEBUGGER
char	*sysnames[] =
{
	"nosys",			/*  0 = indir */
	"exit",				/*  1 = exit */
	"fork",				/*  2 = fork */
	"read",				/*  3 = read */
	"write",			/*  4 = write */
	"open",				/*  5 = open */
	"close",			/*  6 = close */
	"wait",				/*  7 = wait */
	"creat",			/*  8 = creat */
	"link",				/*  9 = link */
	"unlink",			/* 10 = unlink */
	"exec",				/* 11 = exec */
	"chdir",			/* 12 = chdir */
	"time",				/* 13 = time */
	"mknod",			/* 14 = mknod */
	"chmod",			/* 15 = chmod */
	"chown",			/* 16 = chown; now 3 args */
	"break",			/* 17 = break */
	"stat",				/* 18 = stat */
	"seek",				/* 19 = seek */
	"getpid",			/* 20 = getpid */
	"mount",			/* 21 = mount */
	"umount",			/* 22 = umount */
	"setuid",			/* 23 = setuid */
	"getuid",			/* 24 = getuid */
	"stime",			/* 25 = stime */
	"ptrace",			/* 26 = ptrace */
	"alarm",			/* 27 = alarm */
	"fstat",			/* 28 = fstat */
	"pause",			/* 29 = pause */
	"utime",			/* 30 = utime */
	"stty",				/* 31 = stty */
	"gtty",				/* 32 = gtty */
	"access",			/* 33 = access */
	"nice",				/* 34 = nice */
	"statfs",			/* 35 = statfs */
	"sync",				/* 36 = sync */
	"kill",				/* 37 = kill */
	"fstatfs",			/* 38 = fstatfs */
	"setpgrp",			/* 39 = setpgrp */
	"nosys",			/* 40 = tell - obsolete */
	"dup",				/* 41 = dup */
	"pipe",				/* 42 = pipe */
	"times",			/* 43 = times */
	"prof",				/* 44 = prof */
	"lock",				/* 45 = proc lock */
	"setgid",			/* 46 = setgid */
	"getgid",			/* 47 = getgid */
	"sig",				/* 48 = sig */
	"msgsys",			/* 49 = IPC message */
	"sys3b",			/* 50 = 3b2-specific system call */
	"sysacct",			/* 51 = turn acct off/on */
	"shmsys",                   	/* 52 = shared memory */
	"semsys",			/* 53 = IPC semaphores */
	"ioctl",			/* 54 = ioctl */
	"uadmin",			/* 55 = uadmin */
#ifdef MEGA
 	"uexch",			/* 56 = uexch */
#else
	"nosys",			/* 56 = reserved for exch */
#endif /* MEGA */
	"utssys",			/* 57 = utssys */
	"nosys",			/* 58 = reserved for USG */
	"exece",			/* 59 = exece */
	"umask",			/* 60 = umask */
	"chroot",			/* 61 = chroot */
	"fcntl",			/* 62 = fcntl */
	"ulimit",			/* 63 = ulimit */

/*
 * This table is the switch used to transfer
 * to the appropriate routine for processing a vmunix special system call.
 * Each row contains the number of arguments expected
 * and a pointer to the routine.
 */

/* The following 6 entries were reserved for Safari 4 */
	"nosys",			/* 64 +0 = nosys */
	"nosys",			/* 64 +1 = nosys */
	"nosys",			/* 64 +2 = nosys */
	"nosys",			/* 64 +3 = file locking call */
	"nosys",			/* 64 +4 = local system calls */
	"nosys",			/* 64 +5 = inode open */
	"advfs",			/* 70 = advfs */
	"unadvfs",			/* 71 = unadvfs */
	"rmount",			/* 72 = rmount */
	"rumount",			/* 73 = rumount */
	"rfstart",			/* 74 = rfstart */
	"nosys",			/* 75 = not used */
	"rdebug",			/* 76 = rdebug */
	"rfstop",			/* 77 = rfstop */
	"rfsys",			/* 78 = rfsys */
	"rmdir",			/* 79 = rmdir */
	"mkdir",			/* 80 = mkdir */
	"getdents",			/* 81 = getdents */
	"nosys",			/* 82 = not used */
	"nosys",			/* 83 = not used */
	"sysfs",			/* 84 = sysfs */
	"getmsg",			/* 85 = getmsg */
	"putmsg",			/* 86 = putmsg */
	"poll",				/* 87 = poll */
};
#endif /* GDEBUGGER */
