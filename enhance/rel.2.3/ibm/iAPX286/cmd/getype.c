
/* Modification history:
 * uport Rev Id: @(#)getype.c  2.3  5/8/87
 * M000 uport!bernie Thu Nov 20 17:34:29 PST 1986
 *	Allows fdisk to respond to negative drive types.
 * M002 uport!bernie Thu Jan 8 18:00:10 PST 1987
 *	Upgraded query() and getno().
 *	Improve clarity of disk parameter prompting.
 * M003 uport!bernie Thu Mar 26 17:29:47 PST 1987
 *	Included reference to unit number in info about drive params.
 * M004 uport!rex	Tue May 19 18:05:04 PDT 1987
 *	Add code to use a drive parameter table entry in the partition
 *	boot sector instead of drive_params in the master boot block
 *	All of the added code is ifdef'ed with BOOTDRVTBL
 */

static unsigned int type;
static unsigned long fdtypevec[2] =
			 { 0x104, 0x118 }  ; 	/* table pointer */
static unsigned long fdtypadrs;

#ifdef BOOTDRVTBL
#define BOOTDRTBLOFF	0x1ee			/* M004 */
unsigned char partbootsec[512];			/* M004 */
char	bsdptupdate = 0;
int	dptdbg = 0;
#endif BOOTDRVTBL

/* read what's currently in cmos. */

int	cmosfd;
readcmos()
{

	if ((cmosfd = open(CMOSDEV, 0)) == -1) {	/* M000 */
		perror("Opening cmos device");		/* M000 */
		exit(1);
	}
	if ( sizeof(struct cmos) != read(cmosfd, &cmos, sizeof(struct cmos))) {
		perror("Reading cmos device");		/* M000 */
		exit(1);
	}
	close(cmosfd);
}
/****************************************************************************/
/* write what's currently in cmos. */

writecmos()
{
	unsigned int  cksum, i;
	unsigned char *c;

        /* calculate new checksum */
	c = (unsigned char *) &cmos;
        cksum = 0;
        for(i=0x10; i<0x21; i++) {
                cksum += *(c+i);
                }
        /* Set new checksum */
        *(c + 0x2e ) = cksum >> 8;
        *(c + 0x2f ) = cksum ;

	if ((cmosfd = open(CMOSDEV, 1)) == -1) {	/* M000 */
		perror("Opening cmos device");		/* M000 */
		exit(1);
	}
	if ( sizeof(struct cmos) != write(cmosfd, &cmos, sizeof(struct cmos))) {
		perror("writing cmos device");		/* M000 */
		exit(1);
	}
	close(cmosfd);
}
/****************************************************************************/
/* check master boot drive params for valid checksum */
checkdp(dp)
struct dparams *dp;
{
	int *p;
	p = (int *)dp;
	return (docksum(p,4) == dp->checksum);
}

/* simple-minded checksum for integers */
docksum(p,n)
int *p,n;
{
	int k;
	k = 12345;
	for (;n ; n--) k ^= *p++;
	return k;
}

#ifdef BOOTDRVTBL			/* M004 */
/* move the drive parameter values from drivetable to partition boot sector
*/
dptopbs(p,dt)
	unsigned char	*p;
	struct	i1010drtab *dt;
{
	register unsigned short	*pi;
	register	i;

	pi = (unsigned short *) &(p[0]);
	for (i=0; i < 8; ++i)
		*pi++ = 0;
	pi = (unsigned short *) &(p[0]);
	*pi = dt->dr_ncyl;
	p[2] = dt->dr_nfhead;
	if (dt->dr_nfhead > 8)
		p[8] = 0x08;		/* control byte, set bit 3 */
	pi = (unsigned short *) &(p[5]);
	*pi = dt->dr_precomp;
	pi = (unsigned short *) &(p[12]);
	*pi = dt->dr_lzone;
	p[14] = dt->dr_nsec;
}
#endif BOOTDRVTBL			/* M004 */

/* get drive parameter values from drivetable */
getdp(p,dr)
struct dparams *p;
struct i1010drtab *dr;
{
	p->numcyls = dr->dr_ncyl;
	p->numheads = dr->dr_nfhead;
	p->precompcyl = dr->dr_precomp;
	p->landingzone = dr->dr_lzone;
	p->checksum = docksum(p,4);

}
/* put drive parameter values into drivetable */
putdp(p,dr)
struct dparams *p;
struct i1010drtab *dr;
{
	dr->dr_ncyl = p->numcyls; 
	dr->dr_nfhead = p->numheads ;
	dr->dr_precomp = p->precompcyl ;
	dr->dr_lzone = p->landingzone; 
	dr->dr_nsec = 17;
	dr->dr_secsiz = 512;
	dr->dr_spc = dr->dr_nsec * dr->dr_nfhead;

}
/****************************************************************************/
#define INT(byte) *(int *)&(byte)
#define MAXTYPE 23
/* get drive type from CMOS RAM
	confirm with user - if no type, get params from user */

get_drive_params(drivetab)
	struct i1010drtab  *drivetab;
{
    struct i1010drtab  *dt;
    int i, no_params;   /* M002 */

/* if parameter table in master boot block is ok, use that */

	no_params = 0;   /* M002 */
	if (checkdp(tabbuf+DISKPARMS))
	{
		putdp(tabbuf+DISKPARMS,drivetab);
		printf(
		    "\nUnit %d -- Drive parameters on disk:\n",unit);/* M003 */
		print_params(drivetab);	/* get drive table */
        	if (query("Is this correct?")) return 0;
		goto getype;
	}

#ifdef BOOTDRVTBL			/* M004 */

/*  so try the active unix partition boot sector where a ROM compatible
    drive parameter entry may be written */
    {
	struct partit  *pn;
	unsigned char  beginsec, *pc;
	unsigned int   begincyl;

#ifdef DEBUG
	fprintf(stderr,
	    "\n\tTEST VERSION of partition boot sector drive table\n\n");
#endif

	pn = (struct partit *) (tabbuf + PTOFFSET);
	for (i=0; i < 4; ++i, ++pn)		/* find active paratition */
	    if (pn->boot_ind == 0x80 && ( pn->syst_ind == 5 ||
					  pn->syst_ind == 0x52 ))
		break;
	if (i < 4)
	{
	    begincyl = ((pn->bsect & 0xC0) << 2) | pn->bcyl;
	    beginsec = pn->bsect & 0x3F;
	    read_sector (fd,partbootsec,begincyl,pn->bhead,pn->bsect&0x1f);
	    if ( *((int *) &partbootsec[0x1FE]) != 0xAA55
			 || partbootsec[0x1FD] != 0
			 || partbootsec[BOOTDRTBLOFF-1] != 0x90 )
	    {
		bsdptupdate = 2;	/* indicate not uport boot sector */
		goto nobsdpt;
	    }
	    pc = &partbootsec[BOOTDRTBLOFF];
	    if ( pc[2] != 0 && pc[14] == 0x11 )		/* looks good */
	    {
		dt = drivetab;
		dt->dr_ncyl = INT(pc[0]);
		dt->dr_nfhead = pc[2];
		dt->dr_precomp = INT(pc[5]);
		dt->dr_lzone = INT(pc[12]);
		dt->dr_nsec = pc[14];
		dt->dr_secsiz = 512;
		dt->dr_spc = pc[14] * pc[2];
		printf(
		    "Drive parameters from fixed disk unit %d\n", unit);
		print_params(drivetab);			/* get drive table */
        	if (query("Is this correct?"))
		    return 0;
		else
		    goto getype;
	    }
	    bsdptupdate = 1;
	}
    }
nobsdpt:

#endif BOOTDRVTBL			/* M004 */

/* if not, try reading ROM table */
	readcmos();
	type = unit ? cmos.disk & 0xF : cmos.disk >> 4 ;

	if (!getpar(unit,drivetab)) 	/* drive parms from ROM*/
	{
		no_params = 1;   /* M002 */
		goto getype;
	}
	if ((type == 0) || (type > MAXTYPE)) 
	{
		no_params = 1;   /* M002 */
		goto getype;
	}
	if (type == 15) type = unit ? cmos.diskD : cmos.diskC ;

        printf("\nUnit %d -- Drive type in CMOS RAM is %d\n",unit,type);
newtype:
	printf("This will cause the system to assume the following drive parameters:\n");
	print_params(drivetab);

        if (!query("Is this correct?"))
        {
getype:
		if ( no_params )   /* M002 */
		{
		    printf("\nThe parameters of your hard disk cannot be found.");
		    printf("\nPlease enter these parameters directly.\n");
		}
		get_disk_info(drivetab);
        };
/* prepare for putting drive parameters on the disk */
#ifdef BOOTDRVTBL
	dptopbs(&partbootsec[BOOTDRTBLOFF], drivetab); /* in boot sector */
#else	/* ! BOOTDRVTBL */
	getdp(tabbuf+DISKPARMS,drivetab);	/* in master boot block */
#endif	/* ! BOOTDRVTBL */

	return 1; /* indicate new mboot */
}
/****************************************************************************/

/****************************************************************************/
/*
getpar retrieves the current disk parameter vector from ROM
        Larry Weaver  October 86
*/

/****************************************************************************/
getpar(unit,dt)
	int unit;
    struct i1010drtab  *dt;
{
	unsigned char buf[20];
	int fd,i;
	long kk;
	if ((fd = open("/dev/mem", 2)) == -1) {
		printf("Can't open %s, errno=%d\n", "/dev/mem", errno);
		return 0;
		};
	if (lseek(fd,fdtypevec[unit],0) == -1)
	{
		printf("Can't seek to 0x%x !\n",fdtypevec);
		return 0;
	};
	if (read(fd,&fdtypadrs,4) != 4)
	{
		printf("Can't read fdtypevec!\n");
		return 0;
	};
#ifdef	BOOTDRVTBL
	if (dptdbg)
		printf("DPT vertor = 0x%lx.\n", fdtypadrs);
#endif	BOOTDRVTBL
	kk = (long)((long) hiword(fdtypadrs) << 4) + ((long)loword(fdtypadrs));
#ifdef	BOOTDRVTBL
	if (dptdbg)
		printf("DPT rom addr = 0x%lx.\n", kk);
#endif	BOOTDRVTBL
	if ( lseek(fd,kk,0) == -1)
	{
		printf("Can't seek to 0x%x !\n",kk);
		return 0;
	};
	if ((i = read(fd,buf,16)) != 16)
	{
		printf("Can't read entry at %lx :errno: %d!\n",fdtypadrs,errno);
		return 0;
	};
/* the following kludge is due to the wonderful C compiler which
	forces words in structures to even byte addresses.
	All hail the holy memory of the PDP-11 !!!!
*/
	dt->dr_ncyl = INT(buf[0]);
	dt->dr_nfhead = buf[2];
	dt->dr_precomp = INT(buf[5]);
	dt->dr_lzone = INT(buf[12]);
	dt->dr_nsec = buf[14];
	dt->dr_secsiz = 512;
	dt->dr_spc = buf[14] * buf[2];

	close(fd);
	return 1;	/* true return if no problems */
}

/**********************************************************************/
putypvec(unit)
	int unit;
{
	int fd,i;
/*  update type in CMOS */
	if (type > 15)
	{
	 	if (unit) cmos.diskD = type; else cmos.diskC = type;
		type = 15;
	};
	if (unit) cmos.disk = (cmos.disk & 0xF0) | type;
	 else cmos.disk = (cmos.disk & 0x0F) | (type << 4);
printf("putypvec: writecmos: cmos.disk: %x\n",cmos.disk);
	writecmos();

/* update table pointer in interrupt table */
	if ((fd = open("/dev/mem", 2)) == -1) {
		printf("Can't open %s, errno=%d\n", "/dev/mem", errno);
		return 0;
		};
	if (lseek(fd,fdtypevec[unit],0) == -1)
	{
		printf("Can't seek to 0x%x !\n",fdtypevec);
		return 0;
	};
	if (write(fd,&fdtypadrs,4) != 4)
	{
		printf("Can't write fdtypevec!\n");
		return 0;
	};
	close(fd);
	return 1;
}
/****************************************************************************/

print_params(dt)
struct i1010drtab *dt;
{
	printf("Cylinders\tTracks/Cylinder\t   Landing Zone\t\tWrite Precomp\n");
	printf("  %d\t\t   %d\t\t\t%d\t\t   %d\n\n",
		dt->dr_ncyl, dt->dr_nfhead,
		dt->dr_lzone,
		dt->dr_precomp);
	printf(" All drives must be formatted with 17 512-byte sectors per track.\n");
	printf(" Note that tracks/cylinder equals heads/cylinder.\n");
}

/****************************************************************************/
get_disk_info(dt)
struct i1010drtab *dt;
{
    do {
	while ((dt->dr_ncyl = getno("Enter number of cylinders:"))
							 > 1024)
		printf("\n Maximum of 1024 cylinders allowed!") ;
        dt->dr_nfhead=getno("Enter number of heads (or tracks) per cylinder:");
        dt->dr_nsec = 17; /* force 17 secs/track */
        dt->dr_lzone = getno("Enter cylinder no. for landing zone:");
        dt->dr_precomp = getno("Enter cylinder no. for write precompensation:");
	dt->dr_spc = dt->dr_nfhead * dt->dr_nsec;
	dt->dr_secsiz= 512;
	dt->dr_control = dt->dr_nfhead > 8 ? 8 : 0 ;
	printf("\nThese are the current drive parameters:\n"); /* M002 */
	print_params(dt);
    } while (!query("Is this correct"));
}

/****************************************************************************/
/* 
 * This function prompts for a number and returns it.
 */
getno(string)          /* M002 */
char *string;
{
    int i,j;
    char buf[81];

    while (1)
    {
        printf("\n %s ",string);
	if ( fgets(buf, 81, stdin) != NULL )   /* M002 */
	{
	    i = sscanf(buf, "%d", &j);  
	    if ( i == 1 )
		return(j);
	}
        printf("\n Please reply with an integer.");
    }
}

/****************************************************************************/
/*
 *  moveb  (from,to,length) moves a structure 
 */
byte *moveb (from, to, length )
byte *from,*to;
int length;
{
	for ( ;length;length -- ) *to++ = *from++ ;
	return (to);
}

/****************************************************************************/
/* read or write a sector by disk address (cyl,head,sector)     */
read_sector(dev,buf,cyl,head,sector)
dev_t dev;
byte *buf,head,sector;
int cyl;
{

	int i;
	struct i1010iopb *io, iopb;
	io = &iopb;
	io->i_addr = (long) buf;
	io->i_actcylinder = cyl;
	io->i_acthead = head;
	io->i_sector = sector;
	io->i_funct = WD_READ_OP;
	i = ioctl(dev,I1010_RAWIO,io);
	return(i);
}

/**************************************************************************/
write_sector(dev,buf,cyl,head,sector)
dev_t dev;
byte *buf,head,sector;
int cyl;
{
	int i;
	struct i1010iopb *io, iopb;
#ifdef DEBUG
return(0);
#endif
	io = &iopb;
	io->i_addr = (long) buf;
	io->i_actcylinder = cyl;
	io->i_acthead = head;
	io->i_sector = sector;
	io->i_funct = WD_WRITE_OP;
	i = ioctl(dev,I1010_RAWIO,io);
/*
	if (i) printf("\nWrite failed,errno= %d\n",i);
*/
	return(i);
}

/**************************************************************************/
raw_io(dev,buf,cyl,head,sector,op)
dev_t dev;
byte *buf,head,sector;
int cyl,op;
{
	int i;
	struct i1010iopb *io, iopb;
	io = &iopb;
	io->i_addr = (long) buf;
	io->i_actcylinder = cyl;
	io->i_acthead = head;
	io->i_sector = sector;
	io->i_funct = op;
	i = ioctl(dev,I1010_RAWIO,io);
#ifdef DEBUG
	if (i) printf("\nRaw I/O op %d failed,errno= %d\n",op,i);
#endif
	return(i);
}

/**************************************************************************/
/* 
 * This function queries the user, and returns 1 if response is yes 
 * or 0 if response is no.
 */
query(string)     /* M002 */
    char *string;
{
    char  tstr[81], *tp;

    while (1) 
    {
        printf("\n%s (y or n): ",string);
        tp = fgets(tstr, 81, stdin);  /* M002 */
        if ( tp == NULL ) 
	    return(0);
        switch (*tp) 
        {
             case 'Y':
             case 'y':
		 return(1);
             case 'N':
             case 'n':
		 return(0);
             default:
		 printf("\nPlease reply with yes or no.");
		 break;
	}
    }
}
