/*
 * Copyright (@) 1987 Microport Systems Inc. All rights reserved.
 * 
 * Library of useful routines for device drivers
 * 	Initial Coding:	uport!dwight	Sun Jun 21 22:50:03 PDT 1987
 * 
 * The routines in this file are for the public domain. This code can
 * be used as long as credit is given Microport.
 *
 * This file contains the following routines:
 * 	bscopy():	Copies one area of memory to another, using it's
 *			own selectors. 
 */
#ident "@(#)mplib.c	1.0"

#include "sys/types.h"

extern long mapin();

extern bcopy();

int bs0 = 48;				/* selector used only by bcopy */
int bs1 = 49;				/* selector used only by bcopy */

/* bscopy:  copies from one physical address to another. insures that the
 *		addresses involved are properly mapped in the mmu.
 *
 *		input: source physical address, destination physical address.
 *		output: nothing.
 */

bscopy(from, to, count)
unsigned long from, to;
unsigned count;
{
	bcopy((caddr_t) mapin(from, bs0), (caddr_t) mapin(to, bs1), count);
}
