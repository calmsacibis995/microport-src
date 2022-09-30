
#ifndef HZ
#define HZ 60
#endif

/* Definitions for 8254 Programmable Interrupt Timer ports on AT 386 */
#define	PITCTR0_PORT	0x40		/* counter 0 port */	
#define	PITCTR1_PORT	0x41		/* counter 1 port */	
#define	PITCTR2_PORT	0x42		/* counter 2 port */	
#define	PITCTL_PORT	0x43		/* PIT control port */
#define	PITAUX_PORT	0x61		/* PIT auxiliary port */

/* Definitions for 8254 commands */

/* Following are used for Timer 0 */
#define PIT_C0          0x00            /* select counter 0 */
#define	PIT_LOADMODE	0x30		/* load least significant byte followed
					 * by most significant byte */
#define PIT_NDIVMODE	0x04		/*divide by N counter */
#define	PIT_SQUAREMODE	0x06		/* square-wave mode */

/* Used for Timer 1. Used for delay calculations in countdown mode */
#define PIT_C1          0x40            /* select counter 1 */
#define	PIT_READMODE	0x30		/* read or load least significant byte
					 * followed by most significant byte */
#define	PIT_RATEMODE	0x06		/* square-wave mode for USART */
#define PIT_COUNTDOWN PIT_READMODE|PIT_NDIVMODE

#define	CLKNUM	(1193167/HZ)		/* clock speed for timer */
/* bits used in auxiliary control port for timer 2 */
#define	PITAUX_GATE2	0x01		/* aux port, PIT gate 2 input */
#define	PITAUX_OUT2	0x02		/* aux port, PIT clock out 2 enable */
int pitctl_port  = PITCTL_PORT;		/* For 386/20 Board */
int pitctr0_port = PITCTR0_PORT;	/* For 386/20 Board */
int pitctr1_port = PITCTR1_PORT;	/* For 386/20 Board */
int pitctr2_port = PITCTR2_PORT;	/* For 386/20 Board */
/* We want PIT 0 in square wave mode */

int pit0_mode = PIT_C0|PIT_SQUAREMODE|PIT_READMODE ;

unsigned int delaycount;		/* loop count for delay loop */
unsigned int clknumb = CLKNUM;		/* interrupt interval for timer 0 */

#define COUNT	0x2000

pit_init()
{
	unsigned int flags;
	unsigned char byte;
	unsigned int leftover;
	int i;
	int j;

	/* disable interrupts */
	asm("   pushf");
	asm("   cli");

	/* Put counter in count down mode */

	outb(pitctl_port, PIT_COUNTDOWN);
	/* output a count of -1 to counter 0 */
	outb(pitctr0_port, 0xff);
	outb(pitctr0_port, 0xff);
	delaycount = COUNT;
	tenmicrosec();
	/* Read the value left in the counter */
	byte = inb(pitctr0_port);	/* least siginifcant */
	leftover = inb(pitctr0_port);	/* most significant */
	leftover = (leftover<<8) + byte ;
	/* Formula for delaycount is :
	 *  (loopcount * timer clock speed)/ (counter ticks * 1000)
	 * 1000 is for figuring out milliseconds 
	 */
	delaycount = (((COUNT * CLKNUM)/1000) * HZ) / (0xffff-leftover);
	/* restore interrupts */
	asm("   popf");
}
