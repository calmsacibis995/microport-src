static char *uportid = "@(#)lp.c	Microport Rev Id 1.3.8  1/19/87";

/* @(#)lp.c     1.3 */
/*
 *  Line printer driver
 *
 *  Modifications:
 *  M000    uport!larry	1986
 *		implemented transparrent mode with ioctl() calls
 *  M001    uport!mike	Nov 1986
 *		implemented transparrent mode with minor device
 *  M002    uport!rex	Sun Jan 18 12:54:28 PST 1987
 *		added lpstat() macro
 *		added lpinit() routine ( Master file change required )
 *		modified lpopen() to fail if printer not connected or not
 *		    on, otherwise enable interrupts
 *		modified lpprime() to sleep if status is not ready or busy,
 *		    so will resume when printer comes back on line
 *  M003    uport!dean	Tue Mar 10 16:39:58 PST 1987
 *		added MP386 ifdefs
 *  M004    uport!rex	Tue Apr 14 14:47:35 PST 1987
 *		Integrated LCC MERGE support code
 *  M005    uport!chuck	Thu Aug 27 13:46:50 PDT 1987
 *		Modified to ignore automatic FORMFEEDs if lp->line is 0.
 *		New default is 0 lines.
 */

#define BUGG(string) /* { if (lpdb) printf ("%s\n",string); } */
typedef unsigned char byte;
int lpdb= 0;

#include "sys/param.h"
#include "sys/types.h"
#include "sys/dir.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/tty.h"
#include "sys/lprio.h"
#ifdef ATMERGE
#include "sys/realmode.h"
#endif

#define OUTB(port,stuff) { outb(port,stuff); }
			/* printf("OUTB( %x , %x )\n",port,stuff); */
#define INB(port)	inb(port) 
/*
short port;
{	byte b;
	b=inb(port);
	printf("%x = INB( %x )\n",b, port);
	return (b);
}
*/

#define lpstat(unit) (inb(statport[unit]))

/* defines for interrupt levels */
#ifdef MP386				/* M003 */
#define LP0VEC	0x7			/* M003 */
#define LP1VEC 0x5			/* M003 */
#else	/* ! MP386 */		/* M003 */
#define LP0VEC 0x27
#define LP1VEC 0x25
#endif	/* ! MP386 */		/* M003 */

/* defines for printer control port */
#define	IRQEN	0x10	/* enable int on ack down */
#define	SELECT	0x08	/* select printer */
#define	INIT	0x04	/* init, pulse down to start printer */
#define	AUTOFD	0x02	/* cause printer to line feed when a line is printed */
			/* does this mean auto-wrap? */
#define	STROBE	0x01	/* clock data to printer (load data port, then clock) */

/* defines for printer status port */
#define	LPBUSY	0x80	/* printer cannot accept data */
#define	ACK	0x40	/* printer is ready for another character */
#define	PE	0x20	/* end of paper */
#define	ISSLCT	0x10	/* printer is selected (means its alive) */
#define	ERROR	0x08	/* printer error */
#define NORMAL	ERROR	/* used for bit "on" test, 0 means error */
#define LPFAIL	0x28	/* lp not in working condition */
#define LPPASS	0x90	/* lp in working condition */
#define NOPRINT	0x30	/* parallel connector not attached */

extern  lp_cnt;

#define LPPRI   (PZERO+8)
#define LPLOWAT 40
#define LPHIWAT 100
#define LPMAX   3

struct lp {
        struct  clist l_outq;
        char    flag, ind;
        int     ccc, mcc, mlc;
        int     line, col, mode;		/* M000 */
} lp_dt[LPMAX];

short	dataport[LPMAX] = {0x3bc,0x378,0x278};
short	ctrlport[LPMAX] = {0x3be,0x37A,0x27a};
short	statport[LPMAX] = {0x3bd,0x379,0x279};
short	lpvector[LPMAX] = { LP0VEC,LP0VEC,LP1VEC };
static	unsigned char	print_control;

/* defines for flag byte */
#define TOPEND	04
#define OPEN    010
#define CAP     020
#define NOCR    040
#define ASLP    0100
#define	RAW	0200		/* M001: also used in lp->mode */
#define	LPRDY	01		/* M002: only used in lp->mode */

#define FORM    014

#define	UNIT(dev)	(dev&07)
#define	PARMS(dev)	(dev&(CAP|NOCR|RAW))	/* M001 */
#define LPIN (LPR | 03)
#define LPOUT (LPR | 04)

/*
 *  M002: lpinit() to select and initialize each printer
 */
lpinit()
{
        register unit;
        register struct lp *lp;
	BUGG("INIT");
        for (unit=0; unit < LPMAX; ++unit) {
		lp = &lp_dt[unit];
		lp->l_outq.c_cc = 0 ;
		lp->flag = 0;
                lp->ind = 0;
                lp->col = 132;
                lp->line = 0;	/* M005: should be 66 or 0 for no FORMFEEDS */
		lp->mode = 0;	/* M000 */	/* non-xparent */
		print_control = (SELECT|INIT);
		OUTB(ctrlport[unit],print_control);
	}
}

lpopen(dev, mode)
{
        register unit;
        register struct lp *lp;
	BUGG("LPOPEN");
        unit = UNIT(dev);
        if (unit >= lp_cnt || unit >= LPMAX || (lp = &lp_dt[unit])->flag) {
        	u.u_error = EIO;
                return;
        }
					/* M002	   is printer there? */
	if ((lpstat(unit) & NOPRINT) == NOPRINT) {
		u.u_error = EIO;	/*	the power is off or */
		return;			/*	cable not connected */
	}
#ifdef ATMERGE
	/* try to get control of the device */
	if (devclaim(DEVN_PRINTER)) {
		u.u_error = EBUSY;	/* DOS has the device */
		return;
	}
#endif /* ATMERGE */
        lp->l_outq.c_cc = 0 ;
        lp->flag = PARMS(dev)|OPEN;
	print_control |= IRQEN;		/* enable interrupts */
	OUTB(ctrlport[unit],print_control);
	lp->mode |= LPRDY;
}

lpclose(dev)
{
	register struct lp *lp;
        register unit;
BUGG("LPCLOSE");
        unit = dev&07;
	if (lpnrdy(unit)) return;
        lp = &lp_dt[unit];
        while(lp->l_outq.c_cc > 0 )
		lpprime(unit);
        delay(60);
 	print_control &= ~IRQEN;
 	OUTB(ctrlport[unit],print_control);
        lp_dt[unit].flag = 0;
	lp->mode &= ~LPRDY;		/* M002 */
#ifdef ATMERGE
	devrelse(DEVN_PRINTER);
#endif
}

lpwrite(dev)
{
        register unit;
        register c;
        register struct lp *lp;
        int	x;
 BUGG("LPWRITE");
	unit = dev&07;
				/* M002: lpnrdy() check taken out */
        lp = &lp_dt[unit];
        while (u.u_count) {
                c = fubyte(u.u_base++);
                if (c < 0) {
                        u.u_error = EFAULT;
                        break;
                }
                u.u_count--;
                lpoutput(unit, c);
                while(lp->l_outq.c_cc > 0 )
                        lpprime(unit);
        }
}

lpoutput(dev, c)
register dev, c;
{
        register struct lp *lp;
BUGG("LPOUTPUT");
        lp = &lp_dt[dev];
	if((lp->flag & RAW) || (lp->mode & RAW))	/* M002 */
	{
        	putc(c, &lp->l_outq);
		return;
	}
        if(lp->flag&CAP) {
                if(c>='a' && c<='z')
                        c += 'A'-'a'; else
                switch(c) {
                case '{':
                        c = '(';
                        goto esc;
                case '}':
                        c = ')';
                        goto esc;
                case '`':
                        c = '\'';
                        goto esc;
                case '|':
                        c = '!';
                        goto esc;
                case '~':
                        c = '^';
                esc:
                        lpoutput(dev, c);
                        lp->ccc--;
                        c = '-';
                }
        }
        switch(c) {
        case '\t':
                lp->ccc = ((lp->ccc+8-lp->ind) & ~7) + lp->ind;
                return;
        case '\n':
                lp->mlc++;
                putc( '\r', &lp->l_outq );
		if (lpdb) printf("\n"); 
                if(lp->line && lp->mlc >= lp->line )	/* M005 */
                        c = FORM;
        case FORM:
                lp->mcc = 0;
                if (lp->mlc) {
                        putc(c, &lp->l_outq);
                        if(c == FORM)
                                lp->mlc = 0;
                }
        case '\r':
                lp->ccc = lp->ind;
                return;
        case 010:
                if(lp->ccc > lp->ind)
                        lp->ccc--;
                return;
        case ' ':
                lp->ccc++;
                return;
        default:
                if(lp->ccc < lp->mcc) {
                        if (lp->flag&NOCR) {
                                lp->ccc++;
                                return;
                        }
                        putc('\r', &lp->l_outq);
                        lp->mcc = 0;
                }
                if(lp->ccc < lp->col) {
                        while(lp->ccc > lp->mcc) {
                                putc(' ', &lp->l_outq);
                                lp->mcc++;
                        }
                        putc(c, &lp->l_outq);
                        lp->mcc++;
                }
                lp->ccc++;
        }
}

lprto(lp)
struct lp *lp;
{
	lp->flag &= ~TOPEND;
	if (lpdb) printf("T");
	if (lp->flag & ASLP)
	{
		lp->flag &= ~ASLP;
		if (lpdb) printf("W");
		wakeup((caddr_t)lp);
	}
}

lpintr(vec)
{
        struct lp *lp;
        int x, c, unit;
#ifndef	PICFIX1
	x = spl5();
#ifndef MP386					/* M003 */
	eoi(vec);
#endif /* ! MP386 */				/* M003 */

#endif /* ! PICFIX1 */ 

	if (lpdb) printf("I");
	for (unit = 0 ; unit < LPMAX ; unit++) 
	        if ( vec == lpvector[unit] )
		{
        		lp = &lp_dt[unit];
			if (lp->flag & ASLP)
			{
				lp->flag &= ~ASLP;
				if (lpdb) printf("W");
		 		wakeup(lp);
			}
		}
#ifndef	PICFIX1
	splx(x);
#endif /* ! PICFIX1 */ 
}

lpprime( dev )
{
        register struct lp *lp;
        register c;
        int	x,y;
	unsigned char   stat,ctmp;
	int	unit;
	unit = UNIT(dev);
        lp = &lp_dt[unit];
        while((( c = getc( &lp->l_outq )) >= 0) ) 
	{
		if (! (lpstat(unit) & NORMAL) )		/* M002 */
		{
			lp->flag |= ASLP;
			sleep(lp,LPPRI);
		}					/* M002 */
                OUTB( dataport[unit], c );
		for ( x=50 ; x ;x--);	/* slow down a bit */
		x = spl5();
		while ( !((stat = lpstat(unit)) & LPBUSY) )
		{
			if (!(lp->flag & TOPEND)) 
			{
				lp->flag |= TOPEND;
				timeout(lprto,(caddr_t)lp,5);
			}
			if (lpdb) printf("S");
			lp->flag |= ASLP;
			sleep(lp,LPPRI);
		}
		lp->flag &= ~ASLP;
		if (lpnrdy(unit)) return;	/* M001 */
		splx(x);
		OUTB( ctrlport[unit], (print_control | STROBE) );
		for ( x=50 ; x ;x--);	/* slow down a bit */
                OUTB( ctrlport[unit], print_control );
	}
	stat = lpstat(unit);
}

/*
 * M001: Added for ready status check
 */
lpnrdy(unit)
register unit;
{
	if (lpstat(unit) & NORMAL) return 0;
	print_control &= ~IRQEN;	/* kill interrupt */
	if (lpdb) printf("S");
	OUTB( ctrlport[unit], print_control );
	lp_dt[unit].flag = 0;		/* close device */
	lp_dt[unit].mode &= ~LPRDY;	/* M002 */
	return 1;
}


lpioctl(dev, cmd, arg, mode)
int *arg;
{
        register struct lp *lp;
        struct lprio lpr;
BUGG("LPIOCTL");
        lp = &lp_dt[dev];
        switch(cmd) {
        case LPRGET:
                lpr.ind = lp->ind;
                lpr.col = lp->col;
                lpr.line = lp->line;
                lpr.mode = (lp->mode & RAW);	/* M000 & M002 */
                if (copyout(&lpr, arg, sizeof lpr))
                        u.u_error = EFAULT;
                break;
        case LPRSET:
                if (copyin(arg, &lpr, sizeof lpr)) {
                        u.u_error = EFAULT;
                        break;
                }
                lp->ind = lpr.ind;
                lp->col = lpr.col;
                lp->line = lpr.line;
						/* M000 & M002 */
		lp->mode = lpr.mode ? (lp->mode |= RAW) : (lp->mode &= ~RAW);
                break;
	case LPIN:
                if (copyin(arg, &lpr, sizeof lpr)) {
                        u.u_error = EFAULT;
                        break;
                }
		lpr.col = inb(lpr.ind);
                if (copyout(&lpr, arg, sizeof lpr))
                        u.u_error = EFAULT;
                break;
	case LPOUT:
                if (copyin(arg, &lpr, sizeof lpr)) {
                        u.u_error = EFAULT;
                        break;
                }
		outb( lpr.ind, lpr.col );
		if (lpdb)
			printf("3BD: %x \n",inb(0x3bd));
                break;

        default:
                u.u_error = EINVAL;
        }
}

#ifdef ATMERGE
lpreset(devn)
{
}
#endif /* ATMERGE */
