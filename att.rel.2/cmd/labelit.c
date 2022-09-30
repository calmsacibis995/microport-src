/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	@(#)	1.2	*/
/*
 *  Modification history:
 *
 *  M000	uport!chuckp	Fri Aug 28 14:06:57 PDT 1987
 *	Modified to write labels to unlabeled floppies for volcopy
 *	and finc.
 */
#include <stdio.h>
#include <sys/param.h>
#ifndef	RT
#include <signal.h>
#include <sys/types.h>
#endif
#include <sys/sysmacros.h>
#include <sys/filsys.h>
#define DEV 1
#define FSNAME 2
#define VOLUME 3
 /* write fsname, volume # on disk superblock */
struct {
	char fill1[SUPERBOFF];
	union {
		char fill2[SUPERBOFF];
		struct filsys fs;
	} f;
} super;
#ifdef RT
#define IFTAPE(s) (equal(s,"/dev/mt",7)||equal(s,"mt",2))
#else
#define IFTAPE(s) (equal(s,"/dev/rmt",8)||equal(s,"rmt",3)||equal(s,"/dev/rtp",8)||equal(s,"rtp",3))
#endif
#ifdef iAPX286
#include <sys/wn.h>
#include <sys/format.h>
#define	BCKFLPSIZE	2400	/* M000: # blocks on high density floppy */
#endif
int	tapelab;	/* != 0 implies that it is to be treated as a tape */

struct {
	char	t_magic[8];
	char	t_volume[6];
	char	t_reels,
		t_reel;
	long	t_time,
		t_length,
		t_dens;
	char	t_fill[484];
} Tape_hdr;

sigalrm()
{
	signal(SIGALRM, sigalrm);
}

main(argc, argv)
char **argv;
{
int fsi, fso;
long curtime;
int i;

	signal(SIGALRM, sigalrm);

#ifdef RT
	setio(-1,1);	/* use physical io */
#endif

	if(argc!=4 && argc!=2 && argc!=5)  {
showusage:
#ifdef	RT
		fprintf(stderr,"Usage: labelit /dev/??? [fsname volume [-n]]\n");
#else
		fprintf(stderr,"Usage: labelit /dev/r??? [fsname volume [-n]]\n");
#endif
		exit(2);
	}
	tapelab = IFTAPE(argv[DEV]);

#ifdef iAPX286
	/* Check to see if we have a floppy to be treated as a tape. */
	if((fsi = open(argv[DEV], 0)) > 0 && ioctl(fsi, I215_FLOPPY, 0) == BCKFLPSIZE)
		tapelab = 1;
#endif
	if(argc==5) {
		if(strcmp(argv[4], "-n")!=0)
			goto showusage;
		if(!tapelab) {
#ifndef iAPX286
			fprintf(stderr, "labelit: `-n' option for tape only\n");
#else
			fprintf(stderr, "labelit: `-n' option for floppy or tape only\n");
#endif
			exit(2);
		}
		printf("Skipping label check!\n");
		goto do_it;
	}

	if((fsi = open(argv[DEV],0)) < 1) {
		fprintf(stderr, "labelit: cannot open device\n");
		exit(2);
	}

	if(tapelab) {
		alarm(5);
		read(fsi, &Tape_hdr, sizeof(Tape_hdr));
		alarm(0);
		if(!(equal(Tape_hdr.t_magic, "Volcopy", 7)||
		    equal(Tape_hdr.t_magic,"Finc",4))) {
#if iAPX286
			fprintf(stderr, "labelit: floppy not labelled!\n");
#else
			fprintf(stderr, "labelit: tape not labelled!\n");
#endif
			exit(2);
		}
#if iAPX286
		printf("%s floppy volume: %s, floppy %d of %d floppies\n",
#else
		printf("%s tape volume: %s, reel %d of %d reels\n",
#endif
			Tape_hdr.t_magic, Tape_hdr.t_volume, Tape_hdr.t_reel, Tape_hdr.t_reels);
		printf("Written: %s", ctime(&Tape_hdr.t_time));
		if(argc==2 && Tape_hdr.t_reel>1)
			exit(0);
	}
	if((i=read(fsi, &super, sizeof(super))) != sizeof(super))  {
		fprintf(stderr, "labelit: cannot read superblock\n");
		exit(2);
	}

#define	S	super.f.fs
	printf("Current fsname: %.6s, Current volname: %.6s\n",
		S.s_fname, S.s_fpack);

	if (S.s_magic == FsMAGIC && S.s_type == Fs2b) {
		printf("Blocks: %ld, Inodes: %d\nFS Units: 1Kb, ",
			S.s_fsize * 2, (S.s_isize - 2) * 16);
	/* M000: start */
		printf("Date last mounted: %s", ctime(&S.s_time));
	}
#ifdef	preM000
	else
		printf("Blocks: %ld, Inodes: %d\nFS Units: 512b, ",
			S.s_fsize, (S.s_isize - 2) * 8);
#endif	/* preM000 */
	/* M000: end */

	if(argc==2)
		exit(0);
do_it:
	printf("NEW fsname = %.6s, NEW volname = %.6s -- DEL if wrong!!\n",
		argv[FSNAME], argv[VOLUME]);
	sleep(10);
	sprintf(super.f.fs.s_fname, "%.6s", argv[FSNAME]);
	sprintf(super.f.fs.s_fpack, "%.6s", argv[VOLUME]);

	close(fsi);
	fso = open(argv[DEV],1);
	if(tapelab) {
		strcpy(Tape_hdr.t_magic, "Volcopy");
		sprintf(Tape_hdr.t_volume, "%.6s", argv[VOLUME]);
							/* M000: start */
		time(&Tape_hdr.t_time);
		if(write(fso, &Tape_hdr, sizeof(Tape_hdr)) < 0) {
			fprintf(stderr, "labelit cannot write label\n");
			exit(2);
		}					/* M000: end */
	}
	if(write(fso, &super, sizeof(super)) < 0) {
		fprintf(stderr, "labelit cannot write label\n");
		exit(2);
	}
	exit(0);
}
equal(s1, s2, ct)
char *s1, *s2;
int ct;
{
	register i;

	for(i=0; i<ct; ++i) {
		if(*s1 == *s2) {;
			if(*s1 == '\0') return(1);
			s1++; s2++;
			continue;
		} else return(0);
	}
	return(1);
}
