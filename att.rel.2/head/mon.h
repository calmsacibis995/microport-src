/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	@(#)	1.1	*/
struct hdr {
#ifdef iAPX286
	unsigned long	lpc;
	unsigned long	hpc;
#else
	char	*lpc;
	char	*hpc;
#endif
	int	nfns;
#ifdef iAPX286
	unsigned stack,stacke;
#endif
};

struct cnt {
#ifdef iAPX286
	unsigned long	fnpc;
#else
	char	*fnpc;
#endif
	long	mcnt;
};

typedef unsigned short WORD;

#define MON_OUT	"mon.out"
#define MPROGS0	(150 * sizeof(WORD))	/* 300 for pdp11, 600 for 32-bits */
#define MSCALE0	4
#ifdef iAPX286
#ifndef NULL
#define NULL	0l
#endif
#define INDEX(x) ((( (unsigned long)(x) >> 3)& 0xffff0000L)|(unsigned)(x))
#else
#define NULL	0
#endif
