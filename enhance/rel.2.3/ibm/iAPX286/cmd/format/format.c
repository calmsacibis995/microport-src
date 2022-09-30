static char *uportid = "@(#)format.c	Microport Rev Id  1.3.8 12/30/86";
/*      Copyright (c) 1985 AT&T */
/*        All Rights Reserved   */

/*      THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T     */
/*      The copyright notice above does not evidence any        */
/*      actual or intended publication of such source code.     */

/* Modification History:
 * M000:	uport!dwight
 *	Added some minor changes to handle supporting the floppy.
 * M001: 	Lance Norskog
 *	Added support for more than 8 head disks.
 * M002 uport!bernie Mon May 25 17:00:09 PDT 1987
 *  Set successful exit status to zero.
 */

/* iAPX286 @(#)format.c 1.6 */
static char sccsid[] = "@(#)format.c    1.6" ;
#include <stdio.h>
#include <sys/types.h>
#include <sys/format.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/wn.h>

unsigned long first = 0 ;
unsigned long last = -1 ;
unsigned int interleave = 3 ;
unsigned int tpc,spt,precomp, newboot = 0;
char *device = NULL ;

main(argc,argv)
char **argv ;
{
        unsigned long i ;

        for(i=1;i<argc;i++)
        if(argv[i][0] == '-')
                switch( argv[i][1] ) {

                case 'f':
                        if( i++ <argc )
                                first = atol( argv[i] ) ;
                        else
                                giveusage() ;
                        break ;

                case 'l':
                        if( i++ <argc )
                                last = atol( argv[i] ) ;
                        else
                                giveusage() ;
                        break ;

                case 'i':
                        if( i++ <argc )
                                interleave = atoi( argv[i] ) ;
                        else
                                giveusage() ;
                        break ;

                case 's':
                        if( i <argc )
				newboot++;
                        else
                                giveusage() ;
                        break ;

                default:
                        giveusage() ;
                        break ;

                }
        else
        {
                if ( device == NULL )
                        device = argv[i] ;
                else
                        giveusage() ;
        }

        if ( device == NULL )
                giveusage() ;
	if (initialize())
	{
		printf ("\nformat:invalid specification\n");
		exit(1);
	};
        doformat() ;

	if (newboot) init_mbblk(); /* write boot block if -s */
  
	exit(0);   /* M002 */
}

giveusage()
{
        fprintf(stderr,"Usage: format [-s] [-f first] [-l last] [-i interleave] device\n");
        exit() ;
}

int dev ;

doformat()
{
        unsigned int i,j ;
        struct i215ftk control ;
	char *s;
	int p;

#if DEBUG
fprintf(stderr,"doformat -f %ld -l %ld -i %d %s\n",first,last,interleave,device);
#endif

        if( (dev=open(device,O_RDWR)) == -1)
        {
                perror("format: opening device");
                exit() ;
        }

/* Note: calling ioctl on a block device causes u.u_error to get
 *	set to ENOTTY. This will cause perror to print out the
 *	message "[format] ... Not a Typewriter". This is incredibly dumb.
 *	Soooo, to get around this, we must first perform a dummy ioctl().
 */
	if (ioctl( dev, I215_CTEST, 0) == -1) {
		fprintf(stderr, "format: must use a character device ");
		fprintf(stderr, "(try /dev/rdsk/[device])\n");
		exit();
	}

        if ( last == -1)
        {
                /*
                 *      last wasn't specified - get it for them
                 */
                
                if(ioctl(dev, I215_NTRACK, &last) == -1 ) 
                {
                        perror("format: performing ioctl on device");
                        exit();
                }
#if DEBUG
fprintf(stderr,"last (as returned by ioctl) %ld (0x%lx)\n", last, last);
#endif
                last-- ;
        }

        /*
         *      check for absurd
         */

        if( first > last)
        {
                fprintf(stderr,"format: first > last\n");
                exit();
        }

        control.f_intl = interleave ;
        control.f_skew = 0 ;            /* ignored */
        control.f_type = FORMAT_DATA ;
        control.f_pat[0] = 'D' ;
        control.f_pat[1] = 'e' ;
        control.f_pat[2] = 'a' ;
        control.f_pat[3] = 'D' ;

/* Start M000:
 * There are some slight design inconsistancies here, between formatting
 * the hard disk and formatting the floppy. Basically, the variable "tpc"
 * gets set in the initialize() code, for harddisks. For a floppy, tpc will 
 * be zero at this point. Balancing various considerations, the floppy
 * driver will now ignore the f_track scheme, and pick up the # of tracks
 * from the minor device entry.
 * End M000:
 */

	if (tpc > 0)
		raw_io(dev, (char *) 0, precomp, tpc, spt, WD_SET_PARM_OP);
		raw_io(dev, (char *) 0, 0, 0, 0, WD_RECAL_OP);
	for (i=first; i <= last;i++) {
        	control.f_cyl = i;
/* Start M000 */
		if (tpc == 0) {			/* floppy */
			printf ("Formatting Cylinder %d\r",i);
			fflush(stdout);
	        	if( ioctl(dev, I215_IOC_FMT, &control) == -1) 
                   		perror("format: performing ioctl");
		} else {
			for ( j = 0 ; j < ((tpc > 8) ? 8 : tpc) ;j++) {
			       printf ("Formatting Cylinder %d Track %d\r",i,j);
/* End M000 */
				fflush(stdout);
				control.f_track = j ;
	        		if( ioctl(dev, I215_IOC_FMT, &control) == -1) 
                   			perror("format: performing ioctl");
          		}
		}
	}
	if (tpc > 8) {
	    raw_io(dev, (char *) 0, 0, 0, 0, WD_RECAL_OP);
	    for (i=first; i <= last;i++) {
		    control.f_cyl = i;
/* Start M000 */
			for ( j = 8 ; j <tpc ;j++) {
			       printf ("Formatting Cylinder %d Track %d\r",i,j);
/* End M000 */
				fflush(stdout);
				control.f_track = j ;
	        		if( ioctl(dev, I215_IOC_FMT, &control) == -1) 
                   			perror("format: performing ioctl");
          		}
		}
	}
        printf("Formatted cylinders %ld to %ld of device %s\n",first,last,device);

end:
        close(dev);
}
