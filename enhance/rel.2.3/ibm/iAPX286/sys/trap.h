/* uportid = "@(#)trap.h	Microport Rev Id  1.3.3 6/18/86" */
/*      Copyright (c) 1985 AT&T */
/*        All Rights Reserved   */

/*      THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T     */
/*      The copyright notice above does not evidence any        */
/*      actual or intended publication of such source code.     */

/* @(#)trap.h	1.8 */
/* 
* M000 lance 4-17-86
*		Changed EXTERRFLT to handle 287 slave pic interrupt
*/

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
#define	EXTERRFLT	45	/* extension error fault	M000 */
#define	SYSCALL		256	/* System call identifier	*/
#define	RESCHED		257	/* reschedule opportunity id	*/
/*
** WARNING: These vectors are dependent on the initialization
** of the PIC. If you change that, you better check if these
** are still ok.
*/
#ifdef ATMERGE
#define	CLKVEC		40	/* vector of the RT clock	*/
#else
#define	CLKVEC		32	/* vector of the clock		*/
#endif /* ATMERGE */

#ifndef	PICFIX1
#define	INTERRUPT	33	/* front panel button		*/
#endif /* ! PICFIX1 */ 
