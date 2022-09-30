#ifndef lint
static char *uportid = "@(#)asyio.c	3.0 Microport Rev 2.3 10/1/87";
#endif

/*
 * asyio - support kernel I/O for async lines.
 *
 *	uport!mike Tue Sep 22 10:50:53 PDT 1987
 *		Created.
 */

#define	INPUT	0
#define	OUTPUT	1

short asy_initf [2] = { 0 };	/* asynch init flags		*/

/* Asynchronous get character */
asy_getc (dev, wait)
    int dev;
    int wait;
{
    if (asy_initf [OUTPUT] != dev) {
	asy_init (dev);
	asy_initf [OUTPUT] = dev;
    }

    if (wait)
	while  (! (inb (dev + 5) & 0x1))	/* wait for RCA */
	    ;
    else if (! (inb (dev + 5) & 0x1))		/* non-wait, if no RCA */
	return 0xFF;				/* no character available */

    return inb (dev);				/* return character */
}

/* Asynchronous put character */
asy_putc (dev, ch)
    int dev;
    int ch;
{
    int timer;

    if (asy_initf [INPUT] != dev) {
	asy_init (dev);
	asy_initf [INPUT] = dev;
    }

    timer = 20000;
    while (!(inb (dev + 5) & 0x20))	/* wait for TBE */
	if (--timer < 0)		/* avoid lockup */
	    break;
    if (timer >= 0)			/* if we got TBE	*/ 
	    outb (dev, ch);		/* .. output the char	*/
}

/* do minimal initialization on a tty port */
asy_init (dev)
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
