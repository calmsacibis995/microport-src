/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	@(#)	1.1	*/
/*
 *	Include file for assembler system call libraries.
 */

/*
 *	Parameter offsets - depend on model in use as 
 *	return link is either 1 or 2 words
 */

#if MIDDLE_M | LARGE_M | HUGE_M

	/*
	 *	| parameter N |
	 *	|-------------|
	 *	.	      .
	 *	.	      .
	 *	|-------------|
	 *	| parameter 2 |
	 *	|-------------|
	 *	| parameter 1 |
	 *	|-------------| <----- BASE
	 *	| return CS   |
	 *	|-------------|
	 *	| return IP   |
	 *	|-------------|
	 *	| old BP      |
	 *	|-------------| <----- %bp
	 *
	 */

#define BASE	4

#else

	/*
	 *	| parameter N |
	 *	|-------------|
	 *	.	      .
	 *	.	      .
	 *	|-------------|
	 *	| parameter 2 |
	 *	|-------------|
	 *	| parameter 1 |
	 *	|-------------| <----- BASE
	 *	| return IP   |
	 *	|-------------|
	 *	| old BP      |
	 *	|-------------| <----- %bp
	 *
	 */

#define	BASE 2

#endif

#define PARAM(x)	[BASE + [2\*x]](%bp)

#if MIDDLE_M | LARGE_M | HUGE_M
#define	CALL	lcall
#define	JMP	ljmp
#define LVRET	leave ; lret
#define	RET	lret
#else
#define	CALL	call
#define	JMP	jmp
#define LVRET	leave ; ret
#define	RET	ret
#endif

/
/	Definitions of Kernel Entry Call Gates
/

#include	"sys/mmu.h"
#define	SIGCALL		[SIGSEL << 3] , 0
#define	SYSCALL		[SCALL_SEL << 3] , 0

/
/	Definitions of System Call Entry Point Numbers
/

#define	ACCESS	33
#define	ALARM	27
#define	BRK	17
#define	CHDIR	12
#define	CHMOD	15
#define	CHOWN	16
#define	CHROOT	61
#define	CLOSE	6
#define	CREAT	8
#define	DUP	41
#define	EXEC	11
#define	EXECE	59
#define	EXIT	1
#define	FCNTL	62
#define	FORK	2
#define	FSTAT	28
#define	GETGID	47
#define	GETPID	20
#define	GETUID	24
#define	GTTY	32
#define	IOCTL	54
#define	KILL	37
#define	LINK	9
#define	LOCK	45
#define	MKNOD	14
#define	MOUNT	21
#define	MSGSYS	49
#define	NICE	34
#define	OPEN	5
#define	PAUSE	29
#define	PIPE	42
#define	PROF	44
#define	PTRACE	26
#define	READ	3
#define	SEEK	19
#define	SEMSYS	53
#define	SETGID	46
#define	SETPGRP	39
#define	SETUID	23
#define	SHMSYS	52
#define	SIGNAL	48
#define	STAT	18
#define	STIME	25
#define	STTY	31
#define	SYNC	36
#define	SYSACCT	51
#define	TIME	13
#define	TIMES	43
#define UADMIN	55
#define	ULIMIT	63
#define	UMASK	60
#define	UMOUNT	22
#define	UNLINK	10
#define	UTIME	30
#define	UTSSYS	57
#define	WAIT	7
#define	WRITE	4
