/*
 *	Modification history:
 *		M000 uport!bernie Tue Jan 13 15:46:23 PST 1987
 *      To accomodate getype.c, the variable name tabbuf was changed to
 *				tabbuffer and a new variable tabbuf was declared.
 *      To always show disk parameters found and give user an
 *        opportunity to reject them.
 */

static char *uportid = "@(#)init.c	Microport Rev Id  1.3.8 10/21/86";
/* initialize for format */
static char sccsid[] = "@(#)init.c    1.0" ;

#include "sys/divvy.h"
#include <sys/stat.h>
#include <sys/cmos.h>
#define STARS "*********************************************************************"
#define BITCH(s1,s2)  printf("%s\n%s%s\n%s\n\007\007",STARS,s1,s2,STARS)
#define UNCYL(pt)  (pt->ecyl | (((int)(pt->esect & 0xC0)) << 2))
#define	NTRACKS	1	/* # of tracks formatted per ioctl */
#define FILE path[unit]
char *path[NUNITS] = { "/dev/rdsk/0s0","/dev/rdsk/1s0" };
struct per pers[NUNITS];	/* per-unit PER table */

#define i1010minor pers[0].miner
#define SIZE(i) pers[UNIT(i)].slice[SLICE(i)].p_nsec
#define START(i) pers[UNIT(i)].slice[SLICE(i)].p_fsec
#define SPC(i)  pers[UNIT(i)].drtab.dr_spc
#define WNDEVICE 4
#define WNDEV1 20

int fuldisks[NUNITS] = { 10 , 30 };	/* minor device #s for full disks */
struct partit *part[NUNITS];	/* per unit active partition pointers */
struct partit *pt;
dev_t fds[NUNITS];
dev_t fd;

int     bps = 512;    /* bytes / sector */
unsigned int endcyl;
byte *moveb();
int *sig,otto, i;
byte *er,*bt,*bad, savehead, buf0[512], buf1[512], tabbuf[512];  /* M000 */
byte *tabbuffer[2] = { buf0,  buf1 } ;  /* M000 */
#ifdef	BBOOTDRVTBL
unsigned char	*ptab;
#endif	BBOOTDRVTBL
unsigned int unit;
extern unsigned int tpc,spt,precomp;
extern unsigned long first ;
extern unsigned long last  ;
extern unsigned int interleave ;
extern unsigned int newboot;
extern char *device;

initialize()
{
	int i,invalid1,invalid0,numcyl,startcyl;
	struct stat buf,*statbuf;
	union un 
	{
		dev_t urdev;
		struct devide 
		{
			unsigned char min;
			unsigned char maj;
		}   id;
	} u;
	statbuf = &buf;
	if (stat(device,statbuf)) BOMB ("stat failure");
	u.urdev = buf.st_rdev;
	if (u.id.maj != WNDEVICE) return(0);
	invalid0 = init_unit(0);
	unit = (u.id.min >= WNDEV1 )? 1 : 0 ;
	if (unit) invalid1 = init_unit(unit);
  if (! fulldisk(u.id.min))
	  BOMB ("invalid device: must be entire disk");
	for ( i = 0; i < 512; ++i )    /* M000 */
		tabbuf[i] = tabbuffer[unit][i];
	get_drive_params(&pers[unit].drtab);
	for ( i = 0; i < 512; ++i )    /* M000 */
		tabbuffer[unit][i] = tabbuf[i];
	if ( last == -1) last = pers[unit].drtab.dr_ncyl - 1;
	tpc = pers[unit].drtab.dr_nfhead;
	spt = pers[unit].drtab.dr_nsec;
	precomp = pers[unit].drtab.dr_precomp;
	return(0);
	};

/****************************************************************************/
/*  check to see if current device is a full disk  */
/*  return true if so, false if not */
fulldisk(minr)
unsigned char minr;
{
	int i;
	for ( i = 0; i < NUNITS ;i++)
		if ( minr == fuldisks[i]) return (1);
	return(0);
}
/****************************************************************************/


/*  Initialize unit (drive) */

init_unit(unit)
  int unit;
{
	int i;

	if ((fds[unit] = open(FILE,2)) == -1) {
		printf("Can't open %s, errno=%d\n", FILE, errno);
		return(errno);
	}
	fd = fds[unit];			/* for getype.c */

	read(fds[unit],tabbuffer[unit],512);    /* dummy read to force wnsweep */

	if (read_sector(fds[unit], tabbuffer[unit], 0,0,1) ) {
		printf("Can't read %s, errno=%d\n", FILE, errno);
		return(errno);
	}
        if ( ckactpar() )  /* no active partition yet */
	{
		printf("Invalid partition table!\n");
		return(1);
	}
	part[unit] = pt;	/* partition table pointer for drive */

        er = (byte *) &pers[unit];
	/* read end-of-partition record -if any - to get drtabs,
	 * slice table, and pointer to first bad-track sector
	 */
	    /* unkludge cylinder */
	endcyl = UNCYL( pt);
	    /* partition endrecord -includes drvtab   */
	read_sector (fds[unit],er,endcyl,pt->ehead,pt->esect&0x1f);

        sig  = (int *)(er + SIGOFFSET);
        if ((*sig != 0xAA55) || (*(sig-1) != 0xAA55) )
	{
		printf ("Invalid Partition End Record!!\n");
		return(1);
	};

return(0);
}

/****************************************************************************/
ckactpar()
{
	int i,j,valid;
	struct partit *save;
	
        bt =  (tabbuffer[unit] + PTOFFSET); /* partition table pointer */
        pt = (struct partit *) (bt); /* partition table pointer */
	valid = 0;
        for (i= 1; i<=4 ; i++)
	{
                if (pt++->boot_ind == 0x80)
		{
			 valid = 1;
			save = pt;
		};
	};

        if (!valid )  /* no active partition yet */
		return(1);
	else pt = --save;	/* backup to active partition */
	if (pt->syst_ind != 5 && pt->syst_ind != 0x52)
		return(1);	/* active part not System5 */
	endcyl = pt->ecyl | (((int)(pt->esect & 0xC0)) << 2);
	return(0);
}
/****************************************************************************/
/****************************************************************************/
init_mbblk() /* initialize master boot block */
{
	int	j, fdx;
	if ((fdx = open("/etc/master.bblock", O_RDWR)) == -1) {
		fprintf(stderr, "Can't open %s, errno=%d\n", "/etc/master.bblock", errno);
		exit(errno);
	}

	read(fdx,tabbuffer[unit],512) ;	
	close(fdx);
	getdp(tabbuffer[unit] + DISKPARMS,&pers[unit].drtab);
	if (write_sector(fds[unit],tabbuffer[unit],0,0,1))
	{
		printf("Cant write master boot-block, error= %d\n",errno);
		return (errno);
	};
}
/****************************************************************************/
#include "../getype.c"
/****************************************************************************/
/*
get_disk_info(dt)
{
       	dt = &typer[0] + 15;
}
*/
/****************************************************************************/
