static char *uportid = "@(#)setcolor.c	Microport Rev Id  1.3.8 10/22/86";

/* 
 * Setcolor.c: sets foreground and background colors on CGA/EGA/PGA
 */

char *darkfore   = "\033[0;3%cm";
char *brightfore = "\033[1;3%cm";
char *darkback   = "\033[0;4%cm";
char *brightback = "\033[5;4%c;3;0m";
char noblink = 0;

#define	MAXCOLOR	9

struct {
	char *	color;
	int	number;
} colrtab[ MAXCOLOR ] = {
	"black", 	0,
	"red",		1,
	"green",	2,
	"yellow",	3,
	"brown",	3,
	"blue",		4,
	"magenta",	5,
	"cyan",		6,
	"white",	7
};

char *myname;

main (n, args)
    int	n;
    char **args;
{
    char *fmt;
    int	i;

    myname = *args++;
    if (--n == 0)
	usage();
    while (n > 0) {
	if (args [0] [0] == '-') {
	    switch (args [0] [1]) {
		case 'f': fmt = darkfore;	break;
		case 'F': fmt = brightfore;	break;
		case 'b': fmt = darkback;	break;
		case 'B': fmt = brightback;	break;
		case 'n': noblink++;		break;
		default: usage ();
	    }
	    args++;
	    if (--n == 0)
		usage ();
	}
	else 
	    fmt = (n > 1) ? darkback : darkfore;

	for (i=0; i<9; i++) 
	    if (!strcmp (args [0], colrtab [i].color)) 
		break;
	if (i == 9) 
	    usage ();

	printf (fmt, '0' + colrtab [i].number);
	args++;
	n--;
    }
    exit (0);
}

usage()
{
    int i;

    printf("Usage: %s [[ -bBfF ] color1 ] [ -bBfF ] color2\n", myname);
    printf("Where -b denotes a dark background color,\n");
    printf("      -B a bright background color, etc.\n");
    printf("      -f a dark foreground color,\n");
    printf("      -F a bright foreground color. etc.\n");
    printf("If no options, \n");
    printf("   color1 is assumed to be a dark background color, and\n");
    printf("   color2 is assumed to be a dark foreground color.\n");
    printf("Available Colors:\n");
    for (i = 0; i < MAXCOLOR; i++)
	printf ("%s, ", colrtab[i].color);
    putchar ('\n');
    exit(1);
}


