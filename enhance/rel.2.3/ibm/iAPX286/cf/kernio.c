#ifndef lint
static char *uportid = "@(#)kernio.c	1.5 Microport 9/30/87";
#endif

/*
 * kernio - link kernel i/o references to proper drivers
 *
 *	uport!mike Wed Sep 16 11:57:14 PDT 1987
 *		Created.
 */

#ifdef	MP386
#include "sys/conf.h"
#define	PUTCHAR	kdcputc
#define	KD_0
int kdcputc (); 		 	/* console putchar () */
struct conssw conssw = { kdcputc , 0 };

#else	/* 286 */
#include "config.h"
#define	PUTCHAR	putchar
#endif

#ifndef	KD_0			/* no KD driver installed	*/
/*
 *! === kernel printf redirected to COM1: (0x3F8) ===
 *! === debugger i/o  redirected to COM1: (0x3F8) ===
 */
int kernputc = 0x3f8;		/* kernel putchar device	*/
int kdbgputc = 0x3F8;		/* debugger putchar device	*/
int kdbggetc = 0x3F8;		/* debugger getchar device	*/
#else
#ifdef	MERGE386
/*
 *! === kernel printf directed to Console Display ===
 *! === debugger i/o  redirected to COM1: (0x3F8) ===
 */
int kernputc = 0;		/* kernel putchar device	*/
int kdbgputc = 0x3F8;		/* debugger putchar device	*/
int kdbggetc = 0x3F8;		/* debugger getchar device	*/
#else
/*
 *! === kernel printf directed to Console Display ===
 *! === debugger i/o  directed to Console Display ===
 */
int kernputc = 0;		/* kernel putchar device	*/
int kdbgputc = 0;		/* debugger putchar device	*/
int kdbggetc = 0;		/* debugger getchar device	*/
#endif

#include "sys/types.h"
#include "sys/tty.h"		/* get size of tty struct	*/
struct tty kd_tty  [KD_0];	/* generate tty struct's	*/
int kd_numttys = KD_0;		/* number of requested consoles */
#endif


/* Kernel put character */
PUTCHAR (ch)
    int ch;
{
    int dev;
again:
    if (dev = kernputc)
	asy_putc (dev, ch);

#ifdef	KD_0
    else
	kd_putc (ch);
#endif

#ifndef	MP386
    if (ch == '\n') {	/* 286 kernel put NL is followed by put CR */
	ch = '\r';
	goto again;
    }
#endif

#ifdef	REMOVE	/* Doesn't work */
    /* implement ^S/^Q control flow */
    if (kernputc) {
	if (asy_getc (kernputc, 0) == '\023')		/* if ^S */
	    while (asy_getc (kernputc, != '\021')	/* wait for ^Q */
		;
    }
    else {
	if (kd_getc (0) == '\023')		/* if ^S */
	    while (kd_getc (1) != '\021')	/* wait for ^Q */
		;
    }
#endif
}

/* Debugger get character */
asygetchar (wait)
    int wait;
{
    int dev;

    if (dev = kdbggetc)
	return asy_getc (dev, wait);		/* get char from tty */

#ifdef	KD_0
    else {
	kdcinit ();				/* insure it's init'd */
	return kd_getc (wait);			/* get char from keyboard */
    }
#endif
}

/* Debugger put character */
asyputchar (ch)
{
    int dev;

again:
    if (dev = kdbgputc)
	asy_putc (dev, ch);			/* put char to tty */
#ifdef	KD_0
    else
	kd_putc (ch);				/* put char to console */
#endif

    if (ch == '\n') {
	ch = '\r';
	goto again;
    }

#ifdef	REMOVE	/* Doesn't work */
    /* implement ^S/^Q */
    if ((ch = asygetchar (0) != 0xff) && ch == '\023')
	while (asygetchar (1) != '\021')
	    ;
#endif
}

/* === */
