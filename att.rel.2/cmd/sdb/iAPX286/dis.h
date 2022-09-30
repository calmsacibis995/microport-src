/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	@(#)	1.1	*/

/*
 *	This is the header file for the disassemblers (n3b and VAX).
 *	The information contained in the first part of the file
 *	is common to each version, while the last part is dependent
 *	on the particular machine architecture being used.
 */

#if !iAPX286
#define		NCPS	8	/* number of chars per symbol	*/
#define		NHEX	80	/* max # chars in object per line	*/
#define		NLINE	80	/* max # chars in mnemonic per line	*/
#define		FAIL	0
#define		TRUE	1
#define		FALSE	0
#endif

#ifdef u3b
#define		LEAD	1
#define		NOLEAD	0
#define		TERM	0	/* used in _tbls.c to indicate		*/
				/* that the 'indirect' field of the	*/
				/* 'instable' terminates - no pointer.	*/
				/* Is also checked in 'dis_text()' in	*/
				/* _bits.c.				*/

#define		STRNGEQ 0	/* used in string compare operation	*/
				/* in 'disassembly' routine		*/
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
 *		between the different versions of the n3b disassembler.
 *
 *	These are the instruction formats as they appear in
 *	'n3bdis_tbls.c'.  Here they are given numerical values
 *	for use in the actual disassembly of an object file.
 */

#define UNKNOWN		0
#define DYA		1	/* dyadic				*/
#define LNGIO3R		2	/* long I/O with 3 regs			*/
#define LNGIO4R		3	/* long I/O with 4 regs			*/
#define LNGIO2RL	4	/* long I/O, 2 regs, one in last 4 bits	*/
#define LNGIO2S		5	/* long I/O, 2 regs, one special(rmchr)	*/
#define LNGIO2R		6	/* long I/O with 2 regs			*/
#define LNGIOS		7	/* special long I/O--append i,r or blank*/
#define LNGIO4S		8	/* same as above but 4 regs		*/
#define LNG2R		9	/* long inst with 2 regs		*/
#define LNG3R		10	/* long inst with 3 regs		*/
#define SHTR		11	/* short inst, 1 reg, no decode		*/
#define TRI		12	/* triadic				*/
#define JMB		13	/* jump on bit clear/set		*/
#define GEND		14	/* single general mode operand, decode field */
#define BR		15	/* branch				*/
#define RR		16	/* dyadic opt, reg to reg		*/
#define NR		17	/* dyadic opt, nib to reg		*/
#define RN		18	/* dyadic opt, reg to nib		*/
#define MON1		19	/* monadic, type 1			*/
#define MON2		20	/* monadic, type 2			*/
#define LCI4		21	/* loop control, 4 operands		*/
#define LCI3		22	/* loop control, 3 operands		*/
#define I4		23	/* 4 bit immediate operand		*/
#define I8		24	/* 8 bit immediate operand		*/
#define MARC		25	/* move address/read & clear		*/
#define BITF1		26	/* bit field				*/
#define GO_ON		27	/* go on -- no operands			*/
#define D2G		28	/* 2 gen addr mode ops w/ decode	*/
#define CALL		29	/* call					*/
#define RSR		30	/* read special register		*/
#define WSR		31	/* write special register		*/
#define	SDYA		32	/* special dyadic case			*/
#define	BITF2		33	/* second bit field case		*/
#define	TRI2		34	/* second type of triadic		*/
#define	CALL1		35	/* second type of call			*/
#define	CALL2		36	/* third type of call			*/
#define	BP		37	/* for breakpointable hald		*/
#define	GENDW		38	/* single gen mode operand, decode	*/
				/* field, word sized operand assumed	*/
#define	CALE		39	/* call to emulation instruction	*/
#define	RETE		40	/* return to emulation instruction	*/
#define	RDSER		41	/* read store error register instruction */
#define TUSRET		42	/* special return for TUS trap		*/
#define CALL3		43	/* tv call				*/
#define FPD2G		44	/* floating point dyadic instructions	*/
#define FPTRI		45	/* floating point triadic instructions	*/
/*
 *	The following four items indicate in the n3bds_tbls.c  and
 *	n3bds_bits.c files what bits from the byte following the
 *	op code are necessary for decoding.
 */

#define	H4	1	/* high four bits   xxxx----	*/
#define	B54	3	/* bits 5 & 4       --xx----	*/
#define	B654	4	/* bits 6, 5 & 4    -xxx----	*/
#define L4	5	/* low 4 bits       ----xxxx	*/


/*
 *	A 4 bit descriptor to indicate the addressing modes may indicate
 *	a register, a nibble or one of the general addressing modes.
 *	These values are used to initialize tables in 'n3bds_tbls.c' and in
 *	case statements in 'n3bds_bits.c'.
 */

#define	REG	0
#define NIB	1
#define	GEN	2


/*
 *	An operand may be a byte, halfword, fullword or double in length.
 *	Again, these values are used in 'n3bds_tbls.c' and 'n3bds_bits.c'.
 */

#define	BYTE	0
#define	HALF	1
#define	WORD	2
#define DOUBLE  3	/* used for (64 bit) floating point immediate operands */
#else
#if !iAPX286

#define VARNO 36	/* Max number of operands saved in VAX instruction */
#define PCNO 15		/* Number of PC register			   */
/*
 * Argument access types
 */
#define ACCA	(8<<3)	/* address only */
#define ACCR	(1<<3)	/* read */
#define ACCW	(2<<3)	/* write */
#define ACCM	(3<<3)	/* modify */
#define ACCB	(4<<3)	/* branch displacement */
#define ACCI	(5<<3)	/* XFC code */

/*
 * Argument data types
 */
#define TYPB	0	/* byte */
#define TYPW	1	/* word */
#define TYPL	2	/* long */
#define TYPQ	3	/* quad */
#define TYPF	4	/* floating */
#define TYPD	5	/* double floating */


struct optab {
	char *iname;
	char val;
	char nargs;
	unsigned char argtype[6];
};

extern char * 	regname[];
extern char *	fltimm[];

#define LOBYTE	0xff
#else
/* define the number of arguments for the instruction decode */
#define VARNO  6
/* define the instruction table format */
struct optab {
	char *iname;
	char iformat;
	char iflag;
};
/* define the symbolic override settings				*/
#define NONE	10	/* depends on context bp is ss etc		*/
#define ESO	0	/* es overide					*/
#define CSO	1	/* cs overide 					*/
#define SSO	2	/* ss overide					*/
#define DSO	3	/* ds overide					*/
/* define the decode type settings for symbolic decode			*/
#define NILL	0	/* no symbolic representation possible		*/
#define EXTERN  1	/* external data address			*/
#define LABEL	2	/* code label					*/
#define SPARAM  3	/* parameter variable address is positive	*/
#define APNO	SPARAM
#define SLOCAL  4	/* local variable address is negative		*/
#define LABELL	5	/* full segment displacement defined		*/
#define FPNO 	SLOCAL
/* define the decode mappings */
#define SDBAX    0      /* 16 bit acc					*/
#define SDBCX 	 1	/* CX						*/
#define SDBDX	 2	/* DX						*/
#define SDBBX	 3	/* BX						*/
#define SDBSP	 4	/* SP						*/
#define SDBBP	 5	/* BP						*/
#define SDBSI	 6	/* SI						*/
#define SDBDI	 7      /* DI						*/
#define SDBAL	 8	/* AL						*/
#define SDBST    8      /* floating point operand %st			*/
#define SDBCL    9	/* CL						*/
#define SDBDL   10      /* DL						*/
#define SDBBL	11	/* BL						*/
#define SDBAH	12	/* AH						*/
#define SDBCH	13	/* CH						*/
#define SDBDH	14	/* DH						*/
#define SDBBH	15	/* BH						*/
#define SDBXSD  16	/* EA = (BX) + (SI) + DISP			*/
#define SDBXDD	17	/* EA = (BX) + (DI) + DISP			*/
#define SDBPID  18	/* EA = (BP) + (SI) + DISP			*/
#define SDBPDD	19	/* EA = (BP) + (DI) + DISP			*/
#define SDBSD	20	/* EA = (SI) + DISP				*/
#define SDBDD	21	/* EA = (DI) + DISP				*/
#define SDBPD   22	/* EA = (BP) + DISP				*/
#define SDBXD   23      /* EA = (BX) + DISP				*/
#define SDBD    24	/* EA = DISP					*/
#define SDBI    25      /* immediate operand				*/
#define SDB2    26      /* 2 words of operand				*/
#define SDBN    27	/* no arguments					*/
#define SDBES   28      /* ES						*/
#define SDBCS	29	/* CS						*/
#define SDBSS   30      /* SS						*/
#define SDBDS   31      /* DS						*/
#define SEGMI   32      /* immediate segment				*/
#define PRINT1  33      /* treat as a register				*/
#define DXADDR  34      /* dx used as an address register		*/
#define TRIPLEA 35   	/* triple address in use imul with lit		*/
#define SHIFT1  16      /* start of shift mask to determin the format   */
#define SHIFTCL  4      /* shift left by 1 + 1 = value of SDBCL         */
#define MODSHIFT 6	/* no of bits to shift byte to get the mod bits */
#define REGSHIFT 3      /* no of bits to shift byte to get the reg bits */
#define REGMASK  0x7    /* register field is only 3 bits 		*/
#define DISPONLY 24 	/* decode mapping for displacement only		*/
#define RMBASE   16	/* decode mapping for memory access skip 2xreg  */
#define NOARG    27     /* decode mapping for single byte inst 		*/
#define BYTEACC  8      /* this decode mapping for AL			*/
#define WORDACC  0	/* this decode mapping for AX			*/
#define IMMEDIATE 25	/* decode mapping for immediate only		*/
#define WORDFLAG 2	/* iflag value if word operation		*/
#define REGLSTS  8	/* 
#define SEGREGB  28	/* decode mapping base for the segment registers*/
#define SEGSHIFT 3      /* no of bits to shift iflag to get register    */
#define FLOATSH  4      /* flaoting point argument shift		*/
#define SIGNFLAG 4	/* iflag value if sign extending implied in fn  */
#define DIRECTN  1      /* iflag value that gives the direction of op.  */
#define BYTEMASK 0xff   /* byte mask to pick bottom byte from int	*/
#define ARGMASK  0x1f   /* argument mask possible 32 entries		*/
#define SWORD    2      /* indicates a word operand                     */
#define SBYTE    0      /* indicates a byte operand                     */
#define SE       4      /* indicates sign extension required            */
#define NOSE     0      /* indicates no sign extension required         */
#define AXDX     0 
#define DXAX     1
#define RIM      0
#define RMTOR    1      /* indicates rm to reg fn		        */
#define RTORM    0      /* indicates reg to rm fn			*/
#define IM       1      /* indicates imm fn				*/
#define ILLEGAL  0      /* illegal fn code				*/
#define MEMO     8      /* if set on some insts b1a BREAKA table reg ill*/
			/* and BREAKC to indicate memory op only        */
#define FLOATPOINT 0xffff /* floating point flag			*/
#define SHIFTMASK 0x0018 /* gives the shift type function               */
/*  breakout function codes						*/
#define BREAK1A  3
#define BREAKAN  2
#define BREAKAS  1
#define NORMAL   1
#define NORMALI  1
#define NORMIMM  2
/*  define the function codes for the primary table			*/
#define MODREGRM 1
#define IMM	 2
#define SEGR	 3
#define SEGOVR	 4
#define SNGLE	 5
#define DISP	 6
#define IMM1	 7
#define XCHG	 8
#define MODREGRMS 9
#define MODREGRMM 0xa
#define MODREGRMRM 0xb
#define MODREGRMI 0xc
#define DIRISEG	0xd
#define CHECKB	0xe
#define INSDX	0xf
#define MEMACC  0x10
#define IMM2	0x11
#define BREAKA	0x12
#define BREAKB	0x13
#define BREAKC	0x14
#define BREAKD	0x15
#define SHIFT	0x16
#define TRIPLE  0x17
#define ESCD8   0x18
#define ESC	0x19
#define WAITCHK 0x1a
/*  define the floating point argument size /type print index */
#define SHREAL 	1	/*  short real				*/
#define BYTE14	2	/*  14 byte				*/
#define BYTE2	3	/*  2  byte				*/
#define SHINT	4	/*  short integer			*/
#define TEMPREAL 5	/*  tempory real			*/
#define LONGREAL 6	/*  long real				*/
#define BYTE94	7	/*  94 byte ???				*/
#define WINT	8	/*  word integer			*/
#define LINT	9	/*  long integer			*/
#define BCD	10	/*  packed decimal			*/
/*  define the floating point jump tables			*/
#define FPOK	6       /*  add etc. routines are ok		*/
#define FPCOK	2	/*  comp entries			*/
#define FPCNOARG 5      /*  no argument for the double pop      */
#define FPCW	3	/*  execute but the compiler does not generate */
#define FPOKM   4	/*  standard memory operation           */
#define RESERVED 1      /*  reserved instruction                */
#define FNOARG  2       /*  no arguments required               */
#define FREGARG 3       /*  register arguments only             */
#define FEXONLY 4       /*  executes but not comp generated     */
#define FMARG   2       /*  memmory argument                    */
#endif
#endif
