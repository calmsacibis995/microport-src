/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident "@(#)dbg.c	1.3"

#ifdef GDEBUGGER
#include "sys/types.h"
#include "sys/param.h"
#include "sys/mmu.h"
#include "sys/reg.h"
#include "sys/seg.h"
#include <varargs.h>

#define	WRITEMEMSEL BUFSEL	/* free selector */

/*
 * Intel 80286 Debugger
 */

typedef	unsigned char	u_char;
typedef unsigned short	u_short;
typedef unsigned int	u_int;
typedef	unsigned long	u_long;

#define KVBASE		((u_long) 0x0L)
#undef	NULL
#define NULL    '\000'
#define TRUE    (1==1)
#define FALSE   (1==0)
#define ESC     0x1b
#define	TAB	0x09
#define CR      0x0d
#define LF      0x0a
#define NAK     0x15
#define BS      0x08
#define DEL     0x7f

#define BYTE        1           /* display formats */
#define WORD        2
#define LONG        4

#define NUM_LINES    8          /* number of dumped lines   */
#define NUM_BYTES   16          /* number of dumped bytes   */
#define	NUM_REGS    15		/* number of dumped regs per line */
#define NUM_BRKPTS  16          /* max number of brk points */
#define TRACE_SIMPLE	0x00	/* print whenever a location is touched */
#define TRACE_EQUAL	0x01	/* stop when location becomes equal to value */
#define TRACE_NOTEQ	0x02	/* stop when location becomes not eq to value */
#define NOWATCHPT	(u_char *)0xffffffff
#define BRKPT       0xcc	/* breakpoint instruction	*/
#define TRACE       0x100       /* mask to set trace		*/
#define NO_TRACE    ~(TRACE)    /* mask to unset trace		*/
#define	BREAK_INT	0x03	/* break interrupt		*/

/*
 * Table of register names and indexes into the ex_frame register array.
 */

#define	NREGS	SS+1

struct reg_info {
	char	*reg_name;
	short	index;
	short	type;
};

static struct reg_info	rinfo[] = {
	{ "AX", AX, 3 },
	{ "BX", BX, 3 },
	{ "CX", CX, 3 },
	{ "DX", DX, 3 },
	{ "CS", CS, 3 },
	{ "DS", DS, 3 },
	{ "ES", ES, 3 },
	{ "SS", SS, 3 },
	{ "SI", SI, 3 },
	{ "DI", DI, 3 },
	{ "BP", BP, 3 },
	{ "SP", SP, 3 },
	{ "IP", IP, 3 },
	{ "FL", FLGS, 3 },
	{ 0, 0, 0}
};

struct ex_frame {
	u_int	regs[NREGS + 2];
};

static struct ex_frame	*gexp;	/* global pointer to exception stack */

static u_char   *buf_ptr; /* global pointer into command buffer */
static u_char   buf[80]; /* command buffer */
char * findsymname();
char * get_adr();
char * parse_addr();
char * findsymaddr();
char * getname();
long str_to_hex();

u_char * nmi_sav;

static int
	step_over,     /* stepping over brkpt flag  */
	format = WORD, /* display format -- BYTE, WORD, LONG */
	trace_count;   /* trace multiple instructions */

static u_long user_ip;

int db_trace = 0;	/* global flag indicating we initiated trace */

#define	BRK_STOP	0x00	/* stop type breakpoint		*/
#define	BRK_TRACE	0x01	/* trace type breakpoint	*/
static char *brk_types [] = {
    "", "TRACE"
};
struct brk_tbl {
	char *adrs;
	char instr;
	char type;
};
static struct brk_tbl brk_tbl [NUM_BRKPTS] = { 0 };

/* exception types */
static char * ex_msg[] = {
	"DIVIDE", "TRACE", "NMI", "BREAK",
	"INTO OVFL", "BOUND", "INVALIP OP", "PROC EXT",
	"DOUBLE FAULT", "PROCEXT SEG OVRUN", "INV TASK STATE", "SEG NOT PRES",
	"STACK SEG OVRUN", "GEN PROT"
};
#define	NUM_EX_MSG	(sizeof(ex_msg)/sizeof(char *))

/* generic exception */
static char gen_msg[] = "Unknown Exception";
static char huh_msg[] = "?";
static char trace_msg[] = "Bad match, simple trace assumed\n";

static u_int user_sp, user_ss;

/* keyboard/display I/O definitions */
#ifdef AT386
#define	dbg_putchar(c)		asyputchar (c)
#define	dbg_getchar()		asygetchar (TRUE)
#define	dbg_getcharnowait()	asygetchar (FALSE)
#else
#define	dbg_putchar(c)		dbgputc (c)
#define	dbg_getchar()		dbgkbrd (TRUE)
#define	dbg_getcharnowait()	dbgkbrd (FALSE)
#endif

int kernstop = 0;		/* set non-zero to stop kernel early */

/* asy debugger initialize */
asydbginit ()
{
#ifndef	MP386			/* can't stop here on the 386 */
    /*
     * To enter debugger as early as possible, set "kernstop"
     * to non-zero (using the patch utility).
     */
    if (kernstop)
	asm (" int $3");		/* call the debugger */
#endif
}

/* entry to debugger */
dbg_enter ()
{
    asm (" int $3");
}

/* Debugger main entry */
dbg(type, ex_frame, err, cs)
    u_int type;
    struct ex_frame *ex_frame;
    u_int err;
    u_int cs;
{
	char	cmd;
	char	match;
	u_long tmpadr;

	asm(" cli");
	/*
	 * check if we're just stepping over a breakpoint or some such.
	 */
	if (step_over && (ex_frame->regs[FLGS] & TRACE)) {
		step_over = FALSE;
		db_trace = FALSE;
		ex_frame->regs[FLGS] &= NO_TRACE;
		replace_brkpts();
		return(0);
	}
	gexp = ex_frame;
	remove_brkpts();
	/*
	 * Fixup user's IP
	 */
	user_ip = (u_long) (ex_frame->regs [CS]) << 16 
		| (u_long)  ex_frame->regs [IP];

	if (cont_onward (type))	/* if we're to continue */
	    return (0);		/* then do it */
	else {			/* otherwise we're stopping here */
	    dbg_printf ("\nTrap #%b: ", type);
	    if (type >= NUM_EX_MSG)
		    dbg_printf("'%s'", gen_msg);
	    else
		    dbg_printf("'%s Exception'", ex_msg[type]);
	}
	if (!(USERMODE (cs))) {
	    tmpadr = (u_long) (&ex_frame->regs [FLGS + 1]);
	    user_ss = (tmpadr >> 16) & 0xFFFF;
	    user_sp = tmpadr & 0xFFFF;
	}
	else {
	    user_ss = ex_frame->regs [SS];
	    user_sp = ex_frame->regs [SP];
	}

	ex_frame->regs[FLGS] &= NO_TRACE;      /* could have been single step */
	db_trace = FALSE;
	dbg_printf (", Debugger entered from ");
	(void) findsymname (user_ip, 1);
	dbg_printf ("\n");
	prn_regs (ex_frame);
	while (TRUE) {
		while (dbg_getcharnowait() != 0xff)  /* flush uart buf */
			;
		dbg_printf("d: ");	/* display prompt */
		get_line(TRUE);		/* get command line */
		cmd = *buf_ptr++;
		switch (cmd) {
		case NULL:
			break;
		case '?':		/* display break points */
			dspbrkpts();
			break;
		case 'A':		/* set access watch point */
		case 'a':
			no_watchpt();
			break;
		case 'B':		/* (re-) Boot */
			kerndebug ();
			/*NO RETURN*/
		case 'b':		/* set a breakpoint */
			insertbkpt ((char *) user_ip, BRK_STOP);
			break;
		case 'C':		/* clear a watch point */
		case 'c':
			no_watchpt();
			break;
		case 'D':		/* display memory contents */
		case 'd':
			dump_memory();
			break;
		case 'E':		/* examine/alter memory location */
		case 'e':
		case 'W':		/* write to memory location */
		case 'w':
			examine(to_upper(cmd) == 'W');
			break;
		case 'F':		/* set display format */
		case 'f':
			set_format();
			break;
		case 'G':		/* continue from breakpoint */
		case 'g':
			if (*buf_ptr != NULL) {
			    if (tmpadr = (u_long) get_adr()) {
				user_ip = tmpadr;
				replace_ip ();
			    }
			}
			if (find_brkpt ((char *) user_ip)) {
			    step_over = TRUE;
			    db_trace = TRUE;
			    ex_frame->regs[FLGS] |= TRACE;
			}
			else
			    replace_brkpts();
			return(0);
		case 'H':		/* print a list of debugger commands */
		case 'h':
			dbg_help();
			break;
		case 'I':		/* do an inb or inw or inl */
		case 'i':
			do_in();
			break;
		case 'K':		/* kill a breakpoint */
		case 'k':
			rmbkpt();
			break;
		case 'L':	/* Lookup an address and return symbol name */
		case 'l':
			(void) findsymname (get_adr (), 1);
			dbg_printf ("\n");
			break;
		case 'M':		/* set a modify watch point */
		case 'm':
			no_watchpt();
			break;
		case 'O':		/* do an outb or outw or outl */
		case 'o':
			do_out();
			break;
		case 'R':		/* examine/display registers */
		case 'r':
			ex_regs(ex_frame);
			break;
		case 'S':		/* symbol table dump */
			sym_dump();
			break;
		case 's':		/* single step */
		case ESC:
			ex_frame->regs[FLGS] |= TRACE;
			db_trace = TRUE;
			return(0);
		case 'T':		/* trace location */
		case 't':
			no_watchpt();
			break;
		case 'X':		/* trace execution */
		case 'x':
			insertbkpt ((char *) user_ip, BRK_TRACE);
			break;
		case 'Z':		/* dump tss */
		case 'z':
			dump_tss();
			break;
		default:
			dbg_printf ("%s\n", huh_msg);
			break;
		}
	}
}

/*
 * debugger help routine
 */
dbg_help()
{
	dbg_printf("debugger commands:\n");
	dbg_printf("b <address>               ");
	dbg_printf(" - Set breakpoint at address\n");
	dbg_printf("d [address]               ");
	dbg_printf(" - Dump memory in current format\n");
	dbg_printf("e <address>               ");
	dbg_printf(" - Examine/open a memory location\n");
	dbg_printf("f{b|w|l}                  ");
	dbg_printf(" - Set display format\n");
	dbg_printf("g [address]               ");
	dbg_printf(" - Continue execution at address\n");
	dbg_printf("h                         ");
	dbg_printf(" - Print this list\n");
	dbg_printf("i <address>               ");
	dbg_printf(" - Do an in instruction at address\n");
	dbg_printf("k <brkpt_num>             ");
	dbg_printf(" - Kill given breakpoint (* for all)\n");
	dbg_printf("l <address>               ");
	dbg_printf(" - Print nearest symbol before address\n");
	dbg_printf("o <address> <value>       ");
	dbg_printf(" - Do an out instruction at address\n");
	dbg_printf("r [%<reg> <value>]        ");
	dbg_printf(" - Set/display registers\n");
	dbg_printf("s                         ");
	dbg_printf(" - Single step (also ESC)\n");
	dbg_printf("t <address> [{=|!=} <val>]");
	dbg_printf(" - Set trace point at address\n");
	dbg_printf("w <address>               ");
	dbg_printf(" - Write to memory without read\n");
	dbg_printf("x <address>               ");
	dbg_printf(" - Set execution trace point at address\n");
	dbg_printf("z                         ");
	dbg_printf(" - Dump some values from the TSS\n");
	dbg_printf("?                         ");
	dbg_printf(" - Display break points\n");
	dbg_printf("note: <address> can be symbolic name or hex value\n");
}

/*
 * Announce reason for debugger entry.
 * Also determine if we should continue execution or stop.
 */
cont_onward (type)
    int type;		/* interrupt type */
{
    int	brk_num;

    if ((type == BREAK_INT)
    && ((gexp->regs [FLGS] & TRACE) == 0)
    &&  (brk_num = find_brkpt ((char *) (user_ip - 1)))) {
	brk_num--;
	user_ip -= 1;
	replace_ip ();
	switch (brk_tbl [brk_num].type) {
	    case BRK_TRACE:
		dbg_printf ("trace %b at %l ", brk_num, user_ip);
		(void) findsymname (user_ip, 1);
		dbg_printf("\n");
		step_over = TRUE;
		db_trace = TRUE;
		gexp->regs[FLGS] |= TRACE;
		return TRUE;
	}
	dbg_printf("brkpt %b: ", brk_num);
	(void) findsymname (user_ip, 1);
	dbg_printf("\n");
    }
    return FALSE;
}
static
strlen (s)
    char *s;
{
    int i = 0;

    while (*s++)
	i++;
    return i;
}
static
strncmp (s1, s2, n)
    char *s1, *s2;
    int n;
{
    if (n) {
	while (--n > 0 && *s1 && *s1 == *s2) {
	    s1++;
	    s2++;
	}
	return *s1 != *s2;	/* does not have < 0 */
    }
    return 0;
}
	

no_watchpt()
{
    dbg_printf ("Watchpoints are not available on the 286\n");
}

replace_ip ()
{
    gexp->regs [IP] = user_ip & 0xFFFF;
    gexp->regs [CS] = (user_ip >> 16) & 0xFFFF;
}

write_memory (format, adrs, value)
    int format;
    u_char *adrs;
    long value;
{
    extern struct seg_desc gdt[];
    struct seg_desc savegdt;
    u_char *mapadrs;
    u_char *mapin();
    paddr_t paddr;
    paddr_t physaddr();

    savegdt = gdt [WRITEMEMSEL];	/* save present selector contents */
    paddr = physaddr (adrs);		/* get physical address */
					/* map to a logical address */
    mapadrs = mapin (paddr, WRITEMEMSEL);
    switch (format) {
	case BYTE: *(char  *) mapadrs = (char)  value;	break;
	case WORD: *(short *) mapadrs = (short) value;	break;
	case LONG: *(long  *) mapadrs = (long)  value;	break;
    }
    adrs = (u_char *) ((long) adrs & (long) ~0xFFFFL);
    gdt [WRITEMEMSEL] = savegdt;	/* restore previous selector contents */
}

dump_tss()
{
}

/*
 * display break points
 */

dspbrkpts()
{
    int i;
    struct brk_tbl *brkp = brk_tbl;

    dbg_printf ("brkpts:\n");
    for (i = 0; i < NUM_BRKPTS; brkp++, i++)
	if (brkp->adrs) {
	    dbg_printf ("%b at %l ", i, (char *) brkp->adrs);
	    (void) findsymname ((char *) brkp->adrs, 1);
	    if (brkp->type)
		dbg_printf(" (%s)", brk_types [brkp->type]);
	    dbg_printf("\n");
	}
}

/*
 * dump memory
 */
static char *dump_sav;
static char dump_line [16];
int dbg_dumplinelen = NUM_LINES;

dump_memory()
{
    int i, j;
    u_char c;
    char *tmpadr;
    char dmpfmt = format;

    skipblanks ();
    if (*buf_ptr != NULL)         /* is there dump address */
	if (tmpadr = get_adr ())
	    dump_sav = tmpadr;
	else
	    return;

    for (i = 0; i < dbg_dumplinelen; i++) {
	dbg_printf ("%l: ", dump_sav);

	for (j = 0; j < NUM_BYTES; j += dmpfmt) {
	    switch (dmpfmt) {
		case BYTE: dbg_printf ("%b", *((u_char *)  dump_sav));	break;
		case WORD: dbg_printf ("%w", *((u_short *) dump_sav));	break;
		case LONG: dbg_printf ("%l", *((u_long *)  dump_sav));	break;
	    }
	    dump_sav += dmpfmt;
	    dbg_printf(" ");
	}
	dump_sav -= NUM_BYTES;
	dbg_printf("  |");                      /* do ascii part */
	for (j = 0; j < NUM_BYTES; j++) {
	    c = *dump_sav++;
	    if ((c > 0x1f) && (c < 0x7f))
		dbg_putchar (c);
	    else
		dbg_printf (".");
	}
	dbg_printf ("|\n");
    }
}

/*
 *      examine and/or change a memory location
 */

static u_long exam_sav;

examine(write_only)
	int	write_only;
{
    u_long value = 0;
    char c;
    u_long tmpadr;

    if (*buf_ptr && (tmpadr = (u_long) get_adr()))
	exam_sav = tmpadr;
    else
	exam_sav += format;

    while (tmpadr) {
	dbg_printf("%l = ", exam_sav);
	if (!write_only) {
	    switch (format) {
		case BYTE: dbg_printf("%b", *((u_char *) exam_sav)); break;
		case WORD: dbg_printf("%w", *((u_short *)exam_sav)); break;
		case LONG: dbg_printf("%l", *((u_long *) exam_sav)); break;
	    }
	    dbg_printf(": ");
	}
	get_line (TRUE);
	c = *buf_ptr;
sw1:
	switch (c) {
	    case NULL:
		return;
	    case '+':
	    case LF:
		exam_sav += format;
		dbg_printf("\n");
		break;
	    case '-':
		exam_sav -= format;
		dbg_printf("\n");
		break;
	    case '@':
		exam_sav = *((u_long *) exam_sav);
		dbg_printf("\n");
		break;
	    default:
		if (!is_hex_char(c)) {
		    dbg_printf((char *) huh_msg);
		    return;
		}
		value = (u_long) get_adr();
		write_memory (format, exam_sav, value);
		c = *buf_ptr++;
		goto sw1;
	}
    }
}

/*
 *      set the format
 */
set_format()
{
	skipblanks();
	switch (*buf_ptr++) {
	case 'B':
	case 'b':
		format = BYTE;
		dbg_printf("BYTE\n");
		break;
	case 'W':
	case 'w':
		format = WORD;
		dbg_printf("WORD\n");
		break;
	case 'L':
	case 'l':
		format = LONG;
		dbg_printf("LONG\n");
		break;
	}
}

/*
 * do an in instruction from given I/O port.
 */
do_in()
{
    u_int	inaddr;

    if (inaddr = (u_int) get_adr())
	switch (format) {
	    case BYTE:
		dbg_printf(" %b\n", inb(inaddr));
		break;
	    case WORD:
		dbg_printf(" %w\n", in(inaddr));
		break;
	}	
}

/*
 * do an out instruction to given I/O port.
 */
do_out()
{
	u_int	outaddr;
	u_int	outvalue;

	outaddr  = (u_int) get_adr();
	outvalue = (u_int) get_adr();

	switch (format) {
	case BYTE:
		outb(outaddr, outvalue);
		break;
	case WORD:
		out(outaddr, outvalue);
		break;
	}	
}

/*
 *      insert a break point
 */

insertbkpt (eip, brktype)
    char *eip;
    int brktype;
{
    char i;
    struct brk_tbl * tblptr = brk_tbl;
    char *brkadrs;

    for (i = 0; i < NUM_BRKPTS; i++) {		/* find an open slot	*/
	if (! tblptr->adrs)
	    break;
	tblptr++;
    }

    if (i >= NUM_BRKPTS) {			/* all slots filled	*/
	dbg_printf ("Breakpoint table is full\n");
	return;
    }

    skipblanks();
    if (*buf_ptr) {
	brkadrs = (char *) get_adr();
	if (brkadrs)		/* put it in table	*/
	    tblptr->adrs = (char *) brkadrs;
	else {					/* bad address		*/
	    dbg_printf ("No breakpoint installed.\n");
	    return;
	}
    }
    else
	tblptr->adrs = eip;

    tblptr->type = brktype;
    dbg_printf ("%b at %l ", i, (char *) tblptr->adrs);
    (void) findsymname (tblptr->adrs, 1);
    if (tblptr->type)
	dbg_printf (" (%s)", brk_types [tblptr->type]);
    dbg_printf ("\n");
}

/*
 *      remove one or all break points
 */

rmbkpt()
{
	register int i;

	skipblanks();
	if (*buf_ptr == '*')
		for (i = 0; i < NUM_BRKPTS; i++)
			brk_tbl[i].adrs = 0;
	else {
		i = str_to_hex(buf_ptr);
		brk_tbl[i].adrs = 0;
	}
}

/*
 * open a register
 */
ex_regs(exp)
	struct ex_frame	*exp;
{
	char	*name;
	u_int	value;
	int	i, rindx;

	skipblanks();
	if (*buf_ptr == '%')
	    buf_ptr++;

	skipblanks();
	name = getname();

	if (*name) {
	    for (i = 0; rinfo [i].reg_name != NULL; i++)
		if (str_match (name, rinfo [i].reg_name))
		    break;
	    if (rinfo[i].reg_name == NULL) {
		dbg_printf (huh_msg);
		return;
	    }
	    rindx = rinfo [i].index;
	    switch (rindx) {
		case SP:
		case CS:
		case SS:
		case DS:
		case ES:
		    dbg_printf ("not allowed to change reg %s\n",
			rinfo [i].reg_name);
		    return;
	    }
	    skipblanks ();
	    if (*buf_ptr) {
		value = (u_int) get_adr();
		switch (rinfo[i].type) {
		    case 3: /* change 16 bit register */
			    exp->regs [rindx] = value;
			    break;

		    case 2: /* change a hi 8 bit register */
			    hibyte (exp->regs[rindx]) = (value & 0x00ff) << 8;
			    break;

		    case 1: /* change a lo 8 bit register */
			    lobyte (exp->regs[rindx]) = value & 0x00ff;
			    break;

		    default:    /* illegal command */
			    dbg_printf("%s", huh_msg);
		}
		if (rindx == IP) {
		    user_ip = (user_ip & 0xFFFF0000) | exp->regs[IP];
		    replace_ip ();
		}
	    }
	}
	prn_regs(exp);
}

/*
 *      prn_regs() -- display the registers
 */

prn_regs(exp)
struct ex_frame	*exp;
{
	int i = 0;
	int j;
	u_int value;

	while (1) {
	    for (j = i; j < (i + NUM_REGS); j++) 
		if (rinfo [j].reg_name)
		    dbg_printf (" %s  ", rinfo [j].reg_name);
		else
		    break;
	    dbg_printf ("\n");
	    for (j = i; j < (i + NUM_REGS); j++) 
		if (rinfo [j].reg_name) {
		    if (rinfo [j].index == SS)
			value = user_ss;
		    else if (rinfo [j].index == SP)
			value = user_sp;
		    else
			value = exp->regs [rinfo [j].index];
		    dbg_printf ("%w ", value);
		}
		else {
		    dbg_printf ("\n");
		    return;
		}
	    dbg_printf ("\n");
	    i += NUM_REGS;
	}
}

/*
 *      break point routines
 */

replace_brkpts()
{
	register int i;
	register struct brk_tbl * tblptr = brk_tbl;

	for (i = 0; i < NUM_BRKPTS; i++) {
		if (tblptr->adrs) {
			tblptr->instr = *tblptr->adrs;
			write_memory (BYTE, tblptr->adrs, (long) BRKPT);
		}
		tblptr++;
	}
}

find_brkpt (address)
	register char * address;
{
	int i;
	struct brk_tbl * tblptr = brk_tbl;

	for (i = 0; i < NUM_BRKPTS; i++)
	    if ((tblptr++)->adrs == address)
		return (i + 1);
	return 0;
}


remove_brkpts()
{
    register int    i;
    register struct brk_tbl  *tblptr = brk_tbl;

    for (i = 0; i < NUM_BRKPTS; i++) {
	if (tblptr->adrs)
	    write_memory (BYTE, tblptr->adrs, (long) tblptr->instr);
	tblptr++;
    }
}

/*
 * utility subroutines
 */

long
str_to_hex(str)
	register u_char  *str;
{
	register long num = 0;
	register u_char  c;

	for(;;) {
		c = *str++;
		if ((c >= '0') && (c <= '9'))
			num = (num << 4) + (c - '0');
		else if ((c >= 'A') && (c <= 'F'))
			num = (num << 4) + (c - 'A' + 10);
		else if ((c >= 'a') && (c <= 'f'))
			num = (num << 4) + (c - 'a' + 10);
		else
			return(num);
	}
}

/*
 * Take the next thing in the command buffer pointed to by buf_ptr and 
 * interpret it as an address, and return the resulting address.
 */
char *
get_adr()
{
	char	*name;
	int i, rindx;
	u_long ret = 0;

	skipblanks();
	switch (*buf_ptr) {
	case NULL:
		ret = 0;
		break;
	case '%':
		buf_ptr++;
		if (*buf_ptr == 'F')
		    return (char *) gexp;

	    nextreg:
		name = getname();
		for (i = 0; rinfo[i].reg_name != NULL; i++)
		    if (str_match(name, rinfo[i].reg_name)) {
			    rindx = rinfo[i].index;
			    break;
		    }
		if (rinfo[i].reg_name == NULL || rinfo[i].type != 3) {
		    dbg_printf("bad reg %s, %ax assumed.\n", name);
		    rindx = AX;
		}
		/* get address from register */
		switch (rindx) {
		    case BP:	/* these use the Stack Segment */
			ret = (u_long) (user_ss) << 16
			    | (u_long) (gexp->regs [BP]);
			break;
		    case SP:
			ret = (u_long) (user_ss) << 16 | (u_long) (user_sp);
			break;

		    case IP:	/* this uses the Code Segment */
			ret = (u_long) (gexp->regs [CS]) << 16
			    | (u_long) (gexp->regs [IP]);
			break;

		    case CS:	/* these ARE the segment regs */
		    case DS:
		    case ES:
			ret = (u_long) (gexp->regs [rindx]) << 16;
			skipblanks();
			if (*buf_ptr) 	/* a 2nd reg specified */
			    goto nextreg;
			break;

		    case SS:
			ret = (u_long) (user_ss) << 16;
			break;

		    case SI:	/* *normally* the SI is used with DS */
			if (!ret)
			    ret = (u_long) (gexp->regs [DS]) << 16;
			ret |= (u_long) (gexp->regs [SI]);
			break;

		    default:
			/* Just guessing at ES.
			 * Could also be DS, but ES is usually used for
			 * indexing.
			 */
			if (!ret)
			    ret = (u_long) (gexp->regs [ES]) << 16;
			ret |= (u_long) (gexp->regs [rindx]);
			break;
		}
		break;
	case '@':
		buf_ptr++;
		ret = *(u_long *) get_adr ();
		break;
	default:
		ret = (u_long) parse_addr();
		break;
	}
	return ((char *) ret);
}

/*
 * get a debugger input line.
 * if the specdelim flag is true then
 * the line will be terminated by LF, +, -, ESC, or @ in addition to CR.
 */
get_line(specdelim)
{
	register u_char * bufptr = buf;
	register int stop = 0;
	register u_char   c;

	buf_ptr = buf;
	for (;;) {
		c = dbg_getchar() & 0x7f;
		switch (c) {
		case CR:
			dbg_putchar(LF);
			*bufptr = NULL;
			return;
		case '+':
		case '-':
		case '@':
			if (!specdelim)
				goto nospec;
			dbg_putchar(c);
			/* fall thru here */
		case LF:
		case ESC:
			if (!specdelim)
				goto nospec;
			*bufptr++ = c;
			*bufptr = NULL;
			return;
		case DEL:
		case BS:
			if (stop > 0) {
				bufptr--;
				stop--;
				dbg_putchar(BS);
				dbg_putchar(' ');
				dbg_putchar(BS);
			}
			break;
		case NAK:
			while (stop > 0) {
				bufptr--;
				stop--;
				dbg_putchar(BS);
				dbg_putchar(' ');
				dbg_putchar(BS);
			}
			break;
		default :
nospec:
			if (c >= 0x20) {
				dbg_putchar(c);
				*bufptr++ = c;
				stop++;
			}
			break;
		}
	}
}

/*
 * parse a debugger address, we collect the token then
 * first try to parse it as a hex constant, if that
 * fails, then we try it as a symbol name and look it up
 * in the kernel symbol table.  if that fails we print an error
 * message and return 0.  Preceding a symbolic name with a 
 * double quote " will force its evaluation as a symbol, even
 * if the name would be a legal hex constant.
 */

static u_char	pbuf[64];

char *
parse_addr()
{
	char * ret;
	int ishex;
	u_char	*p;
	u_char	c;

	p = pbuf;
	if (*buf_ptr == '"') {
		ishex = FALSE;
		buf_ptr++;
	} else
		ishex = TRUE;
	while (is_char(c = *buf_ptr)) { /* collect the token */
		if (!is_hex_char(c))
			ishex = FALSE;
		*p++ = *buf_ptr++;
	}
	*p = NULL;
	if (ishex)
		return (char *) str_to_hex(pbuf);
	else
		if ((ret = findsymaddr(pbuf)) == 0) {
			dbg_printf("no such symbol %s, %l assumed.\n",
					pbuf, KVBASE);
			ret = (char *) KVBASE;
		}
	return ret;
}

static char	namebuf[64];

char *
getname()
{
	char	*namep;

	namep = namebuf;
	while (is_char(*buf_ptr)) /* collect the token */
		*namep++ = *buf_ptr++;
	*namep = NULL;
	return namebuf;
}


/*
 * findsymname looks thru symtable to find the routine name which begins
 * closest to value.  The format of symtable is a list of
 * (long, null-terminated string) pairs; the list is sorted
 * on the longs.  symtable is populated by patching the kernel using
 * a program called unixsyms.
 */

char symtable[30000] = {0};     /* initialize to get it into .data section */

char *
findsymname(value, tell)
    u_long value;
    int tell;   /* flag to print name and location */
{
	char *p = symtable;
	char *oldp = p;
	long loc;

	while (*(u_long *)p <= value)
	{
		oldp = p;
		p += sizeof(long);	/* jump past long */
		while (*p++);		/* jump past string */
	}
	p = oldp + sizeof(long);        /* name string */
	loc = *(long *)oldp;            /* address */
	switch (tell) {
	    case 1:
		dbg_printf("%s", (char *) p);
		if (loc != value)
			dbg_printf("+%l", (char *) (value - loc));
		break;
	    case 2:
		return oldp;
	}
	return p;
}

/*
 * findsymaddr looks thru symtable to find the address of name.
 */

char *
findsymaddr(name)
    char *name;
{
	char *p = symtable + sizeof(long);
	char *oldp = p;
	char *namep;
	while (*p)
	{
		oldp = p;
		for (namep = name; *namep && *p; namep++, p++)
			if (*namep != *p)
				break;
		if (!*namep && !*p)
			return(*(char **)(oldp - sizeof(long)));
		while (*p++);		/* jump past rest of name */
		p += sizeof(long);	/* jump past long */
	}
	return 0L;
}

/*
 * dump symbols
 */
static u_long *symptr = (u_long *) symtable;
static u_char *symname = (u_char *) "\0";
static short symlen = 0;

sym_dump()
{
    register short i;
    char *nameptr;
    u_long adrs, *tmpptr, *lastptr;

    skipblanks();
    if (*buf_ptr) {
	if (*buf_ptr == '0') {
	    symlen = 0;
	    symptr = (u_long *) symtable;	/* restart at beginning */
	}
	else {
	    symname = buf_ptr;
	    symlen = strlen (symname);
	    if (adrs = (u_long) get_adr ()) {
		lastptr = tmpptr = (u_long *) symtable;
		while (adrs <= *tmpptr) {
		    lastptr = tmpptr++;
		    nameptr = (char *) tmpptr;	/* skip address */
		    while (*nameptr++)		/* skip name */
			;
		    tmpptr = (u_long *) nameptr;
		}
		symptr = lastptr;
		symlen = 0;
	    }
	    else
		symptr = (u_long *) symtable;	/* restart at beginning */
	}
    }

    for (i = 0; i < dbg_dumplinelen; ) {
	nameptr = (char *) (symptr + 1);
	if (*nameptr) {
	    if ((symlen == 0) || (strncmp (nameptr, symname, symlen) == 0)) {
		dbg_printf ("%l: %s\n", *(long *) symptr, nameptr);
		i++;
	    }
	    while (*nameptr++)
		;
	    symptr = (u_long *) nameptr;
	}
	else {
	    symptr = (u_long *) symtable;
	    return;
	}
    }
}

str_match(p1, p2)
	u_char	*p1;
	u_char	*p2;
{
	while (*p1 && *p2 && (to_upper(*p1) == to_upper(*p2))) {
		p1++;
		p2++;
	}
	if (*p1 || *p2)
		return FALSE;
	else
		return TRUE;
}

skipblanks()
{
	while (*buf_ptr == ' ' || *buf_ptr == TAB)
		buf_ptr++;
}

is_char(c)
	u_char	c;
{
	return ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') ||
		(c >= 'A' && c <= 'Z') || c == '_');
}

is_hex_char(c)
	u_char	c;
{
	return ((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') ||
		(c >= 'a' && c <= 'f'));
}

to_upper(c)
	u_char	c;
{
	if (('a' <= c) && (c <= 'z'))
		c -= ('a' - 'A');
	return(c);
}

/*VARARGS0*/
dbg_printf (fmtstr, va_alist)
    char *fmtstr;   /* format string */
    va_dcl
{
    va_list ap;			/* arg pointer */
    char *tmpstr;   		/* temporary for %s format */
    char cc, sc;    		/* next control character */
    unsigned long value;	/* temporary for the value to be printed */

    va_start (ap);
    while ((cc = *fmtstr++) != (char) 0) {
	switch (cc) {
	    default:
		dbg_putchar (cc);
		break;

	    case '%':
		switch (cc = *fmtstr++) {
		    /* formats this dbg_printf understands */
		    case 's':		/* string format */
			tmpstr = va_arg (ap, char *);
			while ((sc = *tmpstr++) != '\0')
			    dbg_putchar (sc);
			break;

		    case 'b':		/* byte value */
			value = va_arg (ap, short);
			hexadecimal ((long) value, 2, '0');
			break;

		    case 'w':		/* word value */
			value = va_arg (ap, short);
			hexadecimal ((long) value, 4, '0');
			break;

		    case 'l':		/* long value */
			value = va_arg (ap, long);
			hexadecimal ((long) value, 8, '0');
			break;

		    case 'L':		/* long value with leading blanks */
			value = va_arg (ap, long);
			hexadecimal ((long) value, 8, ' ');
			break;

		    case '\0':	/* % was at end of string, return */
			dbg_putchar ('%');
			return;

		    default:	/* Bad format, just parrot control string */
			dbg_putchar ('%');
			dbg_putchar (cc);
			break;
		}
	}
    }
}

hexadecimal(number, places, preceed)
    long number;    /* Number to format */
    int places;    /* width of format field */
    char preceed;  /* preceeding 0s/spaces */
{
	int i,digitfound,shift;
	u_char digit;

	digitfound = 0;
	for(i=0; i<places; i++) {
		shift = ((places - 1)-i) * 4;
		digit = (number >> shift) & 0xf;
		if( digit == 0 && !digitfound && i!=(places - 1))
			dbg_putchar(preceed);
		else {
			digitfound++;
			dbg_putchar((digit < 10) ? (digit+'0') : (digit+'a'-10));
		}
	}
}

/*
 * used to call the debugger from the kernel
 */
debugger(dummy)
{
	asm(" int $3");
}

#else /* not GDEBUGGER */
#ifndef DEBUGGER
/*
 * used to call the debugger from the kernel
 */
debugger(dummy)
{
}
#endif
#endif /* GDEBUGGER */

/* === */
