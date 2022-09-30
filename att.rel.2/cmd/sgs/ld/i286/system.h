/*	Copyright (c) 1985 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#/*   @(#)system.h	1.3 - 85/08/09 */

#define COMMON		1	/* allows multiply def'd externs*/
#if iAPX286
#define USEREGIONS	0	/* disallow REGIONS directives	*/
#define	TRVEC		0	/* disallows transfer vectors	*/
#else
#define	TRVEC		1	/* allows transfer vectors	*/
#define	USEREGIONS	1	/* allows allocation by regions */
#endif
#define	UNIX		1	/* ld used as standard UNIX tool*/
#define	DMERT		0	/* ld used to generate DMERT obj*/
#if !iAPX286
#define	IAPX		1	/* ld generates iAPX objects	*/
#endif
#define	B16		0	/* ld generates basic-16 object	*/
#define X86		0	/* ld generates extended 8086	*/
#define	N3B		0	/* ld generates 3B-20 object	*/
#define	M32		0	/* ld generates BELLMAC-32 obj	*/
#define U3B 		0	/* ld generates 3B-20Simplex obj*/
#define	VAX		0	/* ld generates VAX 11/780 obj	*/
#define	PDP		0	/* ld generates PDP 11/70  obj	*/
#define	DEC		0	/* ld generates DEC object	*/
#define	IANDD		0	/* allows separate i and d	*/
#define SWABFMT		0	/* text in reverse order 	*/
#define ILDOPT		0	/* allows .i_l_dxx sections	*/

#ifndef TS
#define	TS		1	/* ld executed under UNIX-TS	*/
#endif

#ifndef RT
#define	RT		0	/* ld executed under UNIX-RT	*/
#endif
