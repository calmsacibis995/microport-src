static char *uportid = "@(#)setkey.c	Microport Rev Id  1.3.8 11/24/86";
/*
 * Setkey program
 * usage: setkey code string
 *
 * Modification History:
 * M000		uport!dwight
 *	Changed the codes table to support function keys f11-f30.
 *	Also modified the usage message; the shift/control-shift
 *	keys are no longer supported.
 * M001		lance
 *	Changed to new scheme which supports more key codes.
 *
 * M002 uport!mike Tue Feb 10 12:31:39 PST 1987
 *	cleaned up code some.
 *	added "alt", keypad keys, tab.
 *	added new print and delete commands.
 *	expanded usefullness of help message.
 *	added capability of inserting control characters in "newstr".
 *
 * M003		uport!rex	2/9/87
 *	Changed f10 entry from CAP_SHIFTED to NONE_SHIFTED.
 * 
 */

#include <stdio.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/kd.h>
#include <sys/kd_info.h>
#include <sys/setkey.h>

#define	MAXSTRLEN	8	/* longest key string */
struct {
	char	input[MAXSTRLEN];
	char	code;
	char	shift;
} maptab[] = {
    /* numerical keypad */
    "center",		0x4c,	NONE_SHIFTED,
    "del",		0x53,	NONE_SHIFTED,
    "down",		0x50,	NONE_SHIFTED,
    "end",		0x4f,	NONE_SHIFTED,
    "home",		0x47,	NONE_SHIFTED,
    "ins",		0x52,	NONE_SHIFTED,
    "left",		0x4b,	NONE_SHIFTED,
    "minus",		0x4a,	NONE_SHIFTED,
    "pgdn",		0x51,	NONE_SHIFTED,
    "pgup",		0x49,	NONE_SHIFTED,
    "plus",		0x4e,	NONE_SHIFTED,
    "prtsc",		0x37,	NONE_SHIFTED,
    "right",		0x4d,	NONE_SHIFTED,
    "tab",		0x0f,	NONE_SHIFTED,	/* allows "backtab" */
    "up",		0x48,	NONE_SHIFTED,
    /* function keypad */
    "f1",		0x3b,	NONE_SHIFTED,
    "f2",		0x3c,	NONE_SHIFTED,
    "f3",		0x3d,	NONE_SHIFTED,
    "f4",		0x3e,	NONE_SHIFTED,
    "f5",		0x3f,	NONE_SHIFTED,
    "f6",		0x40,	NONE_SHIFTED,
    "f7",		0x41,	NONE_SHIFTED,
    "f8",		0x42,	NONE_SHIFTED,
    "f9",		0x43,	NONE_SHIFTED,
    "f10",		0x44,	NONE_SHIFTED,	/* M003 */
    "f11",		0x3b,	CAP_SHIFTED,
    "f12",		0x3c,	CAP_SHIFTED,
    "f13",		0x3d,	CAP_SHIFTED,
    "f14",		0x3e,	CAP_SHIFTED,
    "f15",		0x31,	CAP_SHIFTED,
    "f16",		0x40,	CAP_SHIFTED,
    "f17",		0x41,	CAP_SHIFTED,
    "f18",		0x42,	CAP_SHIFTED,
    "f19",		0x43,	CAP_SHIFTED,
    "f20",		0x44,	CAP_SHIFTED,
    "f21",		0x3b,	CTRL_SHIFTED,
    "f22",		0x3c,	CTRL_SHIFTED,
    "f23",		0x3d,	CTRL_SHIFTED,
    "f24",		0x3e,	CTRL_SHIFTED,
    "f25",		0x32,	CTRL_SHIFTED,
    "f26",		0x40,	CTRL_SHIFTED,
    "f27",		0x41,	CTRL_SHIFTED,
    "f28",		0x42,	CTRL_SHIFTED,
    "f29",		0x43,	CTRL_SHIFTED,
    "f30",		0x44,	CTRL_SHIFTED,
    "",		   	0,	0,
};

main (argc, argv)
    int  argc;
    char **argv;
{
    int i, j, k, m;
    char c, *p;
    struct setkey sk;
    int opt;
    int delete = 0, print = 0;
    extern char *optarg;
    extern int optind;

    while ((opt = getopt (argc, argv, "dp")) != EOF)
	switch (opt) {
	    case 'd':	delete++;	break;
	    case 'p':	print++;	break;
	    case '?':	usage (*argv);
	}

    /*
     * Get the (possible) shift modifiers
     */
    sk.k_shift = NONE_SHIFTED;		/* default shift modifier */
    for (; optind < argc; optind++) {
	if (strcmp (argv [optind], "shift") == 0) {
	    sk.k_shift |= CAP_SHIFTED;
	    continue;
	}
	if ((strcmp (argv [optind], "control") == 0)
	||  (strcmp (argv [optind], "ctrl")    == 0)) {
	    sk.k_shift |= CTRL_SHIFTED;
	    continue;
	}
	if (strcmp (argv [optind], "alt") == 0) {
	    sk.k_shift |= ALT_SHIFTED;
	    continue;
	}
	break;
    }

    /*
     * Decode the key code
     */
    if (optind == argc) {			/* no "key" arg */
	if (delete)
	    delete_keys ();
	if (print)
	    print_keys (*argv);
	usage (*argv);
    }

    for (i = 0; maptab [i].input [0]; i++) {
	if (strcmp (maptab [i].input, argv [optind]) == 0)
	    break;
    }
    if (maptab [i].input [0] == 0) {		/* not found */
	fprintf (stderr, "Key not found!\n");
	usage (*argv);
    }

    sk.k_shift |= maptab [i].shift;
    sk.k_code   = maptab [i].code;

    if (++optind == argc) {			/* no "newstr" */
	if (delete) {
	    sk.k_len = 0;
	    if (ioctl (0, IOCSETKEY, &sk) == -1) {
		perror ("Setkey ioctl");
		exit (1);
	    }
	    exit (0);
	}
	if (ioctl (0, IOCGETKEY, &sk) == -1) {
	    perror ("Getkey ioctl");
	    exit (1);
	}
	if (sk.k_len) {
	    for (i = 0; i < sk.k_len; i++) {
		if ((c = sk.k_data [i]) < ' ') {
		    putchar ('^');
		    c += '@';
		}
		else if (c == '^')
		    putchar ('^');
		putchar (c);
	    }
	}
	exit (0);
    }

    if (strlen (argv [optind]) >= FKEYLEN) {
	fprintf (stderr,
	    "'%s' is too long!\nMaximum length is %d characters.\n",
	    argv [optind], FKEYLEN);
	exit (1);
    }
    for (j = 0, m = 0; c = argv [optind] [j++]; ) {
	if (c == '^') {
	    if (c = argv [optind] [j++]) {
		if (c >= '@' && c != '^')
		    c -= '@';
	    }
	    else {
		j--;	/* null followed '^' */
		c = '^';
	    }
	}
	sk.k_data [m++] = c;
    }
    sk.k_data [m] = '\0';	/* terminate string	*/
    sk.k_len  = m;
    if (ioctl (0, IOCSETKEY, &sk) == -1) {
	perror ("Setkey ioctl");
	exit (1);
    }
    exit (0);
}

static char *helpmsg [] = {
    "Usage: %s [shift-mod] [shift-mod] ... key [newstr]\n",
    "shift-mod: shift ctrl alt\n",
    "Keys:\n",
    "\nNotes:\n",
    "\tIn newstr use ^x for control character 'x' (^^ for '^')\n",
    "\tNo 'newstr' echos current setting of that key\n",
    "\t'setkey -p' prints the current keymap\n",
    "\t'setkey -d' deletes the current keymap\n",
    "\t'setkey -d key' deletes the key setting for key\n",
};

usage (str) 
    char *str;
{
    int i, j;
    char **fmt = helpmsg;

    fprintf (stderr, *fmt++, str);
    fprintf (stderr, *fmt++);
    fprintf (stderr, *fmt++);
    for (i=0, j=0; maptab[i].input[0]; i++)
	printf ("%s%c", maptab[i].input, ((++j) & 7) ? '\t' : '\n');
    putchar ('\n');
    fprintf (stderr, *fmt++);
    fprintf (stderr, *fmt++);
    fprintf (stderr, *fmt++);
    fprintf (stderr, *fmt++);
    fprintf (stderr, *fmt++);
    fprintf (stderr, *fmt++);
    exit (1);
}

delete_keys ()
{
    if (ioctl(0, IOCCLRKEY, 0) == -1) {
	perror ("Setkey ioctl");
	exit (1);
    }
    exit (0);
}

print_keys (cmd)
    char *cmd;
{
    struct setkey sk;
    int i, j, k, m;
    char c;

    for (i = 0; maptab [i].code && (maptab [i].shift == 0); i++) {
	sk.k_code = maptab [i].code;
	for (j = 0; j <= (CAP_SHIFTED|CTRL_SHIFTED|ALT_SHIFTED); j++) {
	    sk.k_shift = j;
	    if (ioctl (0, IOCGETKEY, &sk) == -1) {
		perror ("Getkey ioctl");
		exit (1);
	    }
	    if (sk.k_len) {
		printf ("%s ", cmd);
		if (sk.k_shift & CAP_SHIFTED)  printf ("shift ");
		if (sk.k_shift & CTRL_SHIFTED) printf ("ctrl ");
		if (sk.k_shift & ALT_SHIFTED)  printf ("alt ");
		printf ("%s '", maptab [i].input);
		for (k = 0; k < sk.k_len; k++) {
		    if ((c = sk.k_data [k]) < ' ') {
			putchar ('^');
			c += '@';
		    }
		    else if (c == '^')
			putchar ('^');
		    putchar (c);
		}
		printf ("'\n");
	    }
	}
    }
    exit (0);
}
