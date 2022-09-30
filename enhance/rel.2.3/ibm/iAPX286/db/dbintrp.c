
/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident  "@(#)dbintrp.c	1.7"

#ifdef DEBUGGER

#include "sys/types.h"
#include "sys/param.h"
#include "debugger.h"
#include "dbcmd.h"

ushort dbtos;
extern ushort *firstarg;
struct item dbstack[DBSTKSIZ];
static struct variable variable[VARTBLSIZ];
static char *outformat = "%lx";
static ushort dbobase;
static char *typename[] =
	{"NULL","NUMBER","STRING","NAME" };
char dbverbose = 0;
extern int tcount[4];

struct cmdentry cmdtable[] = {

{ "r1",                     C_READBYTE,     S_1_0,  1,  T_NUMBER,               },
{ "r2",                     C_READWORD,     S_1_0,  1,  T_NUMBER,               },
{ "r4",                     C_READLONG,     S_1_0,  1,  T_NUMBER,               },
{ "w1",                     C_WRITEBYTE,    S_1_0,  2,  T_NUMBER,   T_NUMBER,   },
{ "w2",                     C_WRITEWORD,    S_1_0,  2,  T_NUMBER,   T_NUMBER,   },
{ "w4",                     C_WRITELONG,    S_1_0,  2,  T_NUMBER,   T_NUMBER,   },
{ "!",                      C_BANG,         S_1_0,  1,  T_NUMBER,               },
{ "++",                     C_PLUSPLUS,     S_1_0,  1,  T_NUMBER,               },
{ "--",                     C_MINUSMINUS,   S_1_0,  1,  T_NUMBER,               },
{ "*",                      C_STAR,         S_2_0,  2,  T_NUMBER,   T_NUMBER,   },
{ "/",                      C_DIV,          S_2_0,  2,  T_NUMBER,   T_NUMBER,   },
{ "%",                      C_PERCENT,      S_2_0,  2,  T_NUMBER,   T_NUMBER,   },
{ "+",                      C_PLUS,         S_2_0,  2,  T_NUMBER,   T_NUMBER,   },
{ "-",                      C_MINUS,        S_2_0,  2,  T_NUMBER,   T_NUMBER,   },
{ ">>",                     C_RSHIFT,       S_2_0,  2,  T_NUMBER,   T_NUMBER,   },
{ "<<",                     C_LSHIFT,       S_2_0,  2,  T_NUMBER,   T_NUMBER,   },
{ "<",                      C_LESS,         S_2_0,  2,  T_NUMBER,   T_NUMBER,   },
{ ">",                      C_GREATER,      S_2_0,  2,  T_NUMBER,   T_NUMBER,   },
{ "==",                     C_EQUAL,        S_2_0,  2,  T_NUMBER,   T_NUMBER,   },
{ "!=",                     C_NOTEQUAL,     S_2_0,  2,  T_NUMBER,   T_NUMBER,   },
{ "&",                      C_AND,          S_2_0,  2,  T_NUMBER,   T_NUMBER,   },
{ "^",                      C_XOR,          S_2_0,  2,  T_NUMBER,   T_NUMBER,   },
{ "|",                      C_OR,           S_2_0,  2,  T_NUMBER,   T_NUMBER,   },
{ "&&",                     C_ANDAND,       S_2_0,  2,  T_NUMBER,   T_NUMBER,   },
{ "||",                     C_OROR,         S_2_0,  2,  T_NUMBER,   T_NUMBER,   },
{ "=",                      C_GETS,         S_1_0,  0,                          },
{ "p",                      C_PRINT,        S_1_0,  0,                          },
{ "pop",                    C_POP,          0,      0,                          },
{ "clrstk",                 C_CLRSTK,       0,      0,                          },
{ "dup",                    C_DUP,          S_0_1,  0,                          },
{ "nonverbose",             C_NOVERBOSE,    0,      0,                          },
{ "verbose",                C_VERBOSE,      0,      0,                          },
{ "ibase",                  C_INBASE,       S_1_0,  1,  T_NUMBER,               },
{ "ooctal",                 C_OUTOCTAL,     0,      0,                          },
{ "odecimal",               C_OUTDECIMAL,   0,      0,                          },
{ "ohex",                   C_OUTHEX,       0,      0,                          },
{ "ibinary",                C_INBINARY,     0,      0,                          },
{ "ioctal",                 C_INOCTAL,      0,      0,                          },
{ "idecimal",               C_INDECIMAL,    0,      0,                          },
{ "ihex",                   C_INHEX,        0,      0,                          },
{ "stk",                    C_DUMPSTACK,    0,      0,                          },
{ "vars",                   C_VARS,         0,      0,                          },
{ "stackdump",              C_KERNELSTACKDUMP, 0,   0,                          },
{ "findsym",                C_FINDSYM,      S_1_0,  1,  T_NUMBER,               },
{ "pinode",                 C_PINODE,       S_1_0,  1,  T_NUMBER,               },
{ "sysdump",                C_SYSDUMP,      0,      0, },
{ "dump",                   C_DUMP,         S_2_0,  2,  T_NUMBER,       T_NUMBER, },
{ "stack",                  C_STACK,        0,      0,   },
{ "db0",                    C_DR0,          S_0_1,  0,  },
{ "db1",                    C_DR1,          S_0_1,  0,  },
{ "db2",                    C_DR2,          S_0_1,  0,  },
{ "db3",                    C_DR3,          S_0_1,  0,  },
{ "db6",                    C_DR6,          S_0_1,  0,  },
{ "db7",                    C_DR7,          S_0_1,  0,  },
{ "wdb0",                   C_WDR0,         S_1_0,  1,  T_NUMBER,               },
{ "wdb1",                   C_WDR1,         S_1_0,  1,  T_NUMBER,               },
{ "wdb2",                   C_WDR2,         S_1_0,  1,  T_NUMBER,               },
{ "wdb3",                   C_WDR3,         S_1_0,  1,  T_NUMBER,               },
{ "wdb6",                   C_WDR6,         S_1_0,  1,  T_NUMBER,               },
{ "wdb7",                   C_WDR7,         S_1_0,  1,  T_NUMBER,               },
{ "%ds",                    C_GETDS,        S_0_1,  0,  },
{ "%es",                    C_GETES,        S_0_1,  0,  },
{ "%fs",                    C_GETFS,        S_0_1,  0,  },
{ "%gs",                    C_GETGS,        S_0_1,  0,  },
{ "%edi",                   C_GETDI,        S_0_1,  0,  },
{ "%esi",                   C_GETSI,        S_0_1,  0,  },
{ "%ebp",                   C_GETBP,        S_0_1,  0,  },
{ "%esp",                   C_GETSP,        S_0_1,  0,  },
{ "%ebx",                   C_GETBX,        S_0_1,  0,  },
{ "%edx",                   C_GETDX,        S_0_1,  0,  },
{ "%ecx",                   C_GETCX,        S_0_1,  0,  },
{ "%eax",                   C_GETAX,        S_0_1,  0,  },
{ "%err",                   C_GETER,        S_0_1,  0,  },
{ "%cs",                    C_GETCS,        S_0_1,  0,  },
{ "%ip",                    C_GETIP,        S_0_1,  0,  },
{ "%efl",                   C_GETFL,        S_0_1,  0,  },
{ ".trap",                  C_GETTP,        S_0_1,  0,  },
{ "saveregs",               C_SAVEREGS,     S_2_0,  2,  T_NUMBER,       T_NUMBER, },
{ "useregs",                C_USEREGS,      S_1_0,  1,  T_NUMBER   },
{ ".i",			    C_I,	S_0_1,  0,  },
{ ".a",			    C_A,	S_0_1,  0,  },
{ ".m",			    C_M,	S_0_1,  0,  },
{ ".aw",		    C_AW,	S_0_1,  0,  },
{ ".mw",		    C_MW,	S_0_1,  0,  },
{ ".al",		    C_AL,	S_0_1,  0,  },
{ ".ml",		    C_ML,	S_0_1,  0,  },
{ ".clr",		    C_NOBRK,	S_0_1,	0,  },
{ "brk0",		    C_BRK0,	S_2_0,	2,T_NUMBER,	T_NUMBER,  },
{ "brk1",		    C_BRK1,	S_2_0,	2,T_NUMBER,	T_NUMBER,  },
{ "brk2",		    C_BRK2,	S_2_0,	2,T_NUMBER,	T_NUMBER,  },
{ "brk3",		    C_BRK3,	S_2_0,	2,T_NUMBER,	T_NUMBER,  },
{ "trc0",		    C_TRC0,	S_1_0,	1,T_NUMBER, },
{ "trc1",		    C_TRC1,	S_1_0,	1,T_NUMBER, },
{ "trc2",		    C_TRC2,	S_1_0,	1,T_NUMBER, },
{ "trc3",		    C_TRC3,	S_1_0,	1,T_NUMBER, },
{ "db?",		    C_DBSTAT,	0,	0, },
{ "help",		    C_HELP,	0,	0, },
    };


dbinterp()
{
    short t;

    for (;;) {
	t = dbgetitem(&dbstack[dbtos]);
	if (dbverbose) {
	    printf("%x: ", dbtos);
	    dbprintitem(&dbstack[dbtos]);
	}

	switch (t) {
	case EOF:
	    return;
	    break;
	case (int) NULL:
	    break;
	case NUMBER:
	    push(1);
	    break;
	case STRING:
	    push(1);
	    break;
	case NAME:
	    if (doname(dbstack[dbtos].value.string, 0) < 0)
		return;
	    break;
	default:
	    break;
	}
    }
}

static
push(n)
    int n;
{
    if ((DBSTKSIZ - (dbtos + n)) <  1) {
	dberror("stack overflow on push");
	return;
    }
    dbtos += n;
}


static
pop(n)
    int n;
{
    if (n > dbtos) {
	dberror("not enough items on stack to pop");
	return;
    }
    while (--n >= 0) {
	dbtos--;
	if (dbstack[dbtos].type == STRING)
	    dbstrfree(dbstack[dbtos].value.string);
	dbstack[dbtos].type = NULL;
    }
}

dbstackcheck(down, up)
    int down, up;
{
    if (down > dbtos) {
	dberror("not enough items on stack");
	return 1;
    }
    if ((DBSTKSIZ - (dbtos + up)) <  1) {
	dberror("stack overflow");
	return 1;
    }
    return 0;
}

dbtypecheck(x, t)
    unsigned int x, t;
{
    if (dbstack[x].type == t)
	return 0;
    dberror("bad operand type");
    if (t > TYPEMAX || dbstack[x].type > TYPEMAX)
	printf("*** logic error - illegal item type number in typecheck()\n");
    else
	printf("(operand at stack location %x is a %s and should be a %s)\n",
		x, typename[dbstack[x].type], typename[t]);
    return 1;
}

dbmasktypecheck(x, t)
    unsigned int x, t;
{
    if (t & T_NUMBER && dbstack[x].type == T_NUMBER) return 0;
    if (t & T_NAME   && dbstack[x].type == T_NAME)   return 0;
    if (t & T_STRING && dbstack[x].type == T_STRING) return 0;
    dberror("bad operand type");
    if (!(t & (T_NUMBER|T_NAME|T_STRING)) || dbstack[x].type > TYPEMAX)
	printf("*** logic error - illegal item type number in typecheck()\n");
    else
	printf("(operand at stack location %x is a %s and should be a %s %s %s)\n",
		x, typename[dbstack[x].type],
		    t&T_NUMBER ? typename[NUMBER] : "",
		    t&T_NAME   ? typename[NAME]   : "",
		    t&T_STRING ? typename[STRING] : "" );
    return 1;
}

void
dbprintitem(ip)
    struct item *ip;
{
    ushort t = ip->type;

    if (t > TYPEMAX) {
	printf("*** logic error - illegal item type number = %x\n", t);
	return;
    }
    if (dbverbose)
	printf("%s = ", typename[t]);

    switch (t) {
    case (int) NULL:
	break;
    case NUMBER:
	printf(outformat, ip->value.number);
	break;
    case STRING:
	printf("\"%s\"", ip->value.string);
	break;
    case NAME:
	printf("%s", ip->value.string);
	break;
    }
    printf("\n");
}


doname(name, check)
    char *name;     /* name to process */
    int check;      /* if set, just check for existence of name */
{
    register struct cmdentry *p;
    char *s;
    short i;
    ushort n;

    for (p=cmdtable; p < &cmdtable[C_MAXCMDS]; p++) {
	if ( p->name[0] == name[0] && streq(name, p->name)) {
		/* Found a command */
	    if (!check) {   /* check -> no action, testing */
		    /* Check for stack growth limits */
		if (dbstackcheck(S_DOWN(p->stackcheck),S_UP(p->stackcheck))) {
		    goto bad;
		} else {
			/* Check for correct parameter types */
		    for (i=0; i < p->parmcnt; i++) {
			if (dbmasktypecheck(dbtos-1-i, p->parmtypes[i])) {
			    goto bad;
			}
		    }
		    switch ( p->index ) {
		    case C_READBYTE:
			dbstack[dbtos-1].value.number =
			    *(uchar *)dbstack[dbtos-1].value.number;
			break;
		    case C_READWORD:
			dbstack[dbtos-1].value.number =
			    *(ushort *)dbstack[dbtos-1].value.number;
			break;
		    case C_READLONG:
			dbstack[dbtos-1].value.number =
			    *(ulong *)dbstack[dbtos-1].value.number;
			break;
		    case C_WRITEBYTE:
			*(uchar *)dbstack[dbtos-1].value.number =
			    (uchar)dbstack[dbtos-2].value.number;
			dbtos -= 2;
			break;
		    case C_WRITEWORD:
			*(ushort *)dbstack[dbtos-1].value.number =
			    (ushort)dbstack[dbtos-2].value.number;
			dbtos -= 2;
			break;
		    case C_WRITELONG:
			*(ulong *)dbstack[dbtos-1].value.number =
			    (ulong)dbstack[dbtos-2].value.number;
			dbtos -= 2;
			break;
		    case C_FINDSYM:
			findsymname(dbstack[dbtos-1].value.number, 1);
			dbtos--;
			break;
		    case C_PINODE:
			db_pinode(dbstack[dbtos-1].value.number);
			dbtos--;
			break;
		    case C_KERNELSTACKDUMP:
			stackdump(firstarg);
			break;
		    case C_BANG:
			dbstack[dbtos-1].value.number = ! dbstack[dbtos-1].value.number;
			break;
		    case C_PLUSPLUS:
			dbstack[dbtos-1].value.number++;
			break;
		    case C_MINUSMINUS:
			dbstack[dbtos-1].value.number--;
			break;
		    case C_STAR:
			dbstack[dbtos-2].value.number *= dbstack[dbtos-1].value.number;
			goto cleanup_binop;
		    case C_DIV:
			dbstack[dbtos-2].value.number /= dbstack[dbtos-1].value.number;
			goto cleanup_binop;
		    case C_PERCENT:
			dbstack[dbtos-2].value.number %= dbstack[dbtos-1].value.number;
			goto cleanup_binop;
		    case C_PLUS:
			dbstack[dbtos-2].value.number += dbstack[dbtos-1].value.number;
			goto cleanup_binop;
		    case C_MINUS:
			dbstack[dbtos-2].value.number -= dbstack[dbtos-1].value.number;
			goto cleanup_binop;
		    case C_RSHIFT:
			n = dbstack[dbtos-1].value.number & 0x1f;
			dbstack[dbtos-2].value.number >>= n;
			goto cleanup_binop;
		    case C_LSHIFT:
			n = dbstack[dbtos-1].value.number & 0x1f;
			dbstack[dbtos-2].value.number <<= n;
			goto cleanup_binop;
		    case C_LESS:
			dbstack[dbtos-2].value.number =
			  dbstack[dbtos-2].value.number < dbstack[dbtos-1].value.number;
			goto cleanup_binop;
		    case C_GREATER:
			dbstack[dbtos-2].value.number =
			  dbstack[dbtos-2].value.number > dbstack[dbtos-1].value.number;
			goto cleanup_binop;
		    case C_EQUAL:
			dbstack[dbtos-2].value.number =
			  dbstack[dbtos-2].value.number == dbstack[dbtos-1].value.number;
			goto cleanup_binop;
		    case C_NOTEQUAL:
			dbstack[dbtos-2].value.number =
			  dbstack[dbtos-2].value.number != dbstack[dbtos-1].value.number;
			goto cleanup_binop;
		    case C_AND:
			dbstack[dbtos-2].value.number &= dbstack[dbtos-1].value.number;
			goto cleanup_binop;
		    case C_XOR:
			dbstack[dbtos-2].value.number ^= dbstack[dbtos-1].value.number;
			goto cleanup_binop;
		    case C_OR:
			dbstack[dbtos-2].value.number |= dbstack[dbtos-1].value.number;
			goto cleanup_binop;
		    case C_ANDAND:
			dbstack[dbtos-2].value.number =
			  dbstack[dbtos-2].value.number && dbstack[dbtos-1].value.number;
			goto cleanup_binop;
		    case C_OROR:
			dbstack[dbtos-2].value.number =
			  dbstack[dbtos-2].value.number || dbstack[dbtos-1].value.number;
	    cleanup_binop:
			dbtos--;
			dbstack[dbtos].type = NULL;
			break;
		    case '=':   /* assignment to a variable */
			i = dbgetitem(&dbstack[dbtos]); /* get variable name*/
			if (dbverbose) {
			    printf("%x: ", dbtos);
			    dbprintitem(&dbstack[dbtos]);
			}
			if (i == EOF)
			    return -1;
			if (dbtypecheck(dbtos, NAME))
			    goto bad;
			s = dbstack[dbtos].value.string;
			if (doname(s, 1)) {
			    dberror("name already used");
			    goto bad;
			}
			for (i = 0, n = VARTBLSIZ; i < VARTBLSIZ; i++) {
			    if (variable[i].name == NULL) {
				if (n == VARTBLSIZ)
				    n = i;          /* save free table slot */
			    }
			    else if (streq(variable[i].name, s))    /* name found */
				break;
			}
			if (i == VARTBLSIZ) {       /* name not found */
			    if (n == VARTBLSIZ) {
				dberror("variable table overflow");
				goto bad;
			    }
			    i = n;                  /* new slot */
			    variable[i].name = dbstrdup(s);
			}
			else if (variable[i].item.type == STRING)   /* existing string */
			    dbstrfree(variable[i].item.value.string); /* free string */
			variable[i].item = dbstack[dbtos-1];
			dbtos--;
			break;

		    case C_PRINT:
			if (dbverbose) {
			    printf("%x: ", dbtos-1);
			}
			dbprintitem(&dbstack[dbtos-1]);
			break;
		    case C_POP:
			pop(1);
			break;
		    case C_CLRSTK:
			pop(dbtos);
			break;
		    case C_DUP:
			dbstack[dbtos] = dbstack[dbtos-1];
			if (dbstack[dbtos].type == STRING) {
			    s = dbstrdup(dbstack[dbtos].value.string);
			    dbstack[dbtos].value.string = s;    /* 286 compiler bug */
			}
			dbtos++;
			break;
		    case C_NOVERBOSE:
			dbverbose = 0;
			break;
		    case C_VERBOSE:
			dbverbose = 1;
			break;
		    case C_INBASE:
			n = dbstack[dbtos-1].value.number;
			if (n < 2 || n > 16) {
			    dberror("illegal input base value - 2 thru 16 accepted");
			    goto bad;
			}
			dbibase = n;
			dbtos--;
			dbstack[dbtos].type = NULL;
			break;
		    case C_OUTOCTAL:
			dbobase = 8;
			outformat = "%lo";
			break;
		    case C_OUTDECIMAL:
			dbobase = 10;
			outformat = "%ld";
			break;
		    case C_OUTHEX:
			dbobase = 16;
			outformat = "%lx";
			break;
		    case C_INBINARY:
			dbibase = 2;
			break;
		    case C_INOCTAL:
			dbibase = 8;
			break;
		    case C_INDECIMAL:
			dbibase = 10;
			break;
		    case C_INHEX:
			dbibase = 16;
			break;
		    case C_DUMPSTACK:
			for (i = 0; i < dbtos; i++) {
			    if (dbverbose)
				printf("%x: ", i);
			    dbprintitem(&dbstack[i]);
			}
			break;
		    case C_VARS:
			for (i = 0; i < VARTBLSIZ; i++) {
			    if (variable[i].name != NULL) {
				if (dbverbose)
				    printf("%x: ", i);
				printf("%s = ", variable[i].name);
				dbprintitem(&variable[i].item);
			    }
			}
			break;
		    case C_SYSDUMP:
			sysdump(); break;
		    case C_DUMP:
			db_dump( dbstack[dbtos-1].value.number,
			    dbstack[dbtos-2].value.number );
			dbtos -= 2;
			break;
		    case C_STACK:
			db_stacktrace( 0L ); break;
		    case C_DR0:
			dbstack[dbtos].value.number = _dr0();
			dbstack[dbtos].type = T_NUMBER;
			dbtos++;
			break;
		    case C_DR1:
			dbstack[dbtos].value.number = _dr1();
			dbstack[dbtos].type = T_NUMBER;
			dbtos++;
			break;
		    case C_DR2:
			dbstack[dbtos].value.number = _dr2();
			dbstack[dbtos].type = T_NUMBER;
			dbtos++;
			break;
		    case C_DR3:
 			dbstack[dbtos].value.number = _dr3();
			dbstack[dbtos].type = T_NUMBER;
			dbtos++;
			break;
		    case C_DR6:
			dbstack[dbtos].value.number = _dr6();
			dbstack[dbtos].type = T_NUMBER;
			dbtos++;
			break;
		    case C_DR7:
			dbstack[dbtos].value.number = _dr7();
			dbstack[dbtos].type = T_NUMBER;
			dbtos++;
			break;
		    case C_WDR0:
			_wdr0(dbstack[dbtos-1].value.number);
			dbtos--;
			dbstack[dbtos].type = NULL;
			break;
		    case C_WDR1:
			_wdr1(dbstack[dbtos-1].value.number);
			dbtos--;
			dbstack[dbtos].type = NULL;
			break;
		    case C_WDR2:
			_wdr2(dbstack[dbtos-1].value.number);
			dbtos--;
			dbstack[dbtos].type = NULL;
			break;
		    case C_WDR3:
			_wdr3(dbstack[dbtos-1].value.number);
			dbtos--;
			dbstack[dbtos].type = NULL;
			break;
		    case C_WDR6:
			_wdr6(dbstack[dbtos-1].value.number);
			dbtos--;
			dbstack[dbtos].type = NULL;
			break;
		    case C_WDR7:
			_wdr7(dbstack[dbtos-1].value.number);
			dbtos--;
			dbstack[dbtos].type = NULL;
			break;
		    case C_GETDS:
			dbstack[dbtos].value.number = get_ds();
			dbstack[dbtos].type = T_NUMBER;
			dbtos++;
			break;
		    case C_GETES:
			dbstack[dbtos].value.number = get_es();
			dbstack[dbtos].type = T_NUMBER;
			dbtos++;
			break;
		    case C_GETFS:
			dbstack[dbtos].value.number = get_fs();
			dbstack[dbtos].type = T_NUMBER;
			dbtos++;
			break;
		    case C_GETGS:
			dbstack[dbtos].value.number = get_gs();
			dbstack[dbtos].type = T_NUMBER;
			dbtos++;
			break;
		    case C_GETDI:
			dbstack[dbtos].value.number = get_di();
			dbstack[dbtos].type = T_NUMBER;
			dbtos++;
			break;
		    case C_GETSI:
			dbstack[dbtos].value.number = get_si();
			dbstack[dbtos].type = T_NUMBER;
			dbtos++;
			break;
		    case C_GETBP:
			dbstack[dbtos].value.number = get_bp();
			dbstack[dbtos].type = T_NUMBER;
			dbtos++;
			break;
		    case C_GETSP:
			dbstack[dbtos].value.number = get_sp();
			dbstack[dbtos].type = T_NUMBER;
			dbtos++;
			break;
		    case C_GETBX:
			dbstack[dbtos].value.number = get_bx();
			dbstack[dbtos].type = T_NUMBER;
			dbtos++;
			break;
		    case C_GETDX:
			dbstack[dbtos].value.number = get_dx();
			dbstack[dbtos].type = T_NUMBER;
			dbtos++;
			break;
		    case C_GETCX:
			dbstack[dbtos].value.number = get_cx();
			dbstack[dbtos].type = T_NUMBER;
			dbtos++;
			break;
		    case C_GETAX:
			dbstack[dbtos].value.number = get_ax();
			dbstack[dbtos].type = T_NUMBER;
			dbtos++;
			break;
		    case C_GETTP:
			dbstack[dbtos].value.number = get_tp();
			dbstack[dbtos].type = T_NUMBER;
			dbtos++;
			break;
		    case C_GETER:
			dbstack[dbtos].value.number = get_er();
			dbstack[dbtos].type = T_NUMBER;
			dbtos++;
			break;
		    case C_GETIP:
			dbstack[dbtos].value.number = get_ip();
			dbstack[dbtos].type = T_NUMBER;
			dbtos++;
			break;
		    case C_GETCS:
			dbstack[dbtos].value.number = get_cs();
			dbstack[dbtos].type = T_NUMBER;
			dbtos++;
			break;
		    case C_GETFL:
			dbstack[dbtos].value.number = get_fl();
			dbstack[dbtos].type = T_NUMBER;
			dbtos++;
			break;
		    case C_SAVEREGS:
			db_saveregs( dbstack[dbtos-1].value.number,
			    dbstack[dbtos-2].value.number );
			dbtos -= 2;
			break;
		    case C_USEREGS:
			db_useregs(dbstack[dbtos-1].value.number);
			dbtos--;
			dbstack[dbtos].type = NULL;
			break;
		    case C_I:
			dbstack[dbtos].value.number = 0;
			dbstack[dbtos].type = T_NUMBER;
			dbtos++;
			break;
		    case C_A:
			dbstack[dbtos].value.number = 3;
			dbstack[dbtos].type = T_NUMBER;
			dbtos++;
			break;
		    case C_M:
			dbstack[dbtos].value.number = 1;
			dbstack[dbtos].type = T_NUMBER;
			dbtos++;
			break;
		    case C_AW:
			dbstack[dbtos].value.number = 7;
			dbstack[dbtos].type = T_NUMBER;
			dbtos++;
			break;
		    case C_MW:
			dbstack[dbtos].value.number = 5;
			dbstack[dbtos].type = T_NUMBER;
			dbtos++;
			break;
		    case C_AL:
			dbstack[dbtos].value.number = 15;
			dbstack[dbtos].type = T_NUMBER;
			dbtos++;
			break;
		    case C_ML:
			dbstack[dbtos].value.number = 13;
			dbstack[dbtos].type = T_NUMBER;
			dbtos++;
			break;
		    case C_NOBRK:
			dbstack[dbtos].value.number = 47;
			dbstack[dbtos].type = T_NUMBER;
			dbtos++;
			break;
		    case C_BRK0:
		    {
		       	    unsigned long oldreg, addr, type;

			    addr = dbstack[dbtos-2].value.number;
			    type = dbstack[dbtos-1].value.number;
			    dbtos -= 2;
			    oldreg = _dr7();
			    oldreg &=~ 0xf0002;
			    if ( type != 47 ) {
			        oldreg |= type << 16;
			        oldreg |= 2;
			    }
			    _wdr7(oldreg);
			    _wdr0(addr);
			    tcount[0] = 0;
			    break;
		    }
		    case C_BRK1:
		    {
		       	    unsigned long oldreg, addr, type;

			    addr = dbstack[dbtos-2].value.number;
			    type = dbstack[dbtos-1].value.number;
			    dbtos -= 2;
			    oldreg = _dr7();
			    oldreg &=~ 0xf00008;
			    if ( type != 47 ) {
			        oldreg |= type << 20;
			        oldreg |= 8;
			    }
			    _wdr7(oldreg);
			    _wdr1(addr);
			    tcount[1] = 0;
			    break;
		    }
		    case C_BRK2:
		    {
		       	    unsigned long oldreg, addr, type;

			    addr = dbstack[dbtos-2].value.number;
			    type = dbstack[dbtos-1].value.number;
			    dbtos -= 2;
			    oldreg = _dr7();
			    oldreg &=~ 0xf000020;
			    if ( type != 47 ) {
			        oldreg |= type << 24;
			        oldreg |= 0x20;
			    }
			    _wdr7(oldreg);
			    _wdr2(addr);
			    tcount[2] = 0;
			    break;
		    }
		    case C_BRK3:
		    {
		       	    unsigned long oldreg, addr, type;

			    addr = dbstack[dbtos-2].value.number;
			    type = dbstack[dbtos-1].value.number;
			    dbtos -= 2;
			    oldreg = _dr7();
			    oldreg &=~ 0xf0000080;
			    if ( type != 47 ) {
			        oldreg |= type << 28;
			        oldreg |= 0x80;
			    }
			    _wdr7(oldreg);
			    _wdr3(addr);
			    tcount[3] = 0;
			    break;
		    }
		    case C_TRC0:
			    tcount[0] = dbstack[--dbtos].value.number; break;
		    case C_TRC1:
			    tcount[1] = dbstack[--dbtos].value.number; break;
		    case C_TRC2:
			    tcount[2] = dbstack[--dbtos].value.number; break;
		    case C_TRC3:
			    tcount[3] = dbstack[--dbtos].value.number; break;
		    case C_DBSTAT:
		    {
			int i;
			unsigned long dr[4];
			dr[0] = _dr0(); dr[1] = _dr1();
			dr[2] = _dr2(); dr[3] = _dr3();
			for ( i = 0; i < 4; i++ ) {
				int d7;
				printf( "%d: 0x%x(%s+0x%x) ", i, dr[i],
				  findsymname( dr[i], 0 ),
				  dr[i] - findsymaddr(findsymname(dr[i],0))
			        );
				d7 = _dr7();
				if ( (d7 & (3<<(2*i))) == 0 )
					printf( "OFF " );
				else {
					if ( tcount[i] )
						printf( "ON 0x%x ", tcount[i] );
					else
						printf( "ON " );
				}
				d7 >>= (16+4*i); d7 &= 0xf;
				switch ( d7 & 3 ) {
				case 0: printf( ".i" );
					if ( d7 & 15 )
						printf( "?" );
					break;
				case 1: printf( ".m" ); break;
				case 3: printf( ".a" ); break;
				default: printf( ".?" ); break;
				}
				switch ( d7 & 12 ) {
				case 4: printf( "w" ); break;
				case 12: printf( "l" ); break;
				case 0: ; break;
				default: printf( "?" ); break;
				}
				printf( "\n" );
			}
			break;
		    }
		    case C_HELP:
				help(); break;
		    default:
			printf("DEBUGGER error: no code for %s (index %d)\n",
			    name, p->index);
			break;
		    }
		}
	    }
	    return 1;
	}
    }


    if (check)          /* check only applies to builtin operation names */
	return 0;

    for (i = 0; i < VARTBLSIZ; i++) {
	if ((s = variable[i].name) != NULL && streq(name, s)) {
	    if (dbstackcheck(0, 1))
		goto bad;
	    dbstack[dbtos] = variable[i].item;
	    if (dbstack[dbtos].type == STRING &&
		  (dbstack[dbtos].value.string = dbstrdup(dbstack[dbtos].value.string))
		     == NULL)
		goto bad;
	    dbtos++;
	    return 1;
	}
    }

    if (dbextname(name))        /* try external names */
	return 0;

notfound: /* name not found */
    if (! check)
	dberror("name not found");
    return 0;

bad: /* name found, error exit */
    return 1;
}

help() {
	printf( "%s\n",
"ADDR COUNT dump	show COUNT bytes starting at ADDR\n"
);
	printf( "%s\n",
"ADDR TYPE brk#		set breakpoint at ADDR of type TYPE using reg #"
);
	printf( "%s\n",
"		TYPE == .i for instruction, .a for access, or .m for modify"
);
	printf( "%s\n",
"                       .a or .m can be follwed by w or l for worl or long"
);
	printf( "%s\n",
"COUNT trc#		print a message but don't enter debugger on first"
);
	printf( "%s\n",
"                       CNT instances of breakpoint #"
);
	printf( "%s\n",
"db?			show breakpoints"
);
	printf( "%s\n",
"stack			show stack trace"
);
	printf( "%s\n",
"ADDR findsym		print name of symbol nearest ADDR"
);
}
#endif /* DEBUGGER */
