/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#include <stdio.h>
main()  {
	FILE *infp;
	FILE *outfp;
	FILE *psfp;
	long i=0L;
	unsigned char c;
	int count = 0;
	if ((infp=fopen("/dev/mem","r")) == NULL)  {
		printf("cannot open /dev/mem\n");
		return;
	}
	while (1)  {
		fseek(infp,0,0L);
		if ((psfp = fopen("/dev/prtscr","r")) == NULL)  {
			printf("cannot open /dev/prtscr\n");
			break;
		}

		if ((outfp=fopen("/usr/spool/lp/.prtscr.out","w"))==NULL)  {
			printf("cannot open output file .prtscr.out\n");
			break;
		}

		i = (long)0xb8000;
		count = 0;
		while (count++ < 0x0f00)  {
			fseek(infp,i++,0L);
			c = getc(infp);
			putc(c,outfp);
			i++,count++;
		}

		/* this is where the print file is qeued up */
		system("lp /usr/spool/lp/.prtscr.out");

		fclose(psfp);
		fclose(outfp);
	}
}
