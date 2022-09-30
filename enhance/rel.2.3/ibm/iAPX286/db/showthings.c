/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident  "@(#)showthings.c	1.7"

#ifdef DEBUGGER

/*
 * print various things
 */

#include "sys/param.h"
#include "sys/types.h"
#include "sys/immu.h"
#include "sys/region.h"
#include "sys/proc.h"
#include "sys/var.h"
#include "sys/ipc.h"
#include "sys/shm.h"
#include "sys/inode.h"
#include "sys/dir.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/map.h"
#include "sys/seg.h"
#include "sys/sysmacros.h"
#include "sys/reg.h"

#define L(X) ((long)(X))


db_pinode( ip )
	register inode_t *ip;
{
	long diff;

	diff = (unsigned long)ip - (unsigned long)inode;

	printf( "align in table: %lx\n", diff % sizeof(*ip) );
	printf( "flag: %lx\tcount: %lx\tdev: %lx\tnumber: %lx\n",
	    L(ip->i_flag), L(ip->i_count), L(ip->i_dev), L(ip->i_number) );

/* can't get to ip->i_mode anymore; this hides in fsdependent inode struct */
	printf( "ftype: %lx\tlinks: %lx\t {u,g}ui: %lx,%lx\tsize: %lx\n",
	    L(ip->i_ftype), L(ip->i_nlink), L(ip->i_uid),
	    L(ip->i_gid), L(ip->i_size) );
}

db_dump( count, from )
    long   count;            /* how many bytes to dump */
    ushort *from;            /* address to dump from */
{
    ushort *to;
    ushort *wp;              /* pointer to word to dump */
    ushort *start, *end;     /* range of dump */
    char row[17];
    int i;

    count /= 2;              /* convert to byte count */
    to = from + count - 1;

    end = (ushort *)(((unsigned long)to & 0xFFFFFFF0) + 0xE);
    start = (ushort *)((unsigned long)from & 0xFFFFFFF0);

    for ( i = 0; i < 17; i++ )
	row[i] = 0;
    i = 0;
    for ( wp = end; wp >= start; wp-- ) {
	if ( wp > to || wp < from ) {
	    printf( "...." ); stuffrow( row, i, 0 );
	} else {
	    printf( "%.4x", *wp ); stuffrow( row, i, *wp );
	}
	i += 2;
	if ( (unsigned long)wp & 0xF )
	    printf( " " );
	else {
	    printf( "    %.8lx %.16s\n", wp, row );
	    for ( i = 0; i < 17; i++ )
		row[i] = 0;
	    i = 0;
	}
	if ( wp == start )
	    break;      /* get around compiler bug */
    }
}

stuffrow( row, index, data )
    char *row;          /* array for row */
    int index;          /* offset in array */
    ushort data;        /* two chars to stuff in row */
{
    char c;

    c = (data >> 8 ) & 0xFF;
    if ( c < ' ' || c > 127 )
	c = '.';
    row[index++] = c;
    c = data & 0xFF;
    if ( c < ' ' || c > 127 )
	c = '.';
    row[index] = c;
}

#define G(x,i) (((unsigned long *)(x))[i])
#define MAXSTACK ((unsigned long)&u + KSTKSZ)
#define INSTACK(lower,value) ((lower) < (value) && (value) < MAXSTACK)

db_stacktrace( arg )
	long arg;
{
	unsigned char * rp;             /* some of current routines code */
	unsigned long cbp;              /* bp for current function */
	int     rs;                     /* register save set */

	cbp = (unsigned long)(&arg - 2);
	rp = (unsigned char *)db_stacktrace;

	rs = 1;
	while ( 1 ) {
		if ( INSTACK(cbp,G(cbp,0)) ) {
			/*
			 * normal interrupt frame
			 */
			nframe( cbp, rp );
			rp = (unsigned char *)G(cbp,1);
			cbp = G(cbp,0);
		} else {
			/*
			 * maybe interrupt stack frame
			 */
			if ( ! INSTACK(cbp,G(cbp,ESP)) ) {
				nframe( cbp, rp );
				break;
			}
			iframe( cbp, rp, rs++ );
			if ( rs > 19 )
				rs = 19;
			if ( ! INSTACK(cbp,G(cbp,EBP)) )
				break;
			rp = (unsigned char *)G(cbp,14);
			cbp = G(cbp,EBP);
		}
	}
	return;
}

nframe( cbp, rp )
	unsigned long cbp;
	unsigned char * rp;
{
	printf( "%s(", findsymname( rp, 0 ) );
	printf( "%lx %lx %lx)",G(cbp,2),G(cbp,3),G(cbp,4));
	printf( " ret: %lx, l: %lx %lx, bp: %lx\n",
		G(cbp,1), G(cbp,-1), G(cbp,-2), cbp );
}

iframe( cbp, rp, rs )
	unsigned long cbp;
	unsigned char * rp;
{
	printf( "Trp %lx err %lx at %lx calls %s, bp = %lx\n",
		G(cbp,12), G(cbp,13), G(cbp,14),
		findsymname( rp, 0 ), cbp );
	db_saveregs( cbp, rs );
	frameregs( cbp, rs );
}

frameregs( cbp, rs )
	unsigned long cbp;
{
	printf( "ax:%.8x cx:%.8x dx:%.8x bx:%.8x fl:%.8x ds:%.4x fs:%.4x\n",
	G(cbp,EAX),G(cbp,ECX),G(cbp,EDX),G(cbp,EBX),G(cbp,EFL),
	G(cbp,DS) & 0xFFFF, G(cbp,FS) & 0xFFFF );
	printf( "sp:%.8x bp:%.8x si:%.8x di:%.8x set:%.2d      es:%.4x gs:%.4x\n",
	G(cbp,ESP),G(cbp,EBP),G(cbp,ESI),G(cbp,EDI), rs,
	G(cbp,ES) & 0xFFFF, G(cbp,GS) & 0xFFFF );
}

struct regset {
	unsigned long regs[17];
} regset[20];

db_useregs( set )
{
	regset[0] = regset[set];
}

db_saveregs( from, set )
	struct regset * from;
{
	regset[set] = *from;
}

get_gs() { return regset[0].regs[GS]; }
get_fs() { return regset[0].regs[FS]; }
get_es() { return regset[0].regs[ES]; }
get_ds() { return regset[0].regs[DS]; }
get_di() { return regset[0].regs[EDI]; }
get_si() { return regset[0].regs[ESI]; }
get_bp() { return regset[0].regs[EBP]; }
get_sp() { return regset[0].regs[ESP]; }
get_bx() { return regset[0].regs[EBX]; }
get_dx() { return regset[0].regs[EDX]; }
get_cx() { return regset[0].regs[ECX]; }
get_ax() { return regset[0].regs[EAX]; }
get_tp() { return regset[0].regs[TRAPNO]; }
get_er() { return regset[0].regs[ERR]; }
get_ip() { return regset[0].regs[CS]; }
get_cs() { return regset[0].regs[EIP]; }
get_fl() { return regset[0].regs[EFL]; }

#endif /* DEBUGGER */
