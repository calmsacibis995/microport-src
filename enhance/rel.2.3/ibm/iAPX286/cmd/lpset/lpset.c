#include "sys/param.h"
#include "sys/types.h"
#include "sys/dir.h"
#include "sys/signal.h"
#include "sys/user.h"
#include <errno.h>
#include "sys/tty.h"
#include "sys/lprio.h"
#include <string.h>

#define FILE argv[1]
/* routine to set lp indent, column count , and line count */
main (argc, argv)
int argc;
char *argv[];
{
	struct lprio lps;
	int i ,op, fd;
	char *callname;

	op = LPRGET; /* default */
	callname = argv[0] + strlen(argv[0]) - 5;
	printf("%s\n",callname);
	if (strcmp("lpset",callname ) == 0)
	{
		if (!( argc >= 5)) giveusage();
		lps.ind = atoi(argv[2]);
		lps.col = atoi(argv[3]);
		lps.line = atoi(argv[4]);
		if (argc == 6) lps.mode = 1;
		else lps.mode = 0;
		op = LPRSET;
	};

	if ((fd = open(FILE, 2)) == -1) 
	{
		printf( "Can't open %s, errno=%d\n", FILE, errno);
		exit(errno);
	};

	i = ioctl(fd,op,&lps);
	if (i) printf("ioctl failed, error no: %d \n",errno);	

	else
	{
		printf("\nIndentation: %d   Columns: %d  Lines: %d",
					lps.ind,lps.col,lps.line);
		printf("   Transparency: %s \n", lps.mode ? "on" : "off");
	};
	exit(i);
}
/**********************************************************************/
giveusage()
{
	printf("Usage is lpset device indent columns lines [transparency] \n");
	exit(1);
}

