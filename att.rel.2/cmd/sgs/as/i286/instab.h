/*	Copyright (c) 1985 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#/*   @(#)instab.h	1.3 - 85/08/08 */
/* common definitions for the generation of instructions */
/* NOTE: symbols.h must be included before this file */

#define DBITON	0x0200
#define DBITOFF	0
#define SEGPFX	0x26

#define BX	0x07
#define BP	0x06
#define SI	0x04
#define DI	0x05
#define REGMOD	0x03
#define NODISP	0x00
#define DISP8	0x01
#define DISP16	0x02
#define EA16	0x06

#define EXADMD	0x01	/* external address mode */
#define EDSPMD	0x02	/* displacement mode with expression */
#define DSPMD	0x04	/* displacement mode */
#define IMMD	0x08	/* 16 bit immediate mode */
#define EXPRMASK	0x03	/* mask for presence of an expression */

#define AREG16MD	0x10	/* accumulator (16 bit) mode */
#define AREG8MD		0x20	/* accumulator (8 bit) mode */
#define REG16MD		0x40	/* register (16 bit) mode */
#define REG8MD		0x80	/* register (8 bit) mode */
#define	AREGMASK	0x30
#define REGMASK		0xf0

#define LO8TYPE		0x10
#define LO16TYPE	0x20
#define HI12TYPE	0x40
#define X86TYPE		0x70

#define INSTRB	1
#define INSTRW	2
#define RINST	3
#define SINST	4
#define EINST	5
#define AINST	6
#define IAINST	7
#define ININST	8
#define GREG16	9
#define	GREG8	10
#define SEGREG	11
#define PSEUDO	12

#define AREGOPCD	0

union addrmdtag {
	struct addrtag {
#if	AR16WR || AR32WR
		unsigned rm : 3;
		unsigned reg : 3;
		unsigned mod : 2;
		unsigned dummy : 8;
#else
		unsigned dummy : 8;
		unsigned mod : 2;
		unsigned reg : 3;
		unsigned rm : 3;
#endif
	} addrmd;
	unsigned short addrmdfld;
};

typedef struct {
	short exptype;
	symbol *symptr;
	long expval;
} rexpr;

typedef struct {
	BYTE admode;
	BYTE adreg;
	rexpr adexpr;
} addrmode;

#if iAPX286
/*
 * 	80287 decalrations
 */

#define FSTACK 0x33

/*
 *	type to return float/double/temp long/llong and bcd in
 */

typedef union {
		unsigned short	fvala[5] ;
		unsigned long	fvall ;
		float		fvalf ;
		double		fvald ;
	} floatval ;

#endif

