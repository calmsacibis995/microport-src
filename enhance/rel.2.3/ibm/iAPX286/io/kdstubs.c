#ifndef lint
static char *uportid = "@(#)kdstubs.c	3.0 Microport Rev 2.3 10/1/87";
#endif

/*
 * kernio - statisfy kernel i/o references in case the kd driver is
 * not installed.
 *
 *	uport!mike Wed Sep 16 11:57:14 PDT 1987
 *		Created.
 */

#include "sys/types.h"

#ifdef	MP386
#include "sys/conf.h"
#define	PUTCHAR	kdcputc
int kdcputc (); 		 	/* console putchar () */
struct conssw conssw = { kdcputc , 0 };
#else	/* 286 */
#define	PUTCHAR	putchar
#endif

int kdbgputc = 0x3F8;		/* debugger putchar device	*/
int kdbggetc = 0x3F8;		/* debugger getchar device	*/
int kernputc = 0x3F8;		/* kernel putchar device	*/

short kd_asyinitf [2] = { 0 };	/* asynch init flags		*/

/* Kernel put character */
PUTCHAR (ch)
    int ch;
{
    kd_asyputc (kernputc, ch);
#ifndef	MP386
    if (ch == '\n')	/* 286 kernel put NL is followed by put CR */
	kd_asyputc (kernputc, '\r');
#endif

#ifdef	REMOVE
    /* implement ^S/^Q control flow */
    if (kd_asygetc (kernputc, 0) == '\023')		/* if ^S */
	while (kd_asygetc (kernputc, != '\021')		/* wait for ^Q */
	    ;
#endif
}

/* Debugger get character */
asygetchar (wait)
    int wait;
{
    return kd_asygetc (kdbggetc, wait);		/* get char from tty */
}

/* Debugger put character */
asyputchar (ch)
{
    kd_asyputc (kdbgputc, ch);			/* put char to tty */
    if (ch == '\n')
	kd_asyputc (kdbgputc, '\r');		/* put char to tty */

#ifdef	REMOVE
    /* implement ^S/^Q */
    if ((ch = asygetchar (0) != 0xff) && ch == '\023')
	while (asygetchar (1) != '\021')
	    ;
#endif
}

/* Asynchronous get character */
kd_asygetc (dev, wait)
    int dev;
    int wait;
{
    if (kd_asyinitf [1] != dev) {
	kd_asyinit (dev);
	kd_asyinitf [1] = dev;
    }

    if (wait)
	while  (! (inb (dev + 5) & 0x1))	/* wait for RCA */
	    ;
    else if (! (inb (dev + 5) & 0x1))		/* non-wait, if no RCA */
	return 0xFF;				/* no character available */

    return inb (dev);				/* return character */
}

/* Asynchronous put character */
kd_asyputc (dev, ch)
    int dev;
    int ch;
{
    int timer;

    if (kd_asyinitf [0] != dev) {
	kd_asyinit (dev);
	kd_asyinitf [0] = dev;
    }

    timer = 20000;
    while (!(inb (dev + 5) & 0x20))	/* wait for TBE */
	if (--timer < 0)
	    break;
    if (timer >= 0)			/* if we got TBE	*/ 
	    outb (dev, ch);		/* .. output the char	*/
}

/* do minimal initialization on a tty port */
kd_asyinit (dev)
    int dev;
{
    outb (dev + 3, 0x83);	/* mode control reg - enable dlab */
    outb (dev + 0, 0x0C);	/* divisor latch reg lsb - 9600 baud */
    outb (dev + 1, 0x00);	/* divisor latch reg msb - 9600 baud */
    outb (dev + 3, 0x03);	/* mode contol reg - disable dlab */
    outb (dev + 1, 0x00);	/* interrupt enable reg - disable interrupts */
    outb (dev + 4, 0x03);	/* modem control reg - dtr/rts only */
    (void) inb (dev + 5);	/* clear line status reg */
    (void) inb (dev + 6);	/* clear modem status reg */
    (void) inb (dev + 1);	/* clear interrupt id reg */
    (void) inb (dev);		/* clear input char reg */
}
/* === */
