/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	@(#)	1.3	*/
	/*  setup.c: 1.9 7/18/83 */

/*
 *	UNIX debugger
 *
 * 		Everything here is MACHINE DEPENDENT
 *			and operating system dependent
 */

#include "head.h"
#include "coff.h"

extern MSG		BADMAGN;

INT	wtflag = 0;	/* set to allow writing to a.out, core, ISP */

static L_INT		txtsiz;
static L_INT		datsiz;
static L_INT		datbas;
static L_INT		stksiz;
static L_INT		symbas;		/*  not using ?? */
static L_INT		entrypt;
extern INT		argcount;
extern INT		signo;
extern INT		magic;
extern int		errno;

extern STRING		symfil;
extern STRING		corfil;

#define AOUTHDRSIZ	(filhdr.f_opthdr)

#define	maxfile		(1L<<24)

FILHDR	filhdr;
AOUTHDR	aouthdr;	/* a.out Unix (optional) header */
#if iAPX286
struct sdbsegdata  *segdatap;
SCNHDR *scnhdrp;
int * scnhdri;
#define AOUT2MAGIC	0410
#else
SCNHDR	*scnhdrp;	/* pointer to first section header (space by sbrk) */
#endif

setsym()
{
#if iAPX286
	int i;
#endif
	fsym = getfile(symfil,1);
	txtmap.ufd = fsym;
	errno = 0;
	if (read(fsym,&filhdr,FILHSZ) == FILHSZ&&
	   ISMAGIC(filhdr.f_magic)		&&
	   AOUTHDRSIZ == sizeof aouthdr		&&
	   read(fsym, &aouthdr, AOUTHDRSIZ)==AOUTHDRSIZ)
	{
		magic=aouthdr.magic;
#if iAPX286
		model = M_UDEF;
		if (filhdr.f_magic == I286SMAGIC)
			model = M_SMALL;
		if (filhdr.f_magic == I286LMAGIC)
			model = M_LARGE;
		if (model && magic == AOUT2MAGIC
		    && (txtmap.noscn = rdschdrs())!=0) {
			txtmap.mask = 0x0ffff;
			txtmap.segdatap = segdatap;
			txtmap.shift = CSSHIFT;
			txtmap.offset = STARTSEG;
			magic286 = aouthdr.entry;
		}
		/* should there be an else here that
		 * sets this up as a NON-a.out?
		 */
	}
	else /* this is not an aout or o file */
	{
		setupmax(&txtmap);
		entrypt = 0;
	}
#else
		if (magic==OMAGIC   || 	/* Writable text: private */
		    magic==PMAGIC   ||	/* Paging text: private.  */
		    magic==NMAGIC)	/* Readonly text: private */
		{
			txtsiz=aouthdr.tsize;
			datsiz=aouthdr.dsize;
			symbas=txtsiz+datsiz;
			rdschdrs();

			switch (magic) {
			/*  use to have many more "magic" cases here */
			/*	assuming text is first section */

			case OMAGIC:	/* 0407 */
#if vax || u3b
				txtmap.b1=0;
#else
#if u3b5
				txtmap.b1=scnhdrp[0].s_vaddr;
#endif
#endif
				txtmap.e1=txtmap.b1 + symbas;
				txtmap.f1=scnhdrp[0].s_scnptr;
				txtmap.b2=datbas=scnhdrp[0].s_paddr;
				txtmap.e2=symbas;
				txtmap.f2=txtmap.f1;
				break;

			case PMAGIC:	/* 0413 */
			case NMAGIC:	/* 0410 */
#if vax || u3b
				txtmap.b1=0;
#else
#if u3b5
				txtmap.b1=scnhdrp[0].s_vaddr;
#endif
#endif
				txtmap.e1=txtmap.b1 + txtsiz;
				txtmap.f1=scnhdrp[0].s_scnptr;
#if vax || u3b
				txtmap.b2=datbas=scnhdrp[0].s_paddr;
#else
#if u3b5
				txtmap.b2=datbas=scnhdrp[1].s_vaddr;
#endif
#endif
				txtmap.e2=datbas+datsiz;
#if vax || u3b
				txtmap.f2=txtmap.f1+txtmap.e1;
#else
#if u3b5
				txtmap.f2=scnhdrp[1].s_scnptr;
#endif
#endif
				break;

			}
			entrypt = aouthdr.entry;
		}
		else {
			magic = 0;
			fprintf(FPRT1, "Warn: No magic for %s;\n", symfil);
		}
	}
	else {		/*  may be a ".o" file */
		if (ISMAGIC(filhdr.f_magic))
		{
			magic = filhdr.f_magic;
			rdschdrs();
			/* assuming 3 sections; text, data, and bss */
			txtsiz = scnhdrp[0].s_size;
			datsiz = scnhdrp[1].s_size;
			symbas = txtsiz+datsiz;
			txtmap.b1 = 0;
			txtmap.e1 = txtsiz;
			txtmap.f1 = scnhdrp[0].s_scnptr;
			txtmap.b2 = datbas = scnhdrp[0].s_paddr;
			txtmap.e2 = txtsiz+datsiz;
			txtmap.f2 = scnhdrp[1].s_scnptr;
			entrypt = 0;
		}
	}
#if DEBUG > 1
	if(debugflag)
		fprintf(FPRT2, "magic=%#o;\n", magic);
#endif
	if (magic == 0)
	{
		txtmap.e1 = maxfile;
	}
#endif
}

setcor()
{
#if iAPX286
struct seg_desc segtab;
int nosegs;
int i;       /* count up to ldt nosegs */
long curfilep;  /* current position within the file */
#endif
	datmap.ufd = fcor = getfile(corfil,2);
	if(fcor < 0)
	{
		return;
	}
	   /*  sure a core file */
	if (read(fcor, uu, XTOB(USIZE))==XTOB(USIZE)
	   && magic
#if !(u3b5 || iAPX286)
	   && magic == ((struct user *)uu)->u_exdata.ux_mag
#endif
#if iAPX286

	   && (((struct user *)uu)->u_tss.ts_sp0 == SSTKPTR
		&&((struct user *)uu)->u_tss.ts_ss0 == SSTKSEG
		&&((struct user *)uu)->u_tss.ts_sp1 == 0
		&&((struct user *)uu)->u_tss.ts_ss1 == 0
		&&((struct user *)uu)->u_tss.ts_sp2 == 0
		&&((struct user *)uu)->u_tss.ts_ss2 == 0
	      )
#endif
/*
** ANDF (((struct user *)uu)->u_pcb.pcb_ksp & 0xF0000000L)==0x80000000L removed
** ANDF (((struct user *)uu)->u_pcb.pcb_usp & 0xF0000000L)==0x70000000L	removed
*/
	   )
	{
#if vax || u3b || iAPX286
	/* ((struct user *)uu)->u_ar0 is an absolute address, currently
	   0x7fff ffb8 in the VAX, and 0x00a0 07b8 in the 3B-20.
	   It must be converted to an address relative to the beginning
	   of UBLOCK, which is done by subtracting the absolute address
	   of UBLOCK (ADDR_U, #defined in machdep.h) to get the offset
	   from the beginning of the user area, and then adding the
	   sdb internal addess of the user area (uu).		*/

#if iAPX286
	((struct user *)uu)->u_ar0 =
/* should be:-								*/
/*		        (int *) ((char *)(((struct user *)uu)->u_ar0) - */
/*			(char *)ADDR_U + (char *)uu);		 	*/
/* but compiler has troubles						*/
			(int *) ((long)(((struct user *)uu)->u_ar0) -  
			(long)ADDR_U + (long)uu);		 	

		signo = ((struct user *)uu)->u_ar0[ARG0]&0x1f;
#else
	((struct user *)uu)->u_ar0 =
		(int *) ((char *)(((struct user *)uu)->u_ar0) -
			ADDR_U + (int)uu);
		signo = ((struct user *)uu)->u_arg[0]&037;
#endif
#else
#if u3b5
		signo = 9;	/* signal number unavailable */
#endif
#endif
		txtsiz = XTOB(((struct user *)uu)->u_tsize);
		datsiz = XTOB(((struct user *)uu)->u_dsize);
		stksiz = XTOB(((struct user *)uu)->u_ssize);
#if DEBUG > 1
		if(debugflag > 1) {
			fprintf(FPRT2,
			    "((struct user *)uu)->u_tsize=%#x; ((struct user *)uu)->u_dsize=%#x; ((struct user *)uu)->u_ssize=%#x;\n",
				((struct user *)uu)->u_tsize, ((struct user *)uu)->u_dsize, ((struct user *)uu)->u_ssize);
			fprintf(FPRT2, "txtsiz=%#x; datsiz=%#x; stksiz=%#x;\n",
				txtsiz, datsiz, stksiz);
		}
#endif
#if iAPX286
		curfilep = 0;
		nosegs = (XTOB(((struct user *)uu)->u_lsize)/SEGTESIZ);
		datmap.noseg = nosegs;
		datmap.noscn = 0;
		datmap.segdatap = (struct sdbsegdata *)sbrk(nosegs*(sizeof (struct sdbsegdata)));
		datmap.mask = 0x0ffff;
		datmap.shift = CSSHIFT;
		datmap.offset = 0;
		for(i=0;i<nosegs;i++) {
			if (read(fcor,&segtab,SEGTESIZ)==SEGTESIZ) {
				if (i < UPAGE_SEL)
				{
					continue;
				}
				if (segtab.sd_access&SEGSYS) {
					if (segtab.sd_access&SEGCODE)
					 { 
				/* text seg no area dumped  */
						datmap.segdatap[i].segtype=
								SEG_TEXT;
						datmap.segdatap[i].segvsize= 
							      ((unsigned long)segtab.sd_limit)+1;
						}
					else 	{
						datmap.segdatap[i].segtype=SEG_DATA
							| SEG_AVAIL;
						datmap.segdatap[i].segvsize=	((long)(segtab.sd_limit)&0xffff)+1;
						if (segtab.sd_access&EXPDOWN)
						{
						datmap.segdatap[i].segtype=
							SEG_STACK | SEG_AVAIL;
						datmap.segdatap[i].segvaddr=datmap.segdatap[i].segvsize;
						datmap.segdatap[i].segvsize=(long)0x10000 - datmap.segdatap[i].segvsize;
						}
						datmap.segdatap[i].coreoff=	curfilep;
						curfilep += datmap.segdatap[i].segvsize;
						if (i==8)
						{
						curfilep += XTOB(((struct user *)uu)->u_lsize);
						}
						}
					}	
				else {
					datmap.segdatap[i].segtype = 0;
				}
			}
			else {
				fprintf(FPRT1,"Warn: core file read error\n");
				break;
			}
		} /* end of for loop */
	}  /* end the if clause */
	else
	{
/* set uu back to zero */
		for(i=0;i<XTOB(USIZE);i++)
			uu[i]='\0';
		setupmax(&datmap);
		fprintf(FPRT1,"warning: '%s' not a core file. \n", corfil);
		fakecor = 1;
	}
#else
#if vax || u3b
		if( magic == NMAGIC || magic == PMAGIC )
		{
			datmap.b1=datbas=scnhdrp[0].s_paddr;
		}
		else
		{
			datmap.b1=datbas=0;
		}
		datmap.e1=(magic==OMAGIC?txtsiz:datmap.b1)+datsiz;
		datmap.f1 = XTOB(USIZE);
#if u3b
		datmap.b2 = USRSTACK;	/* loc of usr stack, in <sys/seg.h> */
		datmap.e2 = USRSTACK + stksiz;
#else
		/* VAX stack grows down from USRSTACK, in <sys/param.h> */
		/*  stksiz includes sizeof user area - must add back */
		datmap.b2 = USRSTACK + XTOB(USIZE) - stksiz;
		datmap.e2 = USRSTACK;
#endif
		if( magic == NMAGIC || magic == PMAGIC )
		{
			datmap.f2 = XTOB( USIZE ) + datsiz;
		}
		else
		{
			datmap.f2 = XTOB( USIZE ) + datmap.e1;
		}
#else
#if u3b5
		datmap.b1 = 0x800000 + round(txtsiz,TXTRNDSIZ);
		datmap.e1 = datmap.b1 + datsiz;
		datmap.f1 = XTOB(USIZE);	/* data right after ublock in file */
		datmap.b2 = USRSTACK;		/* stack always starts here */
		datmap.e2 = USRSTACK + stksiz;
		datmap.f2 = datmap.f1 + datsiz;	/* stack in file after data */
#endif
#endif
#if u3b5
		{
		unsigned int	uar0,file_adr;
		int i;

		/* copy registers from core dump into regvals */
		uar0 = (unsigned int) (((struct user *) uu)->u_ar0);
		for (i = 0; i < NUMREGS; i++)
			if (reglist[i].roffs != -1)
			{
				file_adr = uar0 + WORDSIZE * reglist[i].roffs -
						datmap.b2 + datmap.f2;
				lseek(datmap.ufd,file_adr,0);
				read(datmap.ufd,&(regvals[i]),sizeof(long));
			}
		}
#endif

/* ??	signo = *(ADDR *)(((ADDR)uu)+XTOB(USIZE)-4*sizeof(int)); */
#if DEBUG > 1
		if(debugflag > 0)
			fprintf(FPRT2, "((struct user *)uu)->u_arg[0]=%#x; signo=%#x;\n",
					((struct user *)uu)->u_arg[0], signo);
#endif
	      /*  put test up front, so don't need here
	      ** if (magic && magic != ((struct user *)uu)->u_exdata.ux_mag) {
	      **	fprintf(FPRT1, "%s: 0%o, 0%o\n",
	      **		BADMAGN, magic, 
	      **		((struct user *)uu)->u_exdata.ux_mag);
	      ** }
	      */
	}
	else		/*  e.g. /dev/kmem or any ordinary file */
	{
		datmap.e1 = datmap.e2 = maxfile;
		datmap.b1 = datmap.f1 = 0;
		datmap.b2 = datmap.f2 = 0;
		fprintf(FPRT1, "Warning: `%s' not a core file.\n", corfil);
		fakecor = 1;
	}
#endif
}

create(f)
STRING	f;
{	int fd;
	if ((fd=creat(f,0644)) >= 0) {
		close(fd);
		return(open(f,wtflag));
	}
	else return(-1);
}

getfile(filnam,cnt)
STRING	filnam;
{
	REG INT		fd;

	if (!eqstr("-",filnam))
	{
		fd=open(filnam,wtflag);
		if (fd<0 && argcount>cnt)
		{
			if (fd < 0)
			{
				fprintf(FPRT1, "cannot open `%s'\n", filnam);
			}
		}
	}
	else
	{
		fd = -1;
	}
	return(fd);
}

/* rdschdrs(): read section headers */
rdschdrs()
{
#if iAPX286
	int i,j,k,nosec;
	SCNHDR *shp;
	struct sdbsegdata *sgp;
	unsigned short type;

	scnhdrp = (SCNHDR *) sbrk((nosec=filhdr.f_nscns)*sizeof(SCNHDR));
	segdatap = (struct sdbsegdata *) sbrk((txtmap.noseg=CODE1_SEL
			+ nosec)*sizeof(struct sdbsegdata));
	scnhdri = (int *) sbrk(nosec*WORDSIZE);
	if (read(fsym,scnhdrp,nosec*sizeof(SCNHDR)) != nosec*sizeof(SCNHDR)) {
		fprintf(FPRT1, "Warn: section header read error\n");
		return(0);
	}
	for (j=0;j!=nosec;j++) scnhdri[j] = 0;	
	for (shp=scnhdrp,k=0;k < nosec;shp++,k++)
	{
		if (!(shp->s_flags & SCN_USED)) continue;
		type = SEG_DATA | SEG_AVAIL;
		if (shp->s_flags & STYP_TEXT)
			type = SEG_TEXT | SEG_AVAIL;
		j = shp->s_vaddr>>19;
		if (j < CODE1_SEL || j >= txtmap.noseg) {
			fprintf(FPRT1, "Warn: segment number out of range\n");
			fprintf(FPRT1, "      file not a user-level a.out\n");
			return(0);
		}
		scnhdri[k]=j;
		sgp = &segdatap[j];
		if (sgp->scncnt == 0) {

			sgp->scncnt = 1;
			sgp->segtype = type;
			sgp->scnptr = shp;
			sgp->segnum = j;
			sgp->segvaddr = shp->s_vaddr;
			sgp->segvsize = (shp->s_size+511) & ~511;
		} else {
			sgp->scncnt++;
			i = shp->s_size + sgp->segvaddr - shp->s_vaddr;
			sgp->segvsize = (i + 511) & ~511;
		}
	}
	return(nosec);
#else
	register unsigned nb;
	extern STLHDR filhdr;		/* a.out file header */

	nb = filhdr.f_nscns * SCNHSZ;
	scnhdrp = (SCNHDR *) sbrk(nb);
	if (read(fsym, scnhdrp, nb) != nb)
		fprintf(FPRT1, "Warn: section header read error\n");
	/* chkerr();	 does longjmp and haven't done setjmp yet */
	return(filhdr.f_nscns);
#endif
}
#if iAPX286
setupmax(area)
MAP * area;
{
	segdatap = (struct sdbsegdata *)sbrk(sizeof(struct sdbsegdata));
	area->segdatap = segdatap;
	segdatap->scncnt = 0;
	area->mask = 0xffffffff;
	area->offset = 0;
	area->shift = 16;
	area->noscn = 0;
	area->noseg = 1;
	segdatap->segtype = SEG_ODD|SEG_AVAIL;
	segdatap->segvsize = maxfile;
}

#if DEBUG
DBprmaps()
{
	DBprmap(&txtmap, "text map");
	DBprmap(&datmap, "data map");
}

DBprmap(amap, s)
MAPPTR amap;
char * s;
{
	struct sdbsegdata *sgp;
	SCNHDR *shp;
	unsigned short gcnt, hcnt;

	fprintf(FPRT2, "Initial contents for %s:\n", s);
	fprintf(FPRT2, "\tsegdatap=0x%lx\n", amap->segdatap);
	fprintf(FPRT2, "\tnoscn=%d\n", amap->noscn);
	fprintf(FPRT2, "\tnoseg=%d\n", amap->noseg);
	fprintf(FPRT2, "\tmask=0x%lx\n", amap->mask);
	fprintf(FPRT2, "\toffset=%d\n", amap->offset);
	fprintf(FPRT2, "\tshift=%d\n", amap->shift);
	fprintf(FPRT2, "\tufd=%d\n", amap->ufd);
	if (!(sgp = amap->segdatap)) {
		fprintf(FPRT2, "\n\tno segment data\n\n");
		return(0);
	}
	for (gcnt = 0; gcnt < amap->noseg; sgp++, gcnt++) {
		fprintf(FPRT2, "\n\tSegment #%d:\n", gcnt);
		fprintf(FPRT2, "\t\tsegtype=0x%x\n", sgp->segtype);
		fprintf(FPRT2, "\t\tscncnt=%d\n", sgp->scncnt);
		fprintf(FPRT2, "\t\tscnptr=0x%lx\n", sgp->scnptr);
		fprintf(FPRT2, "\t\tcoreoff=0x%lx\n", sgp->coreoff);
		fprintf(FPRT2, "\t\tsegnum=%d\n", sgp->segnum);
		fprintf(FPRT2, "\t\tsegvsize=0x%lx\n", sgp->segvsize);
		fprintf(FPRT2, "\t\tsegvaddr=0x%lx\n", sgp->segvaddr);
		fprintf(FPRT2, "\n");
		if (!(shp = sgp->scnptr)) continue;
		for (hcnt = 0; hcnt < sgp->scncnt; shp++, hcnt++) {
			fprintf(FPRT2, "\t\tsub-section #%d:\n", hcnt);
			fprintf(FPRT2, "\t\t\ts_name=%s\n", shp->s_name);
			fprintf(FPRT2, "\t\t\ts_paddr=0x%lx\n", shp->s_paddr);
			fprintf(FPRT2, "\t\t\ts_vaddr=0x%lx\n", shp->s_vaddr);
			fprintf(FPRT2, "\t\t\ts_size=0x%lx\n", shp->s_size);
			fprintf(FPRT2, "\t\t\ts_scnptr=0x%lx\n", shp->s_scnptr);
			fprintf(FPRT2, "\t\t\ts_relptr=0x%lx\n", shp->s_relptr);
			fprintf(FPRT2, "\t\t\ts_lnnoptr=0x%lx\n", shp->s_lnnoptr);
			fprintf(FPRT2, "\t\t\ts_nreloc=%d\n", shp->s_nreloc);
			fprintf(FPRT2, "\t\t\ts_nlnno=%d\n", shp->s_nlnno);
			fprintf(FPRT2, "\t\t\ts_flags=0x%lx\n", shp->s_flags);
		}
	}
	fprintf(FPRT2, "\n");
	return(0);
}
#endif
#endif
