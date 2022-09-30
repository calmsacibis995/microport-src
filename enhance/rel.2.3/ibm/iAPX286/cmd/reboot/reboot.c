static char *uportid = "@(#)reboot.c	Microport Rev Id  xxxxxxxxxxxxx";

/*
 *
 *	To compile: cc -O reboot.c -o reboot 
 *	First release:	1/18/87 by Dwight H. Leu
 *		(C) Microport Systems, Inc. All Rights Reserved.
 */

#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include "sys/io_op.h"

char *myname;
int n, dev;

main(ac, av)
int	ac;
char	**av;
{
	myname = av[0];
	if (n = ac - 1)
		usage();
	if ((getuid() != 0) || (-1 == (dev = open("/dev/kmem", O_RDWR))))
		usage();
	if (ioctl(dev, IOCIOP_REBOOT, 0) == -1)
		printf("nope\n");

	close(dev);
	exit(0);
}

char *usagemsg[] = {
"Usage: reboot - no options required. You must be superuser!",
0
};

usage() {
	int	i;

	for (i=0; usagemsg[i]; i++) 
		printf(usagemsg[i], myname);
	exit(1);
}
