/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	@(#)	1.1	*/
/* accept dest ... - allow lp to accept requests */

#include	"lp.h"

char errmsg[100];

main(argc, argv)
int argc;
char *argv[];
{
	int i;
	int rc_acc = 0;			/* Return code */
	struct qstat q;
	char reason[Q_RSIZE], *trim(), *dest, *strcpy();

	startup(argv[0]);

	if(! ISADMIN)
		fatal(ADMINMSG, 1);

	if(argc == 1) {
		printf("usage: %s dest ...\n", argv[0]);
		exit(1);
	}

	sprintf(reason, "accepting");

	for(i = 1; i < argc; i++) {
		dest = argv[i];
		if(!isdest(dest)) {
			sprintf(errmsg,
			   "destination \"%s\" non-existent", dest);
			fatal(errmsg, 0);
			rc_acc = 1;
		}
		else if(getqdest(&q, dest) == EOF) {
			sprintf(errmsg,
			   "destination \"%s\" has disappeared!", dest);
			fatal(errmsg, 0);
			rc_acc = 1;
		}
		else if(q.q_accept) {
			sprintf(errmsg,
			  "destination \"%s\" was already accepting requests",
			  dest);
			fatal(errmsg, 0);
			rc_acc = 1;
		}
		else {
			q.q_accept = TRUE;
			time(&q.q_date);
			strcpy(q.q_reason, reason);
			putqent(&q);
			printf("destination \"%s\" now accepting requests\n",
			  dest);
		}
		endqent();
	}

	exit(rc_acc);
}

startup(name)
char *name;
{
	int catch(), cleanup();
	extern char * f_name;
	extern int (*f_clean)();

	if(signal(SIGHUP, SIG_IGN) != SIG_IGN)
		signal(SIGHUP, catch);
	if(signal(SIGINT, SIG_IGN) != SIG_IGN)
		signal(SIGINT, catch);
	if(signal(SIGQUIT, SIG_IGN) != SIG_IGN)
		signal(SIGQUIT, catch);
	if(signal(SIGTERM, SIG_IGN) != SIG_IGN)
		signal(SIGTERM, catch);

	f_name = name;
	f_clean = cleanup;
	if(chdir(SPOOL) == -1)
		fatal("spool directory non-existent", 1);
}

/* catch -- catch signals */

catch()
{
	int cleanup();
	signal(SIGHUP, SIG_IGN);
	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	signal(SIGTERM, SIG_IGN);
	cleanup();
	exit(1);
}

cleanup()
{
	endqent();
	tunlock();
}
