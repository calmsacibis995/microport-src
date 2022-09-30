
static char *uportid = "@(#)ddump.c	Microport Rev Id  1.3.2 6/10/86";

/****************************************************************************/
/*
ddump prints out the contents of the requested sector in hex
        Larry Weaver  April 86
*/

/****************************************************************************/
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/param.h>
#include <sys/types.h>
#include "sys/misc.h"
#include "sys/wn.h"

#define FILE path[unit]
char *path[] = {"/dev/rdsk/0s5","/dev/rdsk/1s5"} ;
unsigned int buf[256];
int fd, unit;

main(argc,argv)
int argc; char *argv[];
{
	int i,j, cyl,head,sect;
	if (argc != 5)
	{
		printf("Usage: ddump unit cyl head sector\n");
		exit(1);
	};
	unit = (atoi(argv[1]));
 	cyl = (atoi(argv[2]));
 	head = (atoi(argv[3]));
 	sect = (atoi(argv[4]));
	initialize();
	if (read_sector(fd,buf,cyl,head,sect)) exit(1);
	for (i=0 ; i < 32 ; i++)
	{
		printf("\n%04x  ",i*16);
		for (j=0 ; j < 8 ; j++)
			printf("%04x ",buf[(8*i)+j]);
	};
	printf("\n");
}

/**********************************************************************/
/**********************************************************************/

/*  Initialize unit (drive) */

initialize()
{
	if ((fd = open(FILE, 2)) == -1) {
		printf("Can't open %s, errno=%d\n", FILE, errno);
		exit(errno);
	}
}

/****************************************************************************/
/* read a sector by disk address (cyl,head,sector)     */
read_sector(dev,buff,cyl,head,sector)
dev_t dev;
byte *buff,head,sector;
int cyl;
{

	int i;
	struct i1010iopb *io, iopb;
	io = &iopb;
	io->i_addr = (long) buff;
	io->i_actcylinder = cyl;
	io->i_acthead = head;
	io->i_sector = sector;
	io->i_funct = WD_READ_OP;
	i = ioctl(dev,I1010_RAWIO,io);
	if (i) printf("ioctl failed, error no: %d \n",errno);	
	return(i);
}
/**************************************************************************/

/****************************************************************************/
