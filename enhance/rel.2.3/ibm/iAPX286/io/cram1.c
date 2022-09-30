
static char *uportid = "@(#)cram.c	Microport Rev Id ISC386  3/6/87";
/* @(#)cram.c	1.0 */
/*
 *	Cmos special file
 * Written by L. Weaver Fri Mar  6 14:58:22 PST 1987
 */

#include "sys/param.h"
#include "sys/types.h"
#include "sys/sysmacros.h"
#include "sys/dir.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/buf.h"
#include "sys/systm.h"
#include "sys/mmu.h"
#include "sys/seg.h"
#include "sys/io_op.h"
#include "sys/ioctl.h"
#include "sys/cram.h"

#define CMOSIZE 0x3F

cmosioctl( dev,cmd,arg,flag )
	int cmd,flag;
	char *arg;
	dev_t dev;
{

	if ((*arg  > CMOSIZE) || (*arg < 0 )) /* if address invalid */
	{
		u.u_error = ENXIO;
		return;
	}
	if ( cmd == CMOSREAD )
	{
		cramitup();
		outb( CMOS_ADDR, *arg );
		*(arg+1) = inb( CMOS_DATA );
		cramres();
		return;
	}

	if ( cmd == CMOSWRITE )
	{
		if (u.u_uid != 0)
		{
			u.u_error = EPERM ; /* only superuser can write */
			return;
		}
		cramitup();
		outb( CMOS_ADDR, *arg );
		outb( CMOS_DATA ,*(arg+1))  ;
		cramres();
		return;
	}
	u.u_error = ENODEV ; /* bad command */
	return;
}


cmosread( dev )
{
	int x;
	if (  u.u_offset  > CMOSIZE )
	{
		u.u_error = ENXIO;
		return;
	};
	x = cramitup();
	while (u.u_count)
	{
		if (  u.u_offset  > CMOSIZE )
			return;
		outb(CMOS_ADDR,	u.u_offset);
		passc(inb(CMOS_DATA));
	}
	cramres(x);

}

cmoswrite( dev )
{
	int x;

	if (  u.u_offset  > CMOSIZE )
	{
		u.u_error = ENXIO;
		return;
	}
	x = cramitup();
	while (u.u_count)
	{
		if (  u.u_offset  > CMOSIZE )
			return;
		outb(CMOS_ADDR,	u.u_offset);
		outb(CMOS_DATA, cpass() );
	}
	cramres(x);
}

cramres(x)
{
	outb( CMOS_ADDR, 0xb );
	outb( CMOS_DATA, x );		/* restore normal updates */
}

cramitup()
{
	int x;
	outb( CMOS_ADDR, 0xb );
	x = inb( CMOS_DATA );
	outb( CMOS_ADDR, 0xb );
	outb( CMOS_DATA, x | 0x80 );	/* inhibit time updates */
	return x;
}
