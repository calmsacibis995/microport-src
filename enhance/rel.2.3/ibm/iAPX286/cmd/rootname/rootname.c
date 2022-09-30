#include <stdio.h>
#include <string.h>

main (argc, argv)
    int argc;
    char **argv;
{
    int len;
    char *filep, *extp;
    char buff [128];

    if (argc != 2) {
	fprintf (stderr, "Usage: %s filename\n", argv [0]);
	exit (1);
    }

    if (filep = strrchr (argv [1], '/'))
	filep++;
    else
	filep = argv [1];

    if (! (extp = strrchr (filep, '.')))
	for (extp = filep; *extp; extp++)
	    ;
    len = extp - filep;
    strncpy (buff, filep, len);
    buff [len] = '\0';
    printf ("%s\n", buff);
    exit (0);
}
