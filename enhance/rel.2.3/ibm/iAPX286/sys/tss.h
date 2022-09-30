/* uportid = "@(#)tss.h	Microport Rev Id  1.3.3 6/18/86" */
/*      Copyright (c) 1985 AT&T */
/*        All Rights Reserved   */

/*      THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T     */
/*      The copyright notice above does not evidence any        */
/*      actual or intended publication of such source code.     */

/* @(#)tss.h	1.3 */
/*
 * Task State Segment structure
 */

struct tss {
	unsigned ts_back;	/* selector of nested TSS descriptor	*/
	unsigned ts_sp0;	/* initial stack for PL0		*/
	unsigned ts_ss0;
	unsigned ts_sp1;	/* initial stack for PL1		*/
	unsigned ts_ss1;
	unsigned ts_sp2;	/* initial stack for PL2		*/
	unsigned ts_ss2;
	unsigned ts_ip;		/* saved registers			*/
	unsigned ts_flags;
	unsigned ts_ax;
	unsigned ts_cx;
	unsigned ts_dx;
	unsigned ts_bx;
	unsigned ts_sp;
	unsigned ts_bp;
	unsigned ts_si;
	unsigned ts_di;
	unsigned ts_es;
	unsigned ts_cs;
	unsigned ts_ss;
	unsigned ts_ds;
	unsigned ts_ldt;	/* selector of LDT descriptor		*/
};
