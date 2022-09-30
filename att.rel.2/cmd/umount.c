/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	@(#)	1.4	*/
#include <stdio.h>
#include <sys/types.h>
#include <mnttab.h>
#include <sys/errno.h>
#define EQ(a,b) (!strcmp(a,b))

extern int errno;
struct mnttab mtab[NMOUNT], *mp;

main(argc, argv)
char **argv;
{
	extern char *getcwd();
	register char *np;
	char buf[128];	/* for constructing filename */
	int	rec,
		fixonly=0;	/* flag to fix mount table */

	sync();
	rec = open("/etc/mnttab",0);
	if(rec < 0) {
		fprintf(stderr, "umount: cannot open /etc/mnttab\n");
		exit(2);
	}
	read(rec, mtab, sizeof mtab);
	if(argc != 2) {
		fprintf(stderr, "usage: umount device\n");
		exit(2);
	}
	if (umount(argv[1]) < 0) {
		rpterr(argv[1]);
		if(errno == EINVAL) {	/* device not mounted */
			fixonly = 1;	/* set flag to fix mount table */
		}
		else
			exit(2);
	}
	np = argv[1];
	while(*np++);
	np--;
	while(*--np == '/') *np = '\0'; /* remove trailing /'s */

	/* most times things are mounted on special files in /dev/.
	   If so, we can strip "/dev/" off the device name in the 
	   mount table.

		This section of code was taken from 'mount.c' and
		replaces the filename generation code that was here,
		originally. The old code did not use the same conventions
		or make the same assumptions about what was in 'mnttab'.
		Now there is consistency.
	*/
	if(strncmp(argv[1],"/dev/",5) == 0) np = &argv[1][5];

		/*  Otherwise use the complete file string */
	else {
		/* if the full path name was passed in, we have it made */
		if(*argv[1] == '/') np = argv[1];

			/* Otherwise, generate the full path name */
		else{
			int x;	/* temporary scratch pad */

			if((np = getcwd(buf,128)) == NULL)
				fprintf(stderr,
				   "umount: cannot get current directory\n");
			if(np[strlen(np)-1] != '/')
				strncat(np,"/",126);
			strncat(np,argv[1],127-strlen(argv[1]));
			if( (x=strspn(np,"/")-1) > 0 )
				np += x;
			if(strncmp(np,"/dev/",5) == 0) np += 5;
			if(strlen(np) > 31){
				np[31] = '\0';
			}
		}
		
	}

	for (argv[1] = np, mp = mtab; mp < &mtab[NMOUNT]; mp++) {
		if(EQ(argv[1], mp->mt_dev)) {
			if(fixonly) {
			  fprintf(stderr,"%s: deleting %s%s from mount table.\n",
				argv[0], (*argv[1] != '/' &&
					strncmp(argv[1],"/dev/",4))?
					"/dev/" : "", argv[1]);
			}
			mp->mt_dev[0] = '\0';
			time(&mp->mt_time);
			mp = &mtab[NMOUNT];
			while ((--mp)->mt_dev[0] == '\0');
			rec = creat("/etc/mnttab", 0644);
			write(rec, mtab, (mp-mtab+1)*sizeof mtab[0]);
			exit(0);
		}
	}
	if(!fixonly)
		fprintf(stderr, "warning: %s%s was not in mount table\n",
			(*argv[1] != '/' && strncmp(argv[1],"/dev/",4)) ?
				"/dev/" : "", argv[1]);
	exit(2);
}
rpterr(s)
char *s;
{
	switch(errno){
	case EPERM:
		fprintf(stderr,"umount: not super user\n");
		break;
	case ENXIO:
		fprintf(stderr,"umount: %s no device\n",s);
		break;
	case ENOENT:
		fprintf(stderr,"umount: %s no such file or directory\n",s);
		break;
	case EINVAL:
		fprintf(stderr,"umount: %s not mounted\n",s);
		break;
	case EBUSY:
		fprintf(stderr,"umount: %s busy\n",s);
		break;
	case ENOTBLK:
		fprintf(stderr,"umount: %s block device required\n",s);
		break;
	default:
		fprintf(stderr, "umount: errno %d, cannot unmount %s\n",
			errno, s);
	}
}
