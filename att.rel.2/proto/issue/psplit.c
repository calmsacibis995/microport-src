/*		@(#)psplit.c	1.2 85/06/20		*/
/***********************************************************************
*
* psplit utility
*
*
* This utility takes as input the list of files in unix 
* of the form <file><product set><size>. This utility produces
* a file .issueflop which contains 
* <product set><disks in product set><disk index 00 - 40 ish>,
* and 40 ish files d00 to d40 which each contain a list of files
* which will fit on a floppy disk.
*
***********************************************************************/

#include <stdio.h>
#include <string.h>

#define d "d/d"
#define issue ".issueflop"

#define room_for(x)  ( (disk_used + x) <= 325000 )

char disk[6];					/* name of file for disk ie d00 */

char fn[80];					/* file name of a unix file */
char ps[20];					/* product set of unix file */
long sz;						/* size of unix file        */

int index = 0;					/* no of first disk for a ps   */
int disk_no = 0;				/* disk number of current disk */
long disk_used = 0;				/* space used on disk in bytes */
char current_ps[20];

FILE *f,*i;						/*  .issueflop and dnn         */

void new_disk_file()
/* to close current file with id f,
*  and to increment disk_no and open next file with id f
*/
{
	int ret;

	ret = fclose(f);
	disk_no = disk_no +1;
	if (disk_no > 60)
	{
		fprintf(stderr,"issue: panic - too many disks\n");
		exit(3);
	}
	if ( disk_no < 10 ) disk[4] = '0' + disk_no;
	else 
	{
		disk[3] = '0' + disk_no/10;
		disk[4] = '0' + disk_no%10;
	}
	if ( (f=fopen(disk,"w")) ==NULL )
	{
		fprintf(stderr,"unable to open disk file %s\n",disk);
		exit(2);
	}
	disk_used = 0;
} /* end of new_disk_file */


main ( argc, argv )
int argc;
char **argv;
{
	int ret;
	int n_read;
	 
	system("if [ -d d ]\nthen /bin/rm -f d/* \nfi"); /* remove files from last time */

	if ( system("if [ ! -d d -a ! -s d ]\nthen mkdir d 2> /dev/null\nfi") != 0 )
	{
		printf("can not create directory d\n");
		exit(-1);
	}

	strcpy(disk,d);
	strcat(disk,"00");
	current_ps[0]=0;

	while ((n_read=fscanf(stdin,"%s\t%s\t%ld",fn,ps,&sz)) != EOF)
	{
		if (n_read != 3)
		{
			fprintf(stderr,"possible data loss in source file\n");
			continue;
		}

		if ( strcmp(current_ps,ps) != 0)
		{
			if ( disk_no == 0 && disk_used == 0 )
			{
				f = fopen(disk,"w");
				i = fopen(issue,"w");
			}
			else
			{
				fprintf(i,"%s\t%d\t%d\t%ld\n",current_ps,disk_no-index+1,index,disk_used);
				new_disk_file();
				index = disk_no;
			}
			strcpy(current_ps,ps);
		}
		 
		/* check file will fit on disk, and output file name */

	 	if (room_for(sz))
		{
			fprintf(f,"%s\n",fn);
			disk_used = disk_used + sz;
		}
		else
		{
			new_disk_file();
			fprintf(f,"%s\n",fn);
			disk_used = sz;
		}
	}
	fprintf(i,"%s\t%d\t%d\t%ld\n",current_ps,disk_no-index+1,index,disk_used);
	ret = fclose(i);
	ret = fclose(d);
}
