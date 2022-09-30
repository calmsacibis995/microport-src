/*	Copyright (c) 1985 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#/*   @(#)gendefs.h	1.3 - 85/08/08 */
/* general definitions used through-out the assembler */

#define  LESS    -1
#define  EQUAL    0
#define  GREATER  1

#define  NO       0
#define  YES      1

#define NCPS	8	/* number of characters per symbol */
#define BITSPBY	8
#define BITSPOW	8
#define OUTWTYPE	char
#define	OUT(a,b)	putc(a,b)

#define NBPW	16

#define SCTALIGN 2L /* byte alignment for sections */
#define TXTFILL	0x90L
#define FILL 0L
#define NULLVAL 0L
#define NULLSYM ((symbol *)NULL)

/* constants used in testing and flag parsing */

#define TESTVAL	-2
#define NFILES	9

/* index of action routines in modes array */

#define NOACTION	0
#define	DEFINE	1
#define	SETVAL	2
#define	SETSCL	3
#define	SETTYP	4
#define	SETTAG	5
#define	SETLNO	6
#define	SETSIZ	7
#define	SETDIM1	8
#define	SETDIM2	9
#define	LLINENO	10
#define	LLINENUM	11
#define	LLINEVAL	12
#define	ENDEF	13
#define	NEWSTMT	14
#define	SETFILE	15
#define SETMAG	16
#define	RESABS	17
#define	RELPC8	18
#define	CJMPOPT	19
#define UJMPOPT	20
#define LOOPOPT	21
#define SWAPB	22
#define LONGREL	23
#define PACK8	24
#define PACK16	25
#define DUMPBITS	26
#define LOW8BITS	27
#define LO16BITS	28
#define HI12BITS	29
#define	SETFLAGS	30
#define NACTION	30
