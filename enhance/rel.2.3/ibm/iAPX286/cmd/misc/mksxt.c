/* %W% */

#include <stdio.h>
#define	conv(x)	(((x / inc) * 10) + (x % inc))

main (argc, argv)
    int argc;
    char **argv;
{
    int state = 0, verbose = 0, debug = 0, delete = 0, i;
    int major = 15, minor = 0, count = 128, inc = 10;
    long strtol();
    char dir[128], ttyname[128], nodename [128], *arg;
    int c, errflg = 0;
    extern int optind;

    dir [0] = '\0';
    while (optind < argc) {
	if ((c = getopt (argc, argv, "duv")) != EOF)
	    switch (c) {
		case 'd': debug++;	continue;
		case 'u': delete++;	continue;	/* unlink */
		case 'v': verbose++;	continue;
		default:  usage (*argv);
	    }
	arg = argv [optind++];
	switch (state++) {
	    case 0: strcpy (dir, arg);				continue;
	    case 1: major = strtol (arg, (char **) 0, 0);	continue;
	    case 2: minor = strtol (arg, (char **) 0, 0);	continue;
	    case 3: count = strtol (arg, (char **) 0, 0);	continue;
	    case 4: inc   = strtol (arg, (char **) 0, 0);	continue;
	    default: usage (*argv);
	}
    }
    if (! state)
	usage (*argv);

    for (i = minor; i < (minor + count); i++) {
	sprintf (nodename, "%s/%03d", dir, conv (i));
	sprintf (ttyname, "%s%03d", dir, conv (i));
	if (verbose) {
	    printf ("mknod %s c %d %d\n", nodename, major, i);
	    printf ("link %s %s\n", nodename, ttyname);
	}
	if (!debug) {
	    if (delete) {
		unlink (nodename);
		unlink (ttyname);
	    }
	    if (mknod (nodename, 0020666, (major << 8) + i) == -1)
		{ perror ("mknod"); exit (2); }
	    if (link (nodename, ttyname) == -1)
		{ perror ("link"); exit (2); }
	}
    }
}

usage (str)
    char *str;
{
    fprintf (stderr, "%s [-dv] name major minor count chans\n", str);
    exit (1);
}
