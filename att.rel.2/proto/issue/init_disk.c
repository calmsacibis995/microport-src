/*		@(#)init_disk.c	1.3	85/06/20		*/
/*	This utility is called from issue to initialise a floppy disk,
*	this routine exists so issue need not be run by su
*
*/
main (argc , argv)
int argc;
char *argv;
{
	int parent, child;

	setgid(0);
	setuid(0);				/* become super_user */

	parent = getpid();
	child = fork();
	if ( parent != getpid() )
	{									/* child process */
		execl("/etc/format","/etc/format","/dev/rdsk/0s24",0);
		printf("format failed\n");
		exit(-1);
	}
	while ( wait(0) != child ); 		/* wait for child to finish */
}

