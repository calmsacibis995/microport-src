/*      Copyright (c) 1985 AT&T */
/*        All Rights Reserved   */

/*      THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T     */
/*      The copyright notice above does not evidence any        */
/*      actual or intended publication of such source code.     */

/* @(#)trap.h	1.8 */
/*
 * Trap type values
 */

#define	DIVERR		0	/* divide error			*/
#define	SGLSTP		1	/* single step			*/
#define	NMIFLT		2	/* NMI				*/
#define	BPTFLT		3	/* breakpoint fault		*/
#define	INTOFLT		4	/* INTO overflow fault		*/
#define	BOUNDFLT	5	/* BOUND instruction fault	*/
#define	INVOPFLT	6	/* invalid opcode fault		*/
#define	NOEXTFLT	7	/* extension not available fault*/
#define	DBLFLT		8	/* double fault			*/
#define	EXTOVRFLT	9	/* extension overrun fault	*/
#define	BADTSSFLT	10	/* invalid TSS fault		*/
#define	NOSEGFLT	11	/* segment not present fault	*/
#define	STKFLT		12	/* stack fault			*/
#define	GPFLT		13	/* general protection fault	*/
#define	EXTERRFLT	16	/* extension error fault	*/
#define	SYSCALL		256	/* System call identifier	*/
#define	RESCHED		257	/* reschedule opportunity id	*/
/*
** WARNING: These vectors are dependent on the initialization
** of the PIC. If you change that, you better check if these
** are still ok.
*/
#define	CLKVEC		32	/* vector of the clock		*/
#define	INTERRUPT	33	/* front panel button		*/
