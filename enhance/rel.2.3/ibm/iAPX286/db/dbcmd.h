
/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident  "@(#)dbcmd.h	1.5"

/*
 * The debugger's fixed command table structure.
 */
struct cmdentry {
    char    *name;          /* command's textual name */
    uchar   index;          /* action in dbintrp.c:doname(); use C_ defines */
    uchar   stackcheck;     /* stack bounds check, see STACKCHK(d,u) below  */
    uchar   parmcnt;        /* number of checked stack parameters for cmd   */
    uchar   parmtypes[3];   /* allowed type mask for each parm, 3 max       */
};

/*
 * Debugger command indices.
 *  These values are used to selection action code in the switch statement
 *  in dbintrp.c : doname().  When adding new commands, add to the end of the
 *  list and change MAXCMDS to have the same value as the highest index.
 */
#define C_READBYTE      1
#define C_READWORD      2
#define C_READLONG      3
#define C_WRITEBYTE     4
#define C_WRITEWORD     5
#define C_WRITELONG     6
#define C_BANG          7
#define C_PLUSPLUS      8
#define C_MINUSMINUS    9
#define C_STAR          10
#define C_DIV           11
#define C_PERCENT       12
#define C_PLUS          13
#define C_MINUS         14
#define C_RSHIFT        15
#define C_LSHIFT        16
#define C_LESS          17
#define C_GREATER       18
#define C_EQUAL         19
#define C_NOTEQUAL      20
#define C_AND           21
#define C_XOR           22
#define C_OR            23
#define C_ANDAND        24
#define C_OROR          25
#define C_GETS          26
#define C_PRINT         27
#define C_POP           28
#define C_CLRSTK        29
#define C_DUP           30
#define C_NOVERBOSE     31
#define C_VERBOSE       32
#define C_INBASE        33
#define C_OUTOCTAL      34
#define C_OUTDECIMAL    35
#define C_OUTHEX        36
#define C_INBINARY      37
#define C_INOCTAL       38
#define C_INDECIMAL     39
#define C_INHEX         40
#define C_DUMPSTACK     41
#define C_VARS          42
#define C_KERNELSTACKDUMP 43
#define C_FINDSYM       44
#define C_PINODE        45
#define	C_SYSDUMP	46
#define	C_DUMP		47
#define C_STACK         48
#define C_DR0           49
#define C_DR1           50
#define C_DR2           51
#define C_DR3           52
#define C_DR6           53
#define C_DR7           54
#define C_WDR0          55
#define C_WDR1          56
#define C_WDR2          57
#define C_WDR3          58
#define C_WDR6          59
#define C_WDR7          60
/* can't use 61 */
#define C_SAVEREGS      62
#define C_GETDS         63
#define C_GETES         64
#define C_GETFS         65
#define C_GETGS         66
#define C_GETDI         67
#define C_GETSI         68
#define C_GETBP         69
#define C_GETSP         70
#define C_GETBX         71
#define C_GETDX         72
#define C_GETCX         73
#define C_GETAX         74
#define C_GETTP         75
#define C_GETER         76
#define C_GETCS         77
#define C_GETIP         78
#define C_GETFL         79
#define C_USEREGS       80
#define C_I             81
#define C_A             82
#define C_AW            83
#define C_AL            84
#define C_M             85
#define C_MW            86
#define C_ML            87
#define C_NOBRK         88
#define C_BRK0          89
#define C_BRK1          90
#define C_BRK2          91
#define C_BRK3          92
#define C_TRC0          93
#define C_TRC1          94
#define C_TRC2          95
#define C_TRC3          96
#define C_DBSTAT        97
#define C_HELP          98


#define C_MAXCMDS       98

/*
 * Parameter type checking masks.
 *  These can be used singly or in combination to specify the allowed types
 *  of stack parameters for each command.
 */
#define T_NUMBER        0x01
#define T_NAME          0x02
#define T_STRING        0x04

/*
 * Stack depth checking masks.
 *  The stackcheck member of struct cmdentry specifies the lower and upper
 *  bounds of stack growth during the command's execution.  The predefined
 *  values S_1_0, S_2_0, and S_0_1 are the only ones currently needed, but
 *  for new commands, arbitrary checks can be constructed with STACKCHK().
 */

#define S_DOWNMASK      0x0f
#define S_UPMASK        0xf0
#define S_DOWNSHIFT     0
#define S_UPSHIFT       4

#define S_DOWN(x) (((x)&S_DOWNMASK)>>S_DOWNSHIFT)
#define S_UP(x)   (((x)&S_UPMASK)>>S_UPSHIFT)

#define STACKCHK(d,u)   ((((d)<<S_DOWNSHIFT)&S_DOWNMASK) | \
					(((u)<<S_UPSHIFT)&S_UPMASK))
#define S_1_0           STACKCHK(1,0)
#define S_2_0           STACKCHK(2,0)
#define S_0_1           STACKCHK(0,1)
