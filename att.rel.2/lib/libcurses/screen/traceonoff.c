/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	@(#)	1.1	*/
#include "curses.ext"

traceon()
{
#ifdef DEBUG
	if (outf == NULL) {
		outf = fopen("trace", "a");
		if (outf == NULL) {
			perror("trace");
			exit(-1);
		}
		fprintf(outf, "trace turned on\n");
	}
#endif
}

traceoff()
{
#ifdef DEBUG
	if (outf != NULL) {
		fprintf(outf, "trace turned off\n");
		fclose(outf);
		outf = NULL;
	}
#endif
}
