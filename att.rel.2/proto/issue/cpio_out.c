/*	@(#)cpio_out.c	1.2	85/06/05			*/
/*
*	this routine is called by issue to cpio the files out to the floppy
*	this routine is so issue need not be run by superuser.
*	routine takes 1 option -h  if set writes to section 0.
*
*	modified to use CPIO to handle files of form /usr/.adm/...
*
*/
main (argc , argv)
int argc;
char *argv[];
{
	int i, parent, child;
	char *f,*c;
	char cmd[100];

	setgid(0);
	setuid(0);				/* become super_user */

	f = argv[1];

	c=cmd;

	if ( f[0] == '-' && f[1] == 'h' && f[2] == '\0' )
	{
		f = argv[2];
		sprintf(c,"ls %s | cpio -ov >/dev/rdsk/0s24",f);
	}
	else sprintf(c,"cat %s | CPIO -ov >/dev/rdsk/0s25",f);

	system(c);
}

