/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	@(#)	1.1	*/
typedef struct sty STYPE;
struct sty { ATYPE t; STYPE *next; };

typedef struct sym {
#ifdef FLEXNAMES
	char *name;
#else
	char name[LCHNM];
#endif
	char nargs;
	int decflag;
	int fline;
	STYPE symty;
	int fno;
	int mno;
	int use;
	short more;
	} STAB;
