static char *uportid = "@(#)setup.c	Microport Rev Id  2.3	5/6/87";

/* Copyright @ 1986 by Microport Systems. All Rights Reserved.	*/

/*
 * Keyword-based setup program
 * M000 June 5, 1986    Lance
 *      Add support for type 20 drives.
 * M001 June 25, 1986	dwight
 *	Changed gethour() to handle 24 hour overflows. 
 *	Undef'ed Hour 12. This code should be based on bit 1 of stat reg B.
 * M002 Wed Jul  9 14:51:21 PDT 1986	uport!dwight
 *	Changed the checksum calculations to go from 0x10 to 0x2D;
 *	was 0x10 to 0x21. Re pps. 5-44 and 1-45 in the AT tech ref. manual.
 * M003 Wed May  6 19:50:07 PDT 1987
 *	added UID check so everyone can display but only superuser
 *	can write.
 */

#include <sys/types.h>
#include <time.h>
#include <stdio.h>

#define NSTRS   10
#define NKEYS   7

int showbase(), chgbase();
int showext(),  chgext();
int showdate(), chgdate();
int showtime(), chgtime();
int showflop(), chgflop();
int showdisk(), chgdisk();
int showcon(),  chgcon();

struct {
        char    *key;
        char    *subkeys[NSTRS];
        char    *showhelp;
        char    *chghelp[NSTRS];
        int             (*showfunc)();
        int             (*chgfunc)();
} menu[NKEYS] = {
        {
        "base",
        {"512", "640", 0},
        "\t\tprints Kbytes of base memory",
        {"\t\tsets base memory to 512 Kbytes\n\t\t\t\t(assumes 0K extended memory)",
                "\t\tsets base memory to 640 Kbytes", 0},
        showbase,
        chgbase,
        },
        {
        "ext",
        {"number", 0},                                          /* Handled by routine */
        "\t\tprints Kbytes of extended memory",
        {"\tsets extended memory to number Kbytes", 0},
        showext,
        chgext,
        },
        {
        "date",
        {"mm/dd/yy", "mm/dd/yyyy", 0},
        "\t\tprints current date setting",
        {"\tsets date to mm/dd/yy", "sets date to mm/dd/yyyy", 0},
        showdate,
        chgdate,
        },
        {
        "time",
        {"hh:mm:ss", 0},
        "\t\tprints current time setting",
        {"\tsets time to hh:mm:ss", 0},
        showtime,
        chgtime,
        },
        {
        "flop",
        {"1 low/high", "2 none", "2 low/high", 0},
        "\t\tprints current floppy configuration",
        {"sets first  floppy drive to 48/96 tpi",
         "\tsets no second floppy drive",
         "sets second floppy drive to 48/96 tpi", 0},
        showflop,
        chgflop,
        },
        {
        "fixed",
        {"1 [1-99]", "2 none", "2 [1-99]", 0},          /* M000 */
        "\t\tprints current fixed disk configuration",
        {"\tsets first  fixed drive to type", 
         "\tsets second fixed drive as not installed",
         "\tsets second fixed drive to type"},
        showdisk,
        chgdisk,
        },
        {
        "con",
        {"mono", "colr40", "colr80", "ega", 0},
        "\t\tprints current console",
        {"\tsets console to monochrome adapter, 80x25",
         "\tsets console to color adapter, 40x25",
         "\tsets console to color adapter, 80x25",
         "\tsets console to enhanced adapter, 80x25"},
        showcon,
        chgcon,
        }
};

#define bcdtoi(x) (((x) & 0xf) + (((x) >> 4) * 10))
#define itobcd(x) ((((x) / 10) << 4) | ((x) % 10))

unsigned char   cmosbuf[0x40];
int                             cmosfd;

char *myname, *key, *subkey1 = 0, *subkey2 = 0;

main(n, args) 
int             n;
char    **args;
{
        int     i, j, k;
        char    answer[100];

        myname  = args[0];
        if (n > 1)
                key         = args[1];
        if (n > 2)
                subkey1 = args[2];
        if (n > 3)
                subkey2 = args[3];
        getcmos();
        if (n == 2 && !strcmp("-d", key)) {
                /* print out date & time in date(1) format */
                datestr();
                exit(0);
                }
        if (!strcmp("all", key)) {
                for(i=0; i< NKEYS; i++) {
                        (*menu[i].showfunc)();
                        }
                fflush(stdout);
                exit(0);
                }
                        
        for(i=0; i< NKEYS; i++)
                if (!strcmp(key, menu[i].key)) 
                        break;
        if (i == NKEYS) {
                printf("Usage:\t%s -d\tprint date&time in 'date' format\n",
                                myname);
                printf("\t\t%s all\tshow all current values\n", myname);
                printf("\t\t%s %s %s\n", myname, menu[0].key,
                        menu[0].showhelp);
                for(k=0; menu[0].subkeys[k]; k++)
                        printf("\t%s %s %s %s\n", myname, menu[0].key, 
                                menu[0].subkeys[k], menu[0].chghelp[k]);
                for(j=1; j < NKEYS; j++) {
                        printf("\t%s %s %s\n",myname,menu[j].key,menu[j].showhelp);
                        for(k=0; menu[j].subkeys[k]; k++)
                                printf("\t%s %s %s %s\n", myname, menu[j].key,
                                        menu[j].subkeys[k], menu[j].chghelp[k]);
                        }
                exit(1);
                }
        if (n == 2) {
                (*menu[i].showfunc)();
        } else {
					/* M003 */
		if (getuid()) {
			fprintf(stderr,"Permission to change cmos denied\n");
			exit(1);
		}
                (*menu[i].chgfunc)();
                for(;;) {
                        printf("Are you sure? (type y or n)");
                        fflush(stdout);
                        gets(answer);
                        if ((answer[0] == 'y') || (answer[0] == 'Y')) {
                                putcmos();
                                printf("Cmos changed\n");
                                exit(0);
                                }
                        if ((answer[0] == 'n') || (answer[0] == 'N')) {
                                printf("No change to cmos\n");
                                exit(0);
                                }
                        }
        }
}

showbase() {
        unsigned size;

        size = (cmosbuf[0x16] << 8) | cmosbuf[0x15];
        printf("Base memory = %d Kbytes\n", size);
}

chgbase() {
        int     size;

        size = atoi(subkey1);
        if ((size == 512) || (size == 640)) {
                cmosbuf[0x16] = size >> 8;
                cmosbuf[0x15] = size;
        } else {
                printf("%s: Can only set base memory to 512 or 640, not %d Kbytes\n",
                        myname, size);
                exit(1);
                }
        printf("Change base memory to %d Kbytes\n", size);
}

showext() {
        int     size;

        size = (cmosbuf[0x18] << 8) | cmosbuf[0x17];
        printf("Extended memory = %d Kbytes\n", size);
}

chgext() {
        unsigned        size;

        size = atoi(subkey1);
        if ((size <= 0x3C00) && ((size & 0x7f) == 0)) {
                cmosbuf[0x18] = size >> 8;
                cmosbuf[0x17] = size;
        } else {
                printf("%s: Can only set extended memory to 0\n", myname);
                printf(
"\tor a multiple of 128 less than or equal to 15360, not %d Kbytes\n", size);
                exit(1);
                }
        printf("Change extended memory to %d Kbytes\n", size);
}

/*
 * Just print date & time in date(1) format: mmddhhmmyy.ss
 */

datestr() {
        printf("%02d%02d%02d%02d%02d\n",
                bcdtoi(cmosbuf[8]), bcdtoi(cmosbuf[7]), gethour(),
                bcdtoi(cmosbuf[2]), bcdtoi(cmosbuf[9]));
}

char *daystr[7] = {
        "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"
};

char *monthstr[12] = {
        "January", "February", "March", "April", "May", "June", "July",
        "August", "September", "October", "November", "December"
};

showdate() {
        char year[10];

        if (cmosbuf[ 0x32 ])
                sprintf(year, "%d", bcdtoi(cmosbuf[0x32]));
        else
                year[0] = 0;
        printf("Date is %s, %s %d, %s%d\n", 
                daystr[cmosbuf[6]], monthstr[bcdtoi(cmosbuf[8]) - 1],
                bcdtoi(cmosbuf[7]), year, bcdtoi(cmosbuf[9]));
}

unsigned monthtab[13] = {
0, 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

int     century = 19;

chgdate() {
        unsigned month, day, year, dayofweek();
        char *yrscan, *strrchr();

        if (!goodtime( gethour(), bcdtoi(cmosbuf[2]), bcdtoi(cmosbuf[0]))) {
                showtime();
                printf("CMOS time setting is invalid, it must be set first\n");
                exit(1);
                }
        if (sscanf(subkey1, "%d/%d/%d", &month, &day, &year) != 3) 
                printf("\t'%s' incorrect format\n", subkey1);
        else if (month == 0 || month > 12)
                printf("\tMonth must be 1 to 12\n");
        else if (day == 0 || day > monthtab[month])
                printf("\tDay must be 1 to %d\n", monthtab[month]);
        else if ((month == 2) && (((year/4)*4) != year) && (day > 28))
                printf("\tDay must be 1 to 28, %d is not a leap year\n", year);
        else {
                cmosbuf[7] = itobcd(day);
                cmosbuf[8] = itobcd(month);
                cmosbuf[9] = itobcd(year % 100);
                yrscan = strrchr(subkey1, '/');
                if (strlen(&yrscan[1]) > 2)                     /* typed 1986 */
                        century = year / 100;
                cmosbuf[6] = dayofweek();
                if (century) {
                        cmosbuf[0x32] = itobcd(century);        /* century byte */
                        printf("Set date to %s, %s/%d/%d%02d\n", daystr[cmosbuf[6]], 
                                monthstr[month - 1], day, century, year%100);
                } else
                        printf("Set date to %s, %s/%d/%d%02d\n", daystr[cmosbuf[6]], 
                                monthstr[month - 1], day, bcdtoi(cmosbuf[0x32]), year);
                return;
                }
        /* If error, get to here */
        exit(1);
}

#undef HOUR12					/* M001 */

/*
 * The time and date bytes are in BCD.
#ifdef  HOUR12
 * The hour byte is in 12-hour mode: bits 0..6 are (in BCD) 1..12,
 * and bit 7 signifies AM (0) or PM (1).
 * 12:00:00 is midnight, 
 * 11:59:59 is just before noon,
 * 92:00:00 is noon,
 * 81:00:00 is 1PM,
 * 91:59:50 is just before midnight.  It all makes perfect sense!
#endif  HOUR12
 */

showtime() {
        unsigned        hour, cmoshour;

        hour = gethour();
        printf("The time is %02d:%02d:%02d\n", 
                hour, bcdtoi(cmosbuf[2]), bcdtoi(cmosbuf[0]));
}

chgtime() {
        unsigned hour, minute, second;

        if (sscanf(subkey1, "%d:%d:%d", &hour, &minute, &second) != 3) 
                printf("\t'%s' incorrect format\n", subkey1);
        else if (hour > 23)
                printf("\tHour must be 0 to 23\n");
        else if (minute > 59)
                printf("\tMinute must be 0 to 59\n");
        else if (second > 59)
                printf("\tSecond must be 0 to 59\n");
        else {
#ifdef  HOUR12
                if (hour == 0)
                        cmosbuf[4] = 0x12;              /* midnight */
                else if (hour == 12)
                        cmosbuf[4] = 0x92;              /* noon */
                else if (hour > 11)                     /* all other PM hours */
                        cmosbuf[4] = itobcd(hour % 12) | 0x80;
                else                                            /* all other AM hours */
#endif  HOUR12
                        cmosbuf[4] = itobcd(hour);
                cmosbuf[2] = itobcd(minute);
                cmosbuf[0] = itobcd(second);
                printf("Set time to %02d:%02d:%02d\n", hour, minute, second);
                return;
                }
        /* If error, get to here */
        exit(1);
}

int goodtime(hour, min, sec) 
unsigned hour, min, sec;
{
        return (hour <= 24) && (min <= 59) && (sec <= 59);
}

/*
 * Set floppy drive configurations
 */

char *floptypes[4] = {"none", "low", "high", 0};

showflop() {
        int     flopnum;
        unsigned char floptype;

        floptypes[0] = "not installed";         /* kludge! */
        for(flopnum = 0; flopnum <= 1; flopnum++) {
                floptype = cmosbuf[ 0x10 ];
                floptype &= (flopnum ? 0x0f : 0xf0);
                floptype >>= (flopnum ? 0 : 4);

                if (floptype > 2)
                        printf("%s floppy drive: unknown type %x\n",
                                flopnum ? "Second" : "First", floptype);
                else
                        printf("%s floppy drive: type %s\n",
                                flopnum ? "Second" : "First ", floptypes[ floptype ]);
                }
        }

chgflop() {
        int     flopnum, floptype;

        if (!subkey1) {
                showflop();             /* kludge! */
                exit(0);
                }

        flopnum = atoi(subkey1);
        if (flopnum < 1 || flopnum > 2) {
                printf("Arguments are 'flop 1 type' or 'flop 2 type'\n");
                exit(1);
                }
        flopnum--;
        
        if (-1 == (floptype = search(subkey2, floptypes))) {
                printf("Floppy type arguments are 'low', 'high', or 'none'\n");
                exit(1);
                }

        if (flopnum == 0 && floptype == 0) {
                printf("There's always a diskette installed!\n");
                exit(1);
                }
        
        cmosbuf[ 0x10 ] &= (flopnum ? 0xf0 : 0x0f);
        cmosbuf[ 0x10 ] |= (flopnum ? floptype : (floptype << 4));
        /* if changing second drive type, update equipment byte */
        if (flopnum) {
                cmosbuf[ 0x14 ] &= 0x3f;
                cmosbuf[ 0x14 ] |= (floptype ? 0x40 : 0);
                }
        /* There's always a diskette installed! */
        cmosbuf[ 0x14 ] |= 1;
        if (flopnum && !floptype)
                printf("Set second floppy drive as not installed\n");
        else
                printf("Set %s floppy drive as %s density\n",
                        flopnum ? "second" : "first", floptypes[ floptype ]);
}

showdisk() {
        int     disknum;
        unsigned char disktype;

        for(disknum = 0; disknum <= 1; disknum++) {
                disktype = cmosbuf[ 0x12 ];
                disktype &= (disknum ? 0x0f : 0xf0);
                disktype >>= (disknum ? 0 : 4);
                if ( disktype == 0xf )                  /* M000 */
                        disktype = cmosbuf[disknum ? 0x1a : 0x19];/* M000*/

                if (disktype == 0) 
                        printf("%s fixed disk drive: not installed\n",
                                disknum ? "Second" : "First " );
                else
                        printf("%s fixed disk drive: type %d\n",
                                disknum ? "Second" : "First ", disktype );
                }
        }

chgdisk() {
        unsigned        disknum, disktype, temp;

        if (!subkey1) {
                showdisk();             /* kludge! */
                exit(0);
                }

        disknum = atoi(subkey1);
        if ((disknum < 1) || (disknum > 2) || !subkey2) {
                printf("Arguments are 'disk 1 type' or 'disk 2 type'\n");
                exit(1);
                }
        disknum--;
        
        if (!strcmp(subkey2, "none"))
                disktype = 0;
        else
                disktype = atoi(subkey2);

        /* start M000 */
        if ((disktype < 0 || disktype > 15) 
                        && (disktype < 20 || disktype > 99)) {  
                printf("Disktype must be 0 to 15 or 20 to 99\n");
                exit(1);
                }

        if (disktype < 15) 
                temp = disktype;
        else {
                temp = 15;
                cmosbuf[ disknum ? 0x1a : 0x19 ] = disktype;
                }
        cmosbuf[ 0x12 ] &= (disknum ? 0xf0 : 0x0f);
        cmosbuf[ 0x12 ] |= (disknum ? temp : (temp << 4));
        /* end M000 */
        if (!disktype)
                printf("Set %s fixed disk drive as not installed\n",
                        disknum ? "second" : "first");
        else
                printf("Set %s fixed disk drive as type %d\n",
                        disknum ? "second" : "first", disktype );
}
        
char *contypes[ 5 ] = { "ega", "colr40", "colr80", "mono", 0};
showcon() {
        int     contype;

        contype = (cmosbuf[ 0x14 ] & 0x30) >> 4;
        printf("Console is %s\n", contypes[ contype ]);
}

chgcon() {
        int     contype;

        if (-1 == (contype = search(subkey1, contypes))) {
                printf("Argument '%s' must be one of 'mono', 'colr40', or 'colr80'\n", 
                        subkey1);
                exit(1);
                }
        cmosbuf[ 0x14 ] &= 0xcf;
        cmosbuf[ 0x14 ] |= (contype << 4);
        printf("Set console to %s\n", subkey1);
        }

/*
 * Low-level subroutines
 */

gethour() {
	unsigned char t;				/* M001 */
#ifdef  HOUR12
        if (cmosbuf[4] == 0x12)         /* midnight */
                return 0;
        else if (cmosbuf[4] == 0x92)            /* noon */
                return 12;
        else if (cmosbuf[4] > 0x11)                     /* all other PM hours */
                return bcdtoi(cmosbuf[4] & 0x7f) + 12;
        else                                            /* all other AM hours */
#endif  HOUR12
		t = bcdtoi(cmosbuf[4]) % 24;		/* M001 */
                return t;
}

unsigned
dayofweek() {
        time_t dtform(), nt;
        struct tm *ntm;

        nt = dtform(bcdtoi(cmosbuf[9]), bcdtoi(cmosbuf[8]), bcdtoi(cmosbuf[7]), 
                        gethour(), bcdtoi(cmosbuf[2]), bcdtoi(cmosbuf[0]));
        tzset();
        ntm = localtime(&nt);
        /*
        printf("Code = %ld\n", nt);
        printf(
         "Sec %d min %d hour %d mday %d\nmon %d year %d wday %d yday %d isdst %d\n",
                ntm->tm_sec,
                ntm->tm_min,
                ntm->tm_hour,
                ntm->tm_mday,
                ntm->tm_mon,
                ntm->tm_year,
                ntm->tm_wday,
                ntm->tm_yday,
                ntm->tm_isdst);
        */
        return (unsigned) (ntm->tm_wday);
}

search(key, subkey)
char    *key;
char    *subkey[];
{
        int     i;

        if (!key)
                return -1;
        for(i=0; subkey[i]; i++)
                if (!strcmp(key, subkey[i]))
                        break;
        if (subkey[i])
                return i;
        else
                return -1;
}

getcmos() {
	if (getuid() == 0)				/* M003 */
		cmosfd = open("/dev/cmos", 2);
	else
		cmosfd = open("/dev/cmos", 0);
        if (cmosfd == -1) {
                printf("%s: Can't open /dev/cmos\n", myname);
                exit(1);
                }
        if (read(cmosfd, cmosbuf, 0x40) != 0x40) {
                printf("%s: Can't read /dev/cmos\n", myname);
                exit(1);
                }
        }

putcmos() {
        int     i;
        unsigned cksum;
        extern errno;

        lseek(cmosfd, 0L, 0);
        /* Get checksum */
        /* calculate new checksum */
        cksum = 0;
        for(i=0x10; i <= 0x2D; i++) {		/* M002 */
                cksum += cmosbuf[i];
                }
        /* Set new checksum */
        cmosbuf[ 0x2e ] = cksum >> 8;
        cmosbuf[ 0x2f ] = cksum;

        errno = 0;
        if (0x40 != write(cmosfd, cmosbuf, 0x40)) {
                printf("%s: Can't write /dev/cmos\n", myname);
                exit(1);
                }
}
