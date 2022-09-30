/* static char *uportid = "@(#)lomem.c	Microport Rev Id  1.3.8 12/10/86"; */
/* Copyright 1986 by Microport. All Rights Reserved.
 *
 * Lo Memory Patchable Constants & cache buffers
 *
 * Initial Coding:
 *		uport!doug	Wed Dec 10 16:54:53 PST 1986
 * Modification History:
 *		uport!doug	Tue Dec 16 15:44:17 EST 1986
 *				add cbuf1 for test with rel1.3.8A1 floppy
 *  M001:	uport!rex	Tue Dec 16 15:44:17 EST 1986
 *				Maximum remote login count and patchable
 *				memory location.
 *  M002:	uport!rex	Sun Feb  1 16:19:06 PST 1987
 *				sleep address for newproc() ( init check )
 * 
 */
#include	"sys/param.h"
#include	"sys/types.h"

#define	MAXREMLOG	32		/* M001: cryptic patchable remote   */
daddr_t	cprlmv = MAXREMLOG;		/*	 login maximum variable     */
char	initchk = 1;			/* M001: wakeup address for newproc */

daddr_t ulpatch = CDLIMIT;		/* 1.3.8 patchable ulpatch */
unsigned char cbuf0[15 * 512] = { 0, };	

