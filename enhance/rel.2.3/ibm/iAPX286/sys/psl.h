/* uportid = "@(#)psl.h	Microport Rev Id  1.3.3 6/18/86" */
/*      Copyright (c) 1985 AT&T */
/*        All Rights Reserved   */

/*      THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T     */
/*      The copyright notice above does not evidence any        */
/*      actual or intended publication of such source code.     */

/* @(#)psl.h	1.4 */
/*
** processor status ( flags register )
*/

#define	PS_C		0x0001		/* carry bit			*/
#define	PS_P		0x0004		/* parity bit			*/
#define	PS_AC		0x0010		/* auxiliary carry bit		*/
#define	PS_Z		0x0040		/* zero bit			*/
#define	PS_N		0x0080		/* negative bit			*/
#define	PS_T		0x0100		/* trace enable bit		*/
#define	PS_IE		0x0200		/* interrupt enable bit		*/
#define	PS_D		0x0400		/* direction bit		*/
#define	PS_V		0x0800		/* overflow bit			*/
#define	PS_IOPL		0x3000		/* I/O privilege level		*/
#define	PS_NT		0x4000		/* nested task flag		*/

/*
** Machine Status Word ( MSW )
*/
#define	MS_PE		0x0001		/* protected mode enable	*/
#define	MS_MP		0x0002		/* monitor processor extension	*/
#define	MS_EM		0x0004		/* emulate processor extension	*/
#define	MS_TS		0x0008		/* task switched		*/
