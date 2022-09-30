/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident  "@(#)db.c	1.11"

#ifdef DEBUGGER

#include "sys/types.h"
#include "sys/param.h"
#include "sys/signal.h"
#include "sys/dir.h"
#include "sys/user.h"
#include "sys/reg.h"
#include "sys/cmn_err.h"
#include "debugger.h"

char dbactive;
ushort *firstarg;

char *findsymname();
long findsymaddr();
label_t dbsave;
extern int panic_level;
int save_panic_level;
int tcount[4];		/* trace count for each debug register */

debugger(argp,why)
    ushort *argp;
    int why;
{
    asm(" pushf");
    asm(" cli");
    if ( why == 1 ) {
	int d6;			/* contents of DR6 */
	int i;
	int enter;
	/*
 	 * called because of trap # 1
	 */

	d6 = _dr6();		/* read control register */
	for ( i = enter = 0; i < 4; i++ ) {
		if ( d6 & (1<<i) ) {
			if ( tcount[i] && (d6 & (1<<i)) ) {
				printf( "trace%d 0x%x\n", i, tcount[i]-- );
			} else {
				enter = 1;
				printf( "break%d\n", i );
			}
		}
	}
	_wdr6(0);
	if ( ! enter )
	    goto doret;
    }
    firstarg = argp;
    if ( dbactive ) {
	printf( "DEBUGGER: restarting debugger\n" );
	longjmp( dbsave );
    } else {
	save_panic_level = panic_level;
	setjmp( dbsave );
	panic_level = 0;
    }
    dbactive = 1;
    printf("DEBUGGER ; ebp: 0x%x\n", (unsigned) (&argp) - 8);
    dbkbinit();

    dbinterp();

    printf("DEBUGGER exiting\n");
    dbactive = 0;
    panic_level = save_panic_level;
doret:
    asm(" popf");
}


dbintr(dummy)
{
	cmn_err(CE_NOTE, "entering debugger\n");
	debugger(&dummy,0);
	cmn_err(CE_NOTE, "leaving debugger\n" );
}

dbextname(name)          /* look for external names in kernel debugger */
    char *name;
{
    if ((dbstack[dbtos].value.number = findsymaddr(name)) != NULL) {
	if (dbstackcheck(0, 1))
	    return 1;
	dbstack[dbtos].type = NUMBER;
	dbtos++;
	return 1;
    }
    return 0;   /* not found */
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
    long value;
    int tell;   /* flag to print name and location */
{
	char *p = symtable;
	char *oldp = p;
	long loc;

	while (*(long *)p && *(long *)p <= value)
	{
		oldp = p;
		p += sizeof(long);	/* jump past long */
		while (*p++);		/* jump past string */
	}
	p = oldp + sizeof(long);        /* name string */
	loc = *(long *)oldp;            /* address */
	if (tell) {
		printf("%s", p);
		if (loc != value)
			printf(" at %lx", loc);
		printf("\n");
	}
	return p;
}

/*
 * findsymaddr looks thru symtable to find the address of name.
 */

long
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
			return(*(long *)(oldp - sizeof(long)));
		while (*p++);		/* jump past rest of name */
		p += sizeof(long);	/* jump past long */
	}
	return 0L;
}

/* kernel stack dump - addresses increase right to left and bottom to top */

stackdump(tos)
    ushort *tos;            /* top-of-stack pointer */
{
    ushort *sp;             /* stack pointer */

    tos -= 3;                       /* bp */
    sp = tos;                       /* for stack selector */
    *(ushort *)&sp = KSTKSZ * 2;    /* offset of high end of stack */

    do {
	sp -= 8;
	printf("%.4x %.4x  %.4x %.4x  %.4x %.4x  %.4x %.4x    %.4x\n",
		*(sp+7),*(sp+6),*(sp+5),*(sp+4),
		*(sp+3),*(sp+2),*(sp+1),*(sp),
		(ushort)sp);
    } while (sp > tos);
    printf("bp at %x\n", (ushort)tos);
}
#endif /* DEBUGGER */
#ifdef GDEBUGGER
dbintr()
{
}
#endif
