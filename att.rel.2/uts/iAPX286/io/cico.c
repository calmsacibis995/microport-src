/*      Copyright (c) 1985 AT&T */
/*        All Rights Reserved   */

/*      THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T     */
/*      The copyright notice above does not evidence any        */
/*      actual or intended publication of such source code.     */

/* @(#)cico.c	1.4 */
#include "sys/mpsc.h"
#define	CHAN_A	0
#define	CHAN_B	1
/*
 * polled 8274 ci/co with fix for transmitter ready
 */
extern struct	mpsc C8274[];

char
getchar(){
	
	while((inb(C8274[CHAN_B].m_ctrl)&M_CHAR_AV)==0)
		;
	return(inb(C8274[CHAN_B].m_data) & 0xFF);
}

putchar(ch)
char	ch;
{
	if ( ch == '\n' )
		putchar( '\r' );

	while((inb(C8274[CHAN_B].m_ctrl)&M_TX_EMPTY)==0)
		;
	outb(C8274[CHAN_B].m_data,ch);
	outb(C8274[CHAN_B].m_ctrl,M_RS_TX_INT);
	outb(C8274[CHAN_A].m_ctrl,M_EOI);
}
