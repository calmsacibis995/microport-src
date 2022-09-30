/*	Copyright (c) 1985 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#/*   @(#)errpt.c	1.3 - 85/08/09 */
/* Format and interpret the error log file */
#include <stdio.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/utsname.h>
#include <sys/map.h>
#include <sys/wn.h>
#include <sys/elog.h>
#include <sys/err.h>
#include <sys/erec.h>
#include <time.h>

#define   PNEED   24
#define dysize(x)	(((x)%4)? 365: 366)
#define writout()	((mode==PRINT) && (page<=limit))
#define major(x)	(int)((unsigned)(x>>8)&0377)
#define minor(x)	((int)x&0377)
#define araysz(x)	((sizeof(x)/sizeof(x[0]))-2)
#define readrec(x)	(fread((char*)&recrd,\
			(e_hdr.e_len - sizeof(struct errhdr)), 1, x) )
#define recrd		ercd.ebb.block
#define erp		ercd.parity
#define erm		ercd.memory

#define FORM "0x%.4x"
#define FORM2 "%16lx\n"
#define DRIVE_REG 11
#define DRIVE_TYPE	ercd.ebb.reginf[DRIVE_REG] & 0777
char Nl [1];
#define NUL4	Nl,Nl,Nl,Nl
#define NUL8	Nl,Nl,Nl,Nl,Nl,Nl,Nl,Nl
#define NULS	Nl,Nl,Nl,Nl,Nl,Nl,Nl,Nl,Nl,Nl,Nl,Nl,Nl,Nl,Nl,Nl

#define INAUG "0301080077"	/* start of error logging [mmddhhmmyy] */
#define WDLEN 16
#define MINREC 8
#define MAXREC 74
#define DEVADR ((physadr)(0160000))
#define MAXLEN 66
#define MAXSTR 40
#define NMAJOR 1
#define NMINOR 8
#define PGLEN 60
#define MBAREG 5
#define INT  15

#define MEM  14
#define YES 1
#define NO 0
#define PRINT 1
#define NOPRINT 0
#define USAGE1 "errpt [-a] [-d devlist] [-s date]"
#define USAGE2 "\n [-e date] [-p n] [-f] [files]"

/* NMAJOR devices of NMINOR possible logical units */
struct sums {
	long	soft;
	long	hard;
	long	totalio;
	long	misc;
	long	missing;
	char	*drvname;
} sums[NMAJOR][NMINOR];
struct tb_sums {
	long	totalio;
	long	misc;
	long	missing;
};

/* structure for maintaining totals across multiple error files: */
struct tb_sums tot_sums[NMAJOR][NMINOR];
struct tb_sums base_sums[NMAJOR][NMINOR];

union ercd {
	struct  estart start;
	struct	eend end;
	struct	etimchg timchg;
	struct	econfchg confchg;
	struct	estray stray;
	struct	eparity parity;
	struct	ememory memory;
	struct  eb {
		struct	eblock block;
		unsigned char reginf [64];
	} ebb;
} ercd;

struct errhdr e_hdr;

int dmsize[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
int maj;
int min;
int page=1;
int print;
int mode = NOPRINT;
int line;
int n = 0;
int aflg;
int dflg;
int fflg;
int Unix = 1;
short cputype;
int parsum;
int straysum;
int limit = 10000;		/* page limit */
int optdev = 0;			/* bit map for device-specific reports */
long readerr = 0;		/* input record error count */
FILE	*file;
time_t	atime;
time_t	xtime = 0L;
time_t	etime = 017777777777L;
time_t	fftime = 017777777777L;
time_t	ltime = 0L;
char interp [MAXSTR];
char choice [MAXSTR];
char *strcpy();
char *ctime();
long tloc;
long time();
struct regs {
	char *regname;
	char *bitcode [WDLEN];
};

struct regs *regs_ptr;
char htime[20];
char *header = "SYSTEM ERROR REPORT";
char *hd1 = "System Error Report - Selected Items";
char *hd2 = "Summary Error Report";

char *dev[] = {
	"WN0",
	0
};

int wnblk();
int wnmap();

/* Device specification functions */
/* Must remain in same order as true major device numbers */
int (*func[]) () = {
	wnblk,
	0
};

int (*maps[]) () = {
	wnmap,
	0
};

char *wmodbit[] = {
	"Interrupt suppressed",
	"Automatic retries inhibited",
	"Allow deleted data",
	Nl,
	"24 bit addressing",
};

char *whardbit[] = {
	Nl,
	Nl,
	Nl,
	"Ram error",
	"Rom error",
	"Seek in progress",
	"Illegal format type",
	"End of media",
	"Illegal sector size",
	"Diagnostic fault",
	"No index",
	"Invalid command",
	"Sector not found",
	"Invalid address",
	"Selected unit not ready",
	"Write protection fault",
};

char *wsoftbit[] = {
	Nl,
	Nl,
	Nl,
	"Data field ecc error",
	"Id field ecc error",
	"Drive fault",
	"Cylinder address miscompare",
	"Seek error",
	NUL8,
};

/* Array order directed by MMR3 */
char *msg[] = {
	"User D Space Enabled",
	"Supervisor D Space Enabled",
	"Kernel D Space Enabled",
	Nl,
	"22 bit mapping Enabled",
	"UNIBUS MAP relocation Enabled",
	0
};

char *lines [] = {
/* 0*/	"\n",
/* 1*/	"%s\tError Logged On   %s\n",
/* 2*/	"\tPhysical Device\t\t\t%u\n",
/* 3*/	"\tLogical Device\t\t\t%d\n",
/* 4*/	"\tDevice Address\t\t\t",
/* 5*/	"\tRetry Count\t\t\t%u\n",
/* 6*/	"\tError Diagnosis\t\t\t%s\n",
/* 7*/	"\tSimultaneous Bus Activity\t",
/* 8*/	"\tRegisters at Error time\n",
/* 9*/	"\t\t%s\t",
/*10*/	"\tBuffer Start Address\t\t",
/*11*/	"\tTransfer Size in Bytes\t\t%16u\n",
/*12*/	"\tType of Transfer\t\t\t%8s\n",
/*13*/	"\tBlock No. in Logical File System\t%8ld\n",
/*14*/	"\tI/O Type\t\t\t\t%8s\n",
/*15*/	"\tCylinder\t\t\t\t%8u\n",
/*16*/	"\tTrack\t\t\t\t\t%8u\n",
/*17*/	"\tSector\t\t\t\t\t%8u\n",
/*18*/	"\tStatistics on Device to date:\n",
/*19*/	"\t\tR/W Operations\t\t%16ld\n",
/*20*/	"\t\tOther Operations\t%16ld\n",
/*21*/	"\t\tUnrecorded Errors\t%16u\n",
/*9b*/	"%o      ",
/*23*/	"\tSector Requested\t\t%u\n",
/*24*/  "\tUnibus Map Utilization?\t\t\t%3.3s\n",
	0
};

char *xlines [] = {
/* 0*/  "\n\nDEVICE CONFIGURATION CHANGE   - %s\n",
/* 1*/	"\tDEVICE: %s - %s\n",
/* 2*/	"\n\nSTRAY INTERRUPT on %s\n",
/* 3*/  "\tFor Controller at - ",
/* 4*/	"\tAt Location\t",
/* 5*/	"\n\nMEMORY ECC on %s\n",
/* 6*/	"\tError Address (%s)\t%.5X\n",
/* 7*/	"\tError Syndrome\t\t\t%.2X\n",
/* 8*/	0,
/* 9*/	"\n\nTIME CHANGE ***** FROM %s",
/*10*/  "\t\t   TO  %s \n\n\n\n",
/*11*/  "\nERROR LOGGING SYSTEM SHUTDOWN - %s\n\n\n",
/*12*/	"\nERROR LOGGING SYSTEM STARTED - %s \n",
/*13*/	"\n\n\tSystem Profile:\n\n",
/*14*/	"\t     iAPX%d  Processor\n",
/*15*/	"\t     System Memory Size - %ld Bytes\n",
/*16*/	Nl,
/*17*/	"\t     UNIX/%s  Operating System (%s)\n",
/*18*/  "\t     %s \n",
	0
};

char *sumlines [] = {
/* 0*/	"\n\n",
/* 1*/	"%s\tUnit %d\n",
/* 2*/	"\tHard Errors\t\t- %10ld\n",
/* 3*/	"\tSoft Errors\t\t- %10ld\n",
/* 4*/	"\tTotal I/O Operations    - %10ld\n",
/* 5*/	"\tTotal Misc. Operations  - %10ld\n",
/* 6*/	"\tErrors Missed\t\t- %10ld\n",
/* 7*/	"\tTotal Read Errors\t\t-   %ld\n",
/* 8*/	"\tTotal Memory Parity Errors\t-   %d\n",
/* 9*/	"\tTotal Stray Interrupts\t\t-   %d\n",
/*10*/	"\tDate of Earliest Entry: %s",
/*11*/	"\tDate of Latest   Entry: %s",
/*12*/  "\tError Types: %s\n",
/*13*/  "\tLimitations: ",
/*14*/  "\t\t",
0
};

char *f215line[] = {
/* 0*/	"INITIALIZE",
/* 1*/	"TRANSFER STATUS",
/* 2*/	"FORMAT",
/* 3*/	"READ SECTOR ID",
/* 4*/	"READ DATA",
/* 5*/	"READ TO BUFFER AND VERIFY",
/* 6*/	"WRITE DATA",
/* 7*/	"WRITE BUFFER DATA",
/* 8*/	"INITIATE TRACK SEEK",
/* 9*/	"INVALID",
/* A*/	"INVALID",
/* B*/	"INVALID",
/* C*/	"iSBX EXECUTE",
/* D*/	"iSBX TRANSFER",
/* E*/	"BUFFER I/O",
/* F*/	"DIAGNOSTIC",
0
};

/* Correspondence of input requests to major device defines */
struct tab {
	char *devname ;
	int devnum;
};
struct tab dtab[] = {
	"wn", WN0,	0,0
};

main (argc,argv)
char *argv[];
int argc;
{
	register i,j;
	print = NO;
	while (--argc>0 && **++argv =='-') {
		switch (*++*argv) {
		case 's':	/* starting at a specified time */
			header = hd1;
			if((--argc <=0) || (**++argv == '-'))
			error("Date required for -s option",(char *)NULL);
			if(gtime(&xtime,*argv))
				error("Invalid Start time",*argv);
			break;
		case 'e':	/* ending at a specified time */
			header = hd1;
			if((--argc<=0) || (**++argv =='-'))
				error("Date required for -e option\n",(char *)NULL);
			if(gtime(&etime,*argv))
				error("Invalid End time.",(char *)NULL);
			break;
		case 'a':	/* print all devices*/
			aflg++;
			mode = PRINT;
			break;
		case 'p':	/* limit total no. of pages */
			if((--argc<=0) || (**++argv == '-'))
				error("Page limit not supplied.\n",(char *)NULL);
			limit = atoi(*argv);
			break;
		case 'f':	/* fatal errors */
			header = hd1;
			fflg++;
			break;

		default:
			if(j=encode(*argv)) {
				optdev = (optdev |= j);
				dflg++;
				header = hd1;
				mode = PRINT;
				if(strlen(choice)) concat(",",choice);
				concat(*argv,choice);
				}
			else
			(void) fprintf(stderr,"%s?\n",argv);
		}
	}
	for(i = 0;i < NMAJOR;i++) {
		for(j = 0; j < NMINOR; j++) {
			sums[i][j].hard = 0;
			sums[i][j].soft = 0;
			sums[i][j].totalio = 0;
			sums[i][j].misc = 0;
			sums[i][j].missing = 0;
			tot_sums[i][j].totalio = 0;
			tot_sums[i][j].misc = 0;
			tot_sums[i][j].missing = 0;
			base_sums[i][j].totalio = 0;
			base_sums[i][j].misc = 0;
			base_sums[i][j].missing = 0;
		}
	}

	parsum=0;
	straysum=0;
	if(gtime(&atime,INAUG)) error("Invalid INAUG time",INAUG);
	if (argc ==0)
		report("/usr/adm/errfile");
	else while(argc--) 
		report(*argv++);
	printsum();
	putft();
	exit(0);
}

/* Associate typed name with a specific bit in "optdev" */
encode(p)
char  *p;
{
	register struct tab *q;
	int lower();

	lower (p); /* convert device name to lower case */
	for(q=dtab;q->devname;q++) {
		if (!strcmp(q->devname,p)) {
			return(1<<(q->devnum));
		}
	}
	return(0);
}

report(fp)
char *fp;
{
	register int i, j;

	if ((file = fopen(fp, "r")) == NULL)
		error("cannot open", fp);
	inithdng();
	if (writout())
		puthead(header);
	putdata();
	if (writout())
		putft();
	for(i = 0; i < NMAJOR; i++)
		for(j = 0; j < NMINOR; j++)
			if(sums[i][j].totalio)
				adjustsum(i, j);
}
putdata()
{
	while(fread((char*)&e_hdr.e_type, sizeof(struct errhdr), 1, file)) {
newtry:
		switch(e_hdr.e_type) {
		
		case E_GOTS:
			setime();
			up();
			break;

		case E_GORT:
			setime();
			up();
			break;

		case E_STOP:
			setime();
			down();
			break;

		case E_TCHG:
			setime();
			timecg();
			break;

		case E_BLK:
			setime();
			blk();
			break;
		
		case E_STRAY:
			setime();
			stray();
			break;

		case E_PRTY:
			setime();
			party();
			break;

		case E_CCHG:
			cconfig();
			setime();
			break;
		default:
			fprintf(stderr, "%d\n", e_hdr.e_len);
			fprintf(stderr, "%d\n", e_hdr.e_type);
			readerr++;
			if (recov())
				goto newtry;
			fprintf (stderr, "Unrecovered read error.\n");
		}
	}

}
/* System Startup Record */

#define est	ercd.start
up()
{
register int i;

	if (!readrec(file)) {
		fprintf(stderr, "at up = %o\n", est.e_cpu);
		fprintf(stderr, "%o\n", est.e_mmr3);
		fprintf(stderr, "%ld\n", est.e_name.release);
		fprintf(stderr,
			"%s %s\n", est.e_name.release, est.e_name.sysname);
		readerr++;
		return;
	}
	cputype = est.e_cpu;
	if (writout()) {
		need(13+araysz(msg));
		printf(xlines[12], ctime(&e_hdr.e_time));
		printf(xlines[13]);
		printf(xlines[17], est.e_name.release, est.e_name.sysname);
		printf(xlines[14], est.e_cpu);
		if (est.e_syssize)
			printf(xlines[15], est.e_syssize);
		else
			line--;
		for(i = 0; i <= araysz(msg) + 1; i++)
			if (est.e_mmr3 & (1 << i))
				printf(xlines[18], msg[i]);
			else
				line--;
		printf(lines[0]);
	}
}
/* System Shutdown Record */

down()
{
	if (writout()) {
		need(5);
		printf(xlines[11], ctime(&e_hdr.e_time));
	}
}
/* Time Change Record */

timecg()
{
	if (!readrec(file)) {
		readerr++;
		return;
	}
	if (writout()) {
		need(8);
		printf(xlines[9], ctime(&e_hdr.e_time));
		printf(xlines[10], ctime(&ercd.timchg.e_ntime));
	}
}
/* Handle a MERT configuration change */
cconfig() 
{

	if (!readrec(file)) {
		readerr++;
		return;
	}
	if (writout()) {
		need(7);
		printf(xlines[0], ctime(&e_hdr.e_time));
		printf(lines[0]);
		printf(xlines[1], dev[ercd.confchg.e_trudev],
			ercd.confchg.e_cflag ? "Attached" : "Detached");
		printf(lines[0]);
	}
}

/* Stray Interrupt Record */

#define estr	ercd.stray
stray()
{
	if (!readrec(file)) {
		readerr++;
		return;
	}
	if (!wanted())
		return;
	if (print == YES) {
		need(6);
		if (page <= limit) {
			printf(xlines[2], ctime(&e_hdr.e_time));
			if (estr.e_saddr < DEVADR)
				printf(xlines[4]);
			else
				printf(xlines[3]);
			printf(FORM, estr.e_saddr);
			printf(lines[0]);
			printf(lines[7]);
			if (estr.e_sbacty == 0)
				printf("None\n");
			else
				afix(araysz(dev)+1,
					(unsigned) estr.e_sbacty, dev);
		}
	}
	straysum++;
}

/* Memory Parity Record */
party()
{
	register struct vaxreg *q;
	char *ctime();

	if (!readrec(file)) {
		readerr++;
		return;
	}
	if (!wanted())
		return;
	if (print == YES) {
		need(9);
		if (page > limit)
			return;
		printf(xlines[5], ctime(&e_hdr.e_time));
		printf(xlines[6],(((long)(erp.e_parreg[1]&077))<<WDLEN) +
			((long)((unsigned) erp.e_parreg[0])),
			(erp.e_parreg[1]>>14)&03);
		printf(lines[0]);
		printf(lines[8]);
		printf(xlines[7], erp.e_parreg[2]);
		printf(lines[0]);
		printf(xlines[8], erp.e_parreg[3]);
	}
	parsum++;
}

/* Device Error Record */

blk()
{
	register union ercd *z;
	register short i;
	register struct sums *p;
	register struct eblock *r;
	int *mbar;
	struct vaxreg *q;
	int ldev;

	if (!readrec(file)) {
		readerr++;
		return;
	}
	z = &ercd;
	maj = major(recrd.e_dev);
	ldev = minor(recrd.e_dev);
	if ((maj > araysz(func)) | maj < 0)
		return;
	if (!wanted())
		return;

	min = (*maps[ maj ])(recrd.e_dev);

	/* Increment summary totals */
	
	p = &sums[maj][min];
	r = &recrd;
	if (r->e_bflags & E_ERROR)
		p->hard++;
	else
		p->soft++;
	if (r->e_stats.io_ops < p->totalio)
		adjustsum(maj, min);

	if (!(p->drvname))
		p->drvname = dev[maj];
	p->totalio = r->e_stats.io_ops;
	p->misc = r->e_stats.io_misc;
	p->missing = r->e_stats.io_unlog;

	if (print == NO)
		return;

	need( 19 );
	if (page > limit)
		return;
	printf(lines[0]);
	printf(lines[1], p->drvname, ctime(&e_hdr.e_time));
	printf(lines[3], ldev, ldev);
	printf(lines[5], r->e_rtry);
	printf(lines[6], r->e_bflags & E_ERROR ?
		"Unrecovered" : "Recovered");
	printf(lines[7]);
	if (r->e_bacty == 0)
		printf("None\n");
	else
		afix(WDLEN, (unsigned) r->e_bacty, dev);
	printf(lines[0]);
	printf(lines[10]);
	printf(FORM2, r->e_memadd);
	printf(lines[11], r->e_bytes);
	i = r->e_bflags;
	printf(lines[12],
		(i & E_NOIO) ? "No-op" : ((i & E_READ) ? "Read" : "Write"));
	printf(lines[13], r->e_bnum);
	if (Unix)
		printf(lines[14], i & E_PHYS ? "Physical" : "Buffered");
	else
		line--;

	printf(lines[0]);
	printf(lines[18]);
	printf(lines[19], tot_sums[maj][min].totalio+r->e_stats.io_ops);
	printf(lines[20], tot_sums[maj][min].misc+r->e_stats.io_misc);
	printf(lines[21], tot_sums[maj][min].missing+r->e_stats.io_unlog);
	printf(lines[0]);

	(*func[maj])();		/* print device dependent data */
}

wnmap( dev )
{
	int min;

	min = ( minor( dev ) >> 5 ) & 3;
	if ( ( minor( dev ) & 31 ) >= 15 )
		min += 4;
	return ( min );
}

wnblk()
{
	struct i215eregs *r;
	struct i215iopb *iopb;
	struct i215err *err;
	uint *p;

	need( 15 );

	r = (struct i215eregs *)ercd.ebb.reginf;
	iopb = (struct i215iopb *)&r->e_iopb;
	err = (struct i215err *)&r->e_error;

	printf( "\n\tiSBC 215 iopb:\n" );
	printf( "\t\tDevice code %d, unit number 0x%02x\n", iopb->i_device,
	    (uint)iopb->i_unit );
	printf( "\t\tFunction code %d - %s\n",
	    (uint)iopb->i_funct,
	    f215line[ iopb->i_funct ] );
	printf( "\t\tModifier %x ", iopb->i_modifier );
	afix( WDLEN, iopb->i_modifier, wmodbit );
	printf( "\t\tCylinder %d, Head %d, Sector %d\n",
	    iopb->i_cylinder, (uint)iopb->i_head, (uint)iopb->i_sector );
	printf( "\t\tTransfer address 0x%lx\n", iopb->i_addr );
	printf( "\t\tRequested count %d, actual count %d\n",
	    iopb->i_xfrcnt, iopb->i_actual );

	printf( "\n\tiSBC 215 error status:\n" );
	printf( "\t\tHard status 0x%04x  ", err->e_hard );
	afix( WDLEN, err->e_hard, whardbit );
	printf( "\t\tSoft status 0x%02x  ", (uint)err->e_soft );
	afix( WDLEN, (uint)err->e_soft, wsoftbit );
	p = (uint *) &err->e_req_cyl_l;
	printf( "\t\tRequested - cyl %d, head %d, sec %d\n",
	    *p, (uint)err->e_req_head, (uint)err->e_req_sec );
	p = (uint *) &err->e_act_cyl_l;
	printf( "\t\tActual    - cyl %d, head %d, sec %d\n",
	    *p, (uint)err->e_act_head, (uint)err->e_act_sec );
	printf( "\t\tRetry count %d\n", err->e_retries );
}

cleanse(p, q)
	register char *p;
	register int q;
{
	while(q--)
		*p++ = '\0';
}

afix(a, b, c)
int a;
unsigned b;
char **c;
{
	register i;
	cleanse(interp,MAXSTR);
	for(i = 0; i < a; i++)  {
		if ((b & (1<<i)) && (*c[i])) {
			if ((strlen(c[i]) + strlen(interp)) >= MAXSTR) {
				concat(",", interp);
				printf("\t%s\n", interp);
				line++;
				cleanse(interp,MAXSTR);
			}
			else {
				if (*interp)
					concat(",", interp);
			}
			concat(c[i], interp);
		}
	}
	if (*interp)
		printf("%s\n", interp);
	else
		putchar('\n');
}
puthead(h)
char *h;
{
	printf("\n\n   %s   Prepared on %s     Page  %d\n\n\n\n",
		h, htime, page);
	line = 6;
}
inithdng()
{

	char *cbuf;
	time(&tloc);
	cbuf = ctime(&tloc);
	cbuf[16] = '\0';
	strcpy(htime, cbuf + 4);
}
putft()
{
	while (line++ < MAXLEN)
		putchar('\n');
	page++;
}
trnpg()
{
	if ( line >= MAXLEN)
		page++;
	else
		putft();
	if (page <= limit)
		puthead(header);
}
need(a)			/* acts like ".ne" command of nroff */
int a;
{
	if ( line > (PGLEN - a))
		trnpg();
	line += a;
}
gtime(tptr, pt)
char *pt;
time_t	*tptr;
{
	register int i;
	register int y, t;
	int d, h, m;
	long nt;

	t = gpair(pt++);
	if (t < 1 || t > 12)
		return(1);
	pt++;
	d = gpair(pt++);
	if (d < 1 || d > 31)
		return (1);
	pt++;
	h = gpair(pt++);
	if (h == 24) {
		h = 0;
		d++;
	}
	pt++;
	m = gpair(pt++);
	if (m < 0 || m > 59)
		return (1);
	pt++;
	y = gpair(pt++);
	if (y < 0) {
		time(&nt);
		y = localtime(&nt)->tm_year;
	}
	*tptr = 0;
	y += 1900;
	for(i = 1970; i < y; i++)
		*tptr += dysize(i);
	/* Leap year */
	if (dysize(y) == 366 && t >= 3)
		*tptr += 1;
	while(--t)
		*tptr += dmsize[t - 1];
	*tptr += d - 1;
	*tptr = (*tptr * 24) + h;
	*tptr = (*tptr * 60) + m;
	*tptr *= 60;
	*tptr += timezone;
	if (localtime(tptr)->tm_isdst)
		 *tptr -= 60 * 60;
	return(0);

}
gpair(pt)
char *pt;
{
	register int c, d;
	register char *cp;

	cp = pt;
	if (*cp == 0)
		return(-1);
	c = (*cp++ - '0') * 10;
	if (c < 0 || c > 100)
		return(-1);
	if (*cp == 0)
		return(-1);
	if ((d = *cp++ - '0') < 0 || d > 9)
		return(-1);
	return (c + d);
}

wanted ()
{
	/* Starting - ending limitations? */
	if (e_hdr.e_time < xtime ) {
		if (e_hdr.e_type != E_BLK)
			return 0;
		(*func[maj])();
		base_sums[maj][min].totalio = recrd.e_stats.io_ops;
		base_sums[maj][min].misc = recrd.e_stats.io_misc;
		base_sums[maj][min].missing = recrd.e_stats.io_unlog;
		return (0);
	}
	if (e_hdr.e_time > etime)
		return (0);
	/* Only fatal error flag? */
	if ((fflg) && (e_hdr.e_type == E_BLK) &&
	    !(recrd.e_bflags & E_ERROR))
		return(0);
	/* Stray interrupts or parity errors to be considered */
	if ((aflg) || ((e_hdr.e_type == E_STRAY) && (optdev & (1 << INT))) ||
		((e_hdr.e_type == E_PRTY) && (optdev & (1 << MEM)))) {
		print = YES;
		return (1);
	}
	/* Device chosen for consideration or printing? */
	if (dflg == 0) {
		print = NO;
		return(1);
		}
	if ((1 << maj) & optdev) {
		print = YES;
		return(1);
		}
	print = NO;
	return(0);
}
error(s1, s2)
char *s1, *s2;
{
	fprintf(stderr, "errpt: %s %s \n", s1, s2);
	exit(16);
}

recov()
{
	struct errhdr *p,*q;
	int i;
	for(;;) {
		p = q = &e_hdr;
		q++;
		for(i = 0; i < ((sizeof(struct errhdr) / 2) - 1); i++)
			*p++ = *q++;
		fread(p, 2, 1, file);
		if (feof(file))
			return(0);
		if (valid())
			return (1);
	}
}
valid()
{
	switch(e_hdr.e_type) {
		default:
			return(0);
		case E_GOTS:
		case E_GORT:
		case E_STOP:
		case E_TCHG:
		case E_BLK:
		case E_STRAY:
		case E_CCHG:
		case E_PRTY:
			if ((e_hdr.e_len < MINREC) ||
				 (e_hdr.e_len > MAXREC) )
				return (0);
			if ((e_hdr.e_time < atime) ||
				 (e_hdr.e_time > tloc))
				return(0);
			return (1);
	}
}
lower(str_ptr)
char *str_ptr;
{
	for(; *str_ptr; str_ptr++)
		*str_ptr = tolower(*str_ptr);
}

char *
findname()
{
	switch (DRIVE_TYPE){

	case WN0:
		return "WN0";

	default:
		return "UNKNOWN";
	}
}
concat(a, b)
	register char *a,*b;
{
	while (*b) b++;
	while (*b++ = *a++);
}
setime()
{
	if (e_hdr.e_time < fftime)
	fftime = e_hdr.e_time;
	if (e_hdr.e_time > ltime)
	ltime = e_hdr.e_time;
}

adjustsum(i, j)
register int i, j;
{
	tot_sums[i][j].totalio += sums[i][j].totalio;
	tot_sums[i][j].misc += sums[i][j].misc;
	tot_sums[i][j].missing += sums[i][j].missing;
	sums[i][j].totalio = 0;
	sums[i][j].misc = 0;
	sums[i][j].missing = 0;
	if (base_sums[i][j].totalio &&
	    base_sums[i][j].totalio < sums[i][j].totalio) {
		tot_sums[i][j].totalio -= base_sums[i][j].totalio;
		tot_sums[i][j].misc -= base_sums[i][j].misc;
		tot_sums[i][j].missing -= base_sums[i][j].missing;
		base_sums[i][j].totalio = 0;
		base_sums[i][j].misc = 0;
		base_sums[i][j].missing = 0;
	}
}
printsum()
{
	int i;
	header = hd2;
	page = 1;
	puthead(header);
	need(11);
	printf(sumlines[12], choice[0] ? choice : "All");
	printf(sumlines[13]);
	if (xtime) {
		printf("On or after %s", ctime(&xtime));
		printf(sumlines[14]);
	}
	else
		line--;
	if (etime != 017777777777L) {
		printf("On or before %s", ctime(&etime));
		printf(sumlines[14]);
	}
	else
		line--;
	if (fflg) {
		printf("Only fatal errors are printed.\n");
		printf(sumlines[14]);
	}
	else
		line--;
	if (limit != 10000)
		printf("Printing suppressed after page %d.\n", limit);
	else
		line--;
	printf(lines[0]);
	printf(sumlines[10], ctime(&fftime));
	printf(sumlines[11], ctime(&ltime));
	printf(lines[0]);
	if (readerr)
		printf(sumlines[7], readerr);
	else
		printf(lines[0]);
	printf(lines[0]);
	if ((optdev & (1 << INT)) || !(dflg) || (aflg)) {
		need(3);
		printf(lines[0]);
		printf(sumlines[9], straysum);
		printf(lines[0]);
	}
	if ((optdev & (1 << MEM)) || !(dflg) || (aflg)) {
		need(3);
		printf(lines[0]);
		printf(sumlines[8], parsum);
		printf(lines[0]);
	}
	if ((dflg == 0) || aflg)
		for(i = 0; i < NMAJOR; i++)
			prsum(i);
	else
		for(i = 0; i < NMAJOR; i++)
			if (optdev & (1<<i))
				prsum(i);
	if (line == 7)
		printf("No errors for this report\n");
}

prsum(i)
register int i;
{
	register int j;

	for(j = 0; j < NMINOR; j++) {
		if (tot_sums[i][j].totalio || sums[i][j].totalio) {
			need(10);
			printf(sumlines[1], sums[i][j].drvname, j);
			printf(sumlines[0]);
			printf(sumlines[2], sums[i][j].hard);
			printf(sumlines[3], sums[i][j].soft);
			printf(sumlines[4], tot_sums[i][j].totalio);
			printf(sumlines[5], tot_sums[i][j].misc);
			printf(sumlines[6], tot_sums[i][j].missing);
			printf(sumlines[0]);
		}
	}
}
