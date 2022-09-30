
/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)dbcon.c	1.4"

/*
 * kernel debugger console interface routines
 */

#include "sys/types.h"
#include "sys/param.h"
#ifdef	DEBUGGER
#include "sys/i8251.h"
#endif

#define EOT     0x04    /* ascii eot character */

#define XOFF    0x13
#define XON     0x11

#ifdef DEBUGGER
static ushort lastscancode;
char dbhold;    /* printf hold flag set by XOFF in s5conintr() or dbwait()*/

/*
 * dbgets reads a line from the debugging console using polled i/o
 */

char *
dbgets (buf, count)
    char *buf;
    short count;
{
    short c;
    short i;

    count--;
    for (i = 0; i < count; ) {
	while ((c = dbkbrd()) == -1) ;
	if (c == '\r')
	    c = '\n';
	putchar(c);
	if (c == '\b') {                /* backspace */
	    putchar(' ');
	    putchar('\b');
	    if (i > 0)
		    i--;
	    continue;
	}
	if (c == EOT && i == 0)         /* ctrl-D */
	    return NULL;
	buf[i++] = c;
	if (c == '\n')
	    break;
    }
    buf[i] = '\0';
    return (buf);
}


/*
 * dbwait - called by printf to wait if XOFF has been entered.
 * If so, turns off interrupts
 * and polls keyboard until XON is entered.
 */
dbwait()
{
    if (! dbhold) {
	if (dbkbrd() == XOFF) {
	    dbhold++;
	}
    }
    if (dbhold) {
	while (dbkbrd() != XON) ;
	dbhold = 0;
    }
}


dbkbinit()
{
}



dbkbrd()
{
extern struct i8251cfg i8251cfg[];

    if (!(inb(i8251cfg[0].u_cntrl) & S_RXRDY)) {
	dbpause((short)1);
	return -1;
    }
    return inb(i8251cfg[0].u_data) & 0xFF;
}


dbpause(seconds)
    short seconds;
{
    long n = (seconds * 100000L);

    while (--n) ;
}
#endif /* DEBUGGER */
#ifdef MB1
#ifdef GDEBUGGER
dbgkbrd(wait)
{
extern struct i8251cfg i8251cfg[];

retry:
    if (!(inb(i8251cfg[0].u_cntrl) & S_RXRDY)) {
	if (wait) {
		tenmicrosec();
		goto retry;
	} else
		return 0xff;
    }
    return inb(i8251cfg[0].u_data) & 0xFF;
}
#endif
#endif

