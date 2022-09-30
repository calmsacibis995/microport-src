
/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)dblex.c	1.3"

#ifdef DEBUGGER

#include "sys/types.h"
#include "sys/param.h"
#include "debugger.h"

#define LCASEBIT    0x20
#define isspace(c) ((c) == ' ' || (c) == '\t' || (c) == '\n')

static ushort column;
static ushort linx;                     /* input line index */
static uchar linbuf[LINBUFSIZ];
static char strbuf[MAXSTRSIZ+1];
static char digit[] = "0123456789abcdef";

ushort dbibase = 16;


short
dbgetchar()
{
    short c;

    while ((c = linbuf[linx++]) == 0) {
	column = linx = 0;
	if (dbgets(linbuf, LINBUFSIZ) == NULL) {
	    linbuf[0] = 0;
	    return(EOF);
	}
    }
    if (c == '\t')
	column = ((column + 8) & ~7);   /* update column */
    else
	column++;
    return c;
}

void
dberror(s)
    char *s;
{
    printf("\r");
    while (--column)
	  printf(" ");
    printf("^ ERROR: %s\n", s);
    column = linx = 0;
    linbuf[0] = 0;
}

short
dbgetitem(ip)
    struct item *ip;
{
    ulong n = 0;
    short c, cn, nc;
    short i;
    ushort d;
    char *s;
    ushort base = dbibase;
    char negative = 0;

    /* switch on first character */

    ip->type = NULL;
firstchar:
    c = dbgetchar();
    switch (c) {

    case EOF:    /* end of file */
	return EOF;

    case ' ':   /* blank */
    case '\t':  /* tab */
    case '\n':  /* newline */
	goto firstchar;                 /* skip over leading white space */

    case '-':   /* minus sign may modify number */
	c = dbgetchar();
	if (c > '9' || c < '0') {   /* if not a number, it's a name */
	    column--, linx--;
	    c = '-';
	    goto doname;
	}
	negative++;
	if (c != '0')
	    goto donumber;
	/* fall thru */

    case '0':    /* zero starts number, perhaps with modified base */
	c = dbgetchar();
	switch(c) {
	case 'b':
	    base = 2; break;
	case 'o':
	    base = 8; break;
	case 'd':
	    base = 10; break;
	case 'x':
	    base = 16; break;
	default:
	    goto donumber;
	}
	c = dbgetchar();
	/* fall thru */

    /* 1-9 starts number with default base */
    case '1': case '2': case '3': case '4': case '5':
    case '6': case '7': case '8': case '9':
donumber:
	for (;;)
	{
	    cn = c | LCASEBIT;
	    for (d = 0; d < base && cn != digit[d]; d++) ;
	    if (d == base)
		if (isspace(c))
		    break;
		else {
		    dberror("illegal digit in number");
		    printf("( base = x%x )\n", base);
		    goto baditem;
		}
	    n *= base;
	    n += d;
	    c = dbgetchar();
	}
	if (negative)
	    n = (ulong)-(long)n;
rtrncon:
	ip->value.number = n;
	return (ip->type = NUMBER);

    case '\'':    /* single quote starts character number */
	for (nc = 0; (c= dbgetchar()) != '\''; nc++) {
	    if (nc > 3) {
		dberror("more than 4 characters in character constant");
		goto baditem;
	    }
	    if (c != '\\')  /* if not escaped */
		cn = c;
	    else
		switch (c = dbgetchar()) {
		case EOF:
		    dberror("end of file in character number");
		    goto baditem;
		default: cn = c; break;
		case 'n': cn = '\n'; break;
		case 't': cn = '\t'; break;
		case 'v': cn = '\v'; break;
		case 'b': cn = '\b'; break;
		case 'r': cn = '\r'; break;
		case 'f': cn = '\f'; break;
		case '\\': cn = '\\'; break;
		case '\'': cn = '\''; break;
		case '0': case '1': case '2': case '3':
		case '4': case '5': case '6': case '7':
		    /* octal character number */
		    cn = c & 7;
		    for (i = 2; --i >= 0; ) {  /* max three octal digits */
			c = dbgetchar();
			if (c >= '0' && c <= '7')
			    cn = (cn << 3) | (c & 7);
			else {
			    column--, linx--;
			    break;
			}
		    }
		}
	    *((char*)&n + nc) = cn;
	}
	goto rtrncon;

    case '"':    /* double quote starts a string */
	for (s = strbuf; (c = dbgetchar()) != '"'; ) {
	    if ((s - strbuf) >= MAXSTRSIZ) {
		dberror("string too long");
		goto baditem;
	    }
	    if (c != '\\') {
		*s++ = c;
		continue;
	    }
	    /* backslash starts escape sequence */
	    switch (c = dbgetchar())
	    {
	    default: cn = c; break;
	    case EOF:
		dberror("end of file in string");
		goto baditem;
	    case '\n': continue;        /* ignore backslash-newline */
	    case 'n': cn = '\n'; break;
	    case 't': cn = '\t'; break;
	    case 'v': cn = '\v'; break;
	    case 'b': cn = '\b'; break;
	    case 'r': cn = '\r'; break;
	    case 'f': cn = '\f'; break;
	    case '\\': cn = '\\'; break;
	    case '\'': cn = '\''; break;
	    case '"': cn = '\"'; break;
	    case '0': case '1': case '2': case '3':
	    case '4': case '5': case '6': case '7':
		/* octal character specification */
		cn = c & 7;
		for (i = 2; --i >= 0; ) {      /* max three octal digits */
		    c = dbgetchar();
		    if (c >= '0' && c <= '7')
			cn = (cn << 3) | (c & 7);
		    else {
			column--, linx--;
			break;
		    }
		}
	    }
	    *s++ = cn;
	}
	*s++ = '\0';                            /* null terminated string */
	if ((ip->value.string = dbstrdup(strbuf)) == NULL)
	    goto baditem;
	return (ip->type = STRING);

    case '/':   /* slash may start comment */
	if ((c = dbgetchar()) != '*') {
	    column--, linx--;
	    c = '/';
	    goto doname;
	}
	for (;;) {
	    c = dbgetchar();
swcomment:
	    switch (c) {
	    case EOF:
		dberror("end of file in comment");
		goto baditem;
	    case '*':               /* possible end of comment */
		c = dbgetchar();
		if (c == '/')       /* end of comment */
		    goto firstchar;
		else
		    goto swcomment;
	    }
	}

    default: /* arbitrary name */
doname:
	s = strbuf;
	*s++ = c;
	for (;;) {
	    c = dbgetchar();
	    if (isspace(c))
		break;
	    if ((s - strbuf) >= MAXSTRSIZ)
	    {
		dberror("name too long");
		goto baditem;
	    }
	    *s++ = c;
	}
	*s++ = '\0';            /* null terminated string */
	ip->value.string = strbuf;
	return (ip->type = NAME);
    }

baditem:
    return NULL;
}


void
dbstrcpy(s1, s2)
    char *s1, *s2;
{
    while (*s1++ = *s2++) ;
}


dbstrcmp(s1, s2)
    char *s1, *s2;
{
    do {
	if (*s1 != *s2)
	    return (*s1 > *s2) ? 1 : -1;
    } while (s1++, *s2++) ;
    return 0;
}


char *
dbstrdup(s1)
    char *s1;
{
    char *s, *s2;

    for (s = s1; *s++; ) ;                      /* find end of string */
    if ((s2 = dbstralloc(s - s1)) != NULL)      /* allocate string space */
	dbstrcpy(s2, s1);                       /* copy string */
    return s2;
}


char *
dbstralloc(size)
    ushort size;
{
    char *s, *start, *maxstart, *end;
    char skipping = 0;
    static char strspc[STRSPCSIZ];  /* space for strings */

    if (size > STRSPCSIZ)
	goto out;
    start = strspc;
    maxstart = strspc + (STRSPCSIZ - size);

    while (start < maxstart) {
	if (*start) {
	    start++;
	    skipping = 1;
	    continue;
	}
	else if (skipping) {    /* must skip over null */
	    start++;
	    skipping = 0;
	    continue;
	}
	/* check available space here */
	for (s = start, end = start + size; ! *s; ) {
	    if (s++ == end) {
		if (dbverbose)
		    printf("dbstralloc() got %x bytes at %lx\n", size, start);
		return start;
	    }
	}
	start = s;
    }
out:
    dberror("out of string space");
    printf("( %x bytes requested )\n", size);
    return NULL;
}


void
dbstrfree(s)
    char *s;
{
    while (*s)
	*s++ = 0;
}


#endif /*DEBUGGER*/
