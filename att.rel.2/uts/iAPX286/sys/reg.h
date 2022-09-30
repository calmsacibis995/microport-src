/*      Copyright (c) 1985 AT&T */
/*        All Rights Reserved   */

/*      THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T     */
/*      The copyright notice above does not evidence any        */
/*      actual or intended publication of such source code.     */

/* @(#)reg.h	1.5 */
/*
 * Location of the users' stored registers relative
 * to base of stack frame
 * Usage is u.u_ar0[XX].
 */
#define	ES	0
#define	DS	1
#define	DI	2
#define	SI	3
#define	BP	4
#define	BX	6
#define	DX	7
#define	CX	8
#define	AX	9
#define	IP	19
#define	CS	20
#define	FLGS	21
#define	SP	22
#define	SS	23

/*
 * location of system call arguments and cs and ip as
 * of a system call
 */
#define	SCIP	10
#define	SCCS	11
#define	ARG0	12
#define	ARG1	13
#define	ARG2	14
#define	ARG3	15
#define	ARG4	16
#define	ARG5	17
#define	ARG6	18
#define	ARG7	19
#define	ARG8	20
#define	ARG9	21
