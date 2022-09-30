/*      Copyright (c) 1985 AT&T */
/*        All Rights Reserved   */

/*      THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T     */
/*      The copyright notice above does not evidence any        */
/*      actual or intended publication of such source code.     */

/* @(#)ib.c	1.4 - 85/08/09 */

/*
**	Install Bootstrap 
**
**	device1 device2 [ boot_file ]
*/

#include <stdio.h>
#include <fcntl.h>
#include <aouthdr.h>
#include <filehdr.h>
#include <scnhdr.h>
#include <sys/types.h>
#include <sys/wn.h>

/*
**	structure definitions and data for boot label 
*/

struct drtab {
	int	drtab_number_of_cylinders ;
	char	drtab_number_of_fixed_heads ;
	char	drtab_number_of_removable_heads ;
	char	drtab_number_of_sectors_per_track ;

	/*
	 *	following should be an int but alignment problems
	 *	because we start each int on a 16-bit boundary
	 */

	char	drtab_low_sector_size ;
	char	drtab_high_sector_size ;
	char	drtab_number_of_alternate ;
	int	drtab_sectors_per_cylinder ;
	int	drtab_sectors_per_block ;
	int	drtab_sector_size ;
	int	drtab_partition_table ;
} ;

struct label {
	char	lab_name[10] ;
	char	lab_flags ;
	char	lab_driver ;
	int	lab_granularity ;
	long	lab_size ;
	int	lab_node0 ;
	long	lab_node1 ;
	int	lab_node2 ;
	int	lab_node3 ;
	int	lab_secsz ;
	int	lab_interleave ;
	int	lab_skew ;
	int	lab_osid ;
	char	lab_sysname[12] ;
	struct drtab	lab_drtab ;
	char	lab_pad[66] ;
} label ;

extern int errno ;
int wn   = 0 ;
char *dev1 = "" ,
	*dev2 = "" ,
	*boot = "/etc/boot" ;

char ourlabel[10] ;

#if FIRST1024
char buff[0x1c00] ;
int first1024 = 0 ;
#else
char buff[0xd00] ;
#endif

main(argc,argv)
char **argv ;
{
	int i ;
	int dev ;

	if((argc<3) || (argc>4))
		giveusage() ;

	dev1 = argv[1] ;
	dev2 = argv[2] ;

	if( argc==4 )
		boot = argv[3] ;

	/*
	 *	get the ioctl information about the device
	 *	given in the options.
	 */

	if ( (dev = open ( dev2 , O_RDWR)) == -1 )
	{
		perror("ib: opening device2");
		exit();
	}

	if ( ioctl(dev,I215_CHAR,ourlabel) == -1 )
	{
		perror("ib: performing ioctl to get characteristics");
		exit() ;
	}

	close(dev) ;

	/*
	 *	determine if floppy or not
	 */

	wn = ( ourlabel[3] == 0 ) ;	/* number of removable heads */

	/*
	 *	frig - because last 4 bytes aren't word aligned
	 *	(why couldn't the kernel get it right?)
	 */

	ourlabel[5] = ourlabel[6] ;
	ourlabel[6] = ourlabel[7] ;
	ourlabel[7] = ourlabel[8] ;

	ourlabel[8] = ( wn ? 0x10 : 0x12 ) ;
	ourlabel[9] = 0 ;

#if DEBUG
	fprintf(stderr," wn=%d  dev1=%s dev2=%s boot=%s\n",wn,dev1,dev2,boot);
#endif

	if(wn)
		instalwini() ;
	else
		instalflop() ;

#if DEBUG
	fprintf(stderr,"Install complete\n");
#endif
}

giveusage()
{
	fprintf(stderr,"Usage: ib device1 device2 [ boot_file ]\n");
	fprintf(stderr,"Don't Panic\n");
	exit() ;
}

instalwini()
{
	int i ;
#if DEBUG
	fprintf(stderr,"INSTALWINI:\n");
#endif
	fprintf(stderr,"Installing Winchester boot '%s' on device '%s'\n",
			boot,dev1);

	makehdlabel() ;
	makeboot() ;

#if FIRST1024
	if(first1024) {
	/*
	 *	move the boot so that it
	 *	is the frirst 512 bytes of each
	 *	1024 - fun ain't it
	 */

	for(i=6;i>=0;i--)
	{
		/*
		 *	zero fill dead part
		 */

		memset ( &buff[ (i*0x400) + 0x200 ] , 0 , 0x200 ) ;

		/*
		 *	copy 512 bytes of the boot
		 */

		memcpy ( &buff[ (i*0x400) ] , &buff[ (i*0x200) ] , 0x200 ) ;
	}

	/*
	 *	now write it all out
	 */

	dowrite(dev1,0,0,0x1c00) ;
	} else
#endif
	dowrite(dev1,0,0,0xd00) ;
}

instalflop()
{
#if DEBUG
	fprintf(stderr,"INSTALFLOP\n");
#endif
	if(dev2[0] == '\0')
		giveusage();
	fprintf(stderr,"Installing floppy boot '%s' on devices '%s' and '%s'\n",
		boot,dev1,dev2);

	makefllabel() ;
	makeboot() ;

	dowrite(dev1,0,0,0x800);
#if SPT8
	dowrite(dev2,4096,0x800,0x4ff);
#else
	dowrite(dev2,4608,0x800,0x4ff);
#endif
}

makeboot()
{
	FILE *bootin ;
	int result ;
	long startat ;
	FILHDR  filhdr ;

#if DEBUG
	fprintf(stderr,"MAKEBOOT:\n");
#endif

	if( (bootin=fopen(boot,"r")) == NULL )
	{
		fprintf(stderr,"Failure to open bootfile '%s'\n",boot);
		exit() ;
	}

	/*
	 *	read file header 
	 */

	if(fread(&filhdr,1,FILHSZ,bootin) != FILHSZ)
	{
		fprintf(stderr,"Failure to read file header of '%s'\n",boot);
		exit() ;
	}

	/*
	 *	check magic number
	 */

	if( filhdr.f_magic != I286SMAGIC )
	{
		fprintf(stderr,"%s is not an COFF file\n",boot);
		exit() ;
	}

	/*
	 *	read in first part of boot code
	 */

	startat = sizeof(struct filehdr)
		+ sizeof(struct aouthdr)
		+ filhdr.f_nscns * SCNHSZ;

#if DEBUG
	fprintf(stderr,"seeking to %lx\n",startat);
#endif

	if ( fseek ( bootin , startat , 0 ) != 0 )
	{
		fprintf(stderr,"Failure to seek to boot code of '%s'\n",boot);
		exit() ;
	}

	if ( fread ( buff , 1 , 0x180 , bootin ) != 0x180 )
	{
		fprintf(stderr,"Failure to read boot code of '%s'\n",boot);
		exit() ;
	}
	
	/*
	 *	insert label into buffer
	 */

	memcpy ( &buff[ 0x180 ] , &label , sizeof( struct label ) ) ;

	/*
	 *	insert our label into buffer
	 */

	memcpy ( &buff[ 0x2 ] , ourlabel , sizeof( ourlabel ) ) ;

	/*
	 *	now read rest of boot code 
	 *	(assume read will get as much as we need )
	 */

	fread( &buff[ 0x200 ] , 1 , 0xb00 , bootin ) ;

	/*
	 *	close the boot file
	 */

	fclose ( bootin ) ;
}

makefllabel()
{
#if DEBUG
	fprintf(stderr,"makefllabel\n");
#endif
	strncpy ( label.lab_name , "unix286flp" , 10 ) ;
	label.lab_flags	= 0xf ;
	label.lab_driver	= 0x5 ;
	label.lab_granularity	= 0x200 ;
	label.lab_size	= 0x5L ;
	label.lab_node0	= 0x0 ;
	label.lab_node1	= 0x0L ;
	label.lab_node2	= 0x0 ;
	label.lab_node3	= 0x0 ;
	label.lab_secsz	= 0x200 ;
	label.lab_interleave	= 0x0 ;
	label.lab_skew	= 0x0 ;
	label.lab_osid	= 0x30 ;
	strncpy(label.lab_sysname , "Unix 286    " , 12) ; 

	/*
	 *	driver table
	 */

	label.lab_drtab.drtab_number_of_cylinders = 0x28 ;
	label.lab_drtab.drtab_number_of_fixed_heads = 0 ;
	label.lab_drtab.drtab_number_of_removable_heads = 0x2 ;
#if SPT8
	label.lab_drtab.drtab_number_of_sectors_per_track = 0x08 ;
#else
	label.lab_drtab.drtab_number_of_sectors_per_track = 0x09 ;
#endif

	/*
	 *	following should be an int but alignment problems
	 *	because we start each int on a 16-bit boundary
	 */

	label.lab_drtab.drtab_low_sector_size = 0x0 ;
	label.lab_drtab.drtab_high_sector_size = 0x10 ;
	label.lab_drtab.drtab_number_of_alternate = 0x01 ;
	label.lab_drtab.drtab_sectors_per_cylinder = 0x0 ;
	label.lab_drtab.drtab_sectors_per_block = 0x4788 ;
	label.lab_drtab.drtab_sector_size = 0x0 ;
	label.lab_drtab.drtab_partition_table = 0x0 ;
}

makehdlabel()
{
#if DEBUG
	fprintf(stderr,"makehdlabel\n");
#endif
	strncpy ( label.lab_name , "unix286hrd" , 10 ) ;
	label.lab_flags	= 0x18 ;
	label.lab_driver	= 0x5 ;
	label.lab_granularity	= 0x400 ;
	label.lab_size	= 0x800000a6 ;
	label.lab_node0	= 0x0 ;
	label.lab_node1	= 0x0L ;
	label.lab_node2	= 0x0 ;
	label.lab_node3	= 0x0 ;
	label.lab_secsz	= 0x400 ;
	label.lab_interleave	= 0x0 ;
	label.lab_skew	= 0x0 ;
	label.lab_osid	= 0x30 ;
	strncpy(label.lab_sysname , "Unix 286    " , 12) ; 

	/*
	 *	driver table
	 */

	label.lab_drtab.drtab_number_of_cylinders = 306 ; /* was 0x32 */
	label.lab_drtab.drtab_number_of_fixed_heads = 6 ; /* was 1 */
	label.lab_drtab.drtab_number_of_removable_heads = 0 ; /* was 4 */
	label.lab_drtab.drtab_number_of_sectors_per_track = 0x08 ;

	/*
	 *	following should be an int but alignment problems
	 *	because we start each int on a 16-bit boundary
	 */

	label.lab_drtab.drtab_low_sector_size = 0x0 ;
	label.lab_drtab.drtab_high_sector_size = 0x04 ;
	label.lab_drtab.drtab_number_of_alternate = 0x0a ;
	label.lab_drtab.drtab_sectors_per_cylinder = 0x0 ;
	label.lab_drtab.drtab_sectors_per_block = 0x47d4 ;
	label.lab_drtab.drtab_sector_size = 0x0 ;
	label.lab_drtab.drtab_partition_table = 0x0 ;
}

long lseek() ;

dowrite(where,startdev,startbuff,howmuch)
char *where;
{
	int dev ;
	int result ;
	long lresult ;
#if DEBUG
	fprintf(stderr,"DOWRITE: ( %s , %x , %x , %x )\n",where,startdev,startbuff,howmuch);
#endif

	if( (dev=open(where,O_RDWR)) == -1 )
	{
		fprintf(stderr,"failure to open %s\n",where);
		exit() ;
	}

	if(startdev)
	{
#if DEBUG 
	fprintf(stderr,"seeking to %x\n",startdev);
#endif
		lresult = lseek(dev,(long)startdev,0);
#if DEBUG
	fprintf(stderr,"result of seek %lx\n",lresult);
#endif
	}

	result = write(dev,&buff[startbuff],howmuch);
#if DEBUG
	fprintf(stderr,"result of write %x\n",result);
	fprintf(stderr,"errno is %x\n",errno);
#endif
	result = close(dev) ;
#if DEBUG
	fprintf(stderr,"result of close %x\n",result);
	fprintf(stderr,"errno is %x\n",errno);
	fprintf(stderr,"about to do forced sync\n");
#endif
	sync() ;
}

