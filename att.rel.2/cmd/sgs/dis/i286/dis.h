/*	Copyright (c) 1985 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#/*   @(#)dis.h	1.3 - 85/08/09 */
/*
 */

/*	Modified March 84 to add 186 instructions; jws */

/*
 *	This is the header file for the iapx disassembler.
 *	The information contained in the first part of the file
 *	is common to each version, while the last part is dependent
 *	on the particular machine architecture being used.
 */

#define		NCPS	8	/* number of chars per symbol	*/
#define		NHEX	40	/* max # chars in object per line	*/
#define		NLINE	36	/* max # chars in mnemonic per line	*/
#define		FAIL	0
#define		TRUE	1
#define		FALSE	0
#define		LEAD	1
#define		NOLEAD	0
#define		TERM 0		/* used in _tbls.c to indicate		*/
				/* that the 'indirect' field of the	*/
				/* 'instable' terminates - no pointer.	*/
				/* Is also checked in 'dis_text()' in	*/
				/* _bits.c.				*/

#ifdef AR32W
#define	LNNOBLKSZ	1024	/* size of blocks of line numbers		*/
#define	SYMBLKSZ	1024	/* size if blocks of symbol table entries	*/
#else
#define	LNNOBLKSZ	512	/* size of blocks of line numbers		*/
#define	SYMBLKSZ	512	/* size of blocks of symbol table entries	*/
#endif
#define		STRNGEQ 0	/* used in string compare operation	*/

/*
 *	This is the structure that will be used for storing all the
 *	op code information.  The structure values themselves are
 *	in '_tbls.c'.
 */

struct	instable {
	char		name[NCPS];
	struct instable *indirect;	/* for decode op codes */
	unsigned	adr_mode;
};

/*	NOTE:	the following information in this file must be changed
 *		between the different versions of the disassembler.
 *
 *	This structure is used to determine the displacements and registers
 *	used in the addressing modes.  The values are in 'tables.c'.
 */
struct addr {
	int	disp;
	char	regs[9];
};
/*
 *	These are the instruction formats as they appear in
 *	'tables.c'.  Here they are given numerical values
 *	for use in the actual disassembly of an object file.
 */
#define UNKNOWN	0
#define RMMR	1
#define MRw	2
#define IMlw	3
#define IMw	4
#define IR	5
#define OA	6
#define AO	7
#define MS	8
#define SM	9
#define Mv	10
#define Mw	11
#define M	12
#define R	13
#define RA	14
#define SEG	15
#define MR	16
#define IA	17
#define MA	18
#define SD	19
#define AD	20
#define SA	21
#define D	22
#define INM	23
#define SO	24
#define BD	25
#define I	26
#define P	27
#define V	28
#define Iv	29
#define U	30
#define OVERRIDE 31
#define GO_ON	32
#define	O	33	/* for call	*/
#define JTAB	34	/* jump table 	*/
#define RMMRI	35	/* for 186 iimul instr  */
#define MvI	37	/* for 186 logicals */
#define	II	38	/* for 186 enter instr  */
#define RMw	39	/* for 286 arpl instr */
#define Ib	40	/* for push immediate byte */
#define	F	41	/* for 287 instructions */
#define	FF	42	/* for 287 instructions */
#define	FILL	0x90	/* Fill byte used for alignment (nop)	*/
