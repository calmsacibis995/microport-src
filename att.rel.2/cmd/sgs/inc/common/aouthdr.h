/*	Copyright (c) 1985 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


typedef	struct aouthdr {
	short	magic;		/* see magic.h				*/
	short	vstamp;		/* version stamp			*/
	long	tsize;		/* text size in bytes, padded to FW
				   bdry					*/
	long	dsize;		/* initialized data "  "		*/
	long	bsize;		/* uninitialized data "   "		*/
#if U3B
	long	dum1;
	long	dum2;		/* pad to entry point	*/
#endif
	long	entry;		/* entry pt.				*/
	long	text_start;	/* base of text used for this file	*/
	long	data_start;	/* base of data used for this file	*/
} AOUTHDR;
