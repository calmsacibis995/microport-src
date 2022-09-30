/* uportid = "@(#)fdisk.h	Microport Rev Id  1.3.3 6/18/86" */

/* Modification History:
 * M000		uport!bernie Mon Apr 20 15:07:35 PST 1987
 * 	Improved consistency of help menu.
 */

struct partition {
	unsigned char bootind;		/* boot indicator		*/
	unsigned char bih;		/* boot indicator: head		*/
	unsigned char bis;		/* boot indicator: sector	*/
	unsigned char bicyl;		/* boot indicator: cylinder	*/
	unsigned char systind;		/* system indicator		*/
	unsigned char sih;		/* system indicator: head	*/
	unsigned char sis;		/* system indicator: sector	*/
	unsigned char sicyl;		/* system indicator: cylinder	*/
	union {				/* relative sector: 		*/
		unsigned char s[4];
		unsigned long sector;
	} rel;
	union {				/* # of sectors in partition	*/
		unsigned char n[4];
		unsigned long partsize;	
	} nsec;
};

struct parttab {
	struct partition p[4];
	union {
		unsigned int signature;
		unsigned char s[2];
	} sig;
} ptab, wptab, dummyptab;
/* ptab contains the information that was read off of the disk.		*/
/* wptab is a working copy of this, for the user. It contains all 	*/
/* changes to the partition table, and is written out to the disk 	*/
/* during a 'w' (write) or 'q' (quit) command				*/


/* what the main menu looks like				*/
static char *mainmenu[] = { 
	"------------------------------------------------------\n",
	"Microport's Fixed Disk Setup Program\n\n",
	"FDISK Options:\n\n\n\n\n",
	"Choose one of the following:\n",
	"\t 1. Create a Partition\n",
	"\t 2. Change Active Partition\n",
	"\t 3. Delete a Partition\n",
	"\t 4. Display Partition Information\n",
	"\t 5. Scan and Assign Bad Tracks\n",
	"\t 6. Advance to next disk unit-Current unit is ",/* must be entry[9]*/
	"Press 'x' to return to UNIX\n",
	"Enter choice <cr>:............: ",
	 "0",
};

/* menu #1: Create a Partition					*/
static char *menu1[] = { 
	"Microport's Fixed Disk Setup Program\n\n",
	"Create a Partition\n\n\n\n",
	"0",					/* Display Part. Info here */
};

/* menu #2: Change Active Partition					*/
static char *menu2[] = { 
	"Microport's Fixed Disk Setup Program\n\n",
	"Change the Active Partition\n\n\n\n",
	"0",					/* Display Part. Info here */
	"Enter the number of the partition that you\n",
	"want to make active.......................",
	"0",
};

/* menu #3: Delete a partition						*/
static char *menu3[] = { 
	"Microport's Fixed Disk Setup Program\n\n",
	"Delete a Partition\n\n\n\n",
	"0",					/* Display Part. Info here */
	"WARNING! All data on this partition\n",
	"will be DESTROYED! Do you wish\n",
	"to continue.................?:N",
	"0",
};

/* menu #4: Display Partition Information	*/
static char *menu4[] = { 
	"Microport's Fixed Disk Setup Program\n\n",
	"Display Partition Information\n\n\n\n",
	"0",					/* Display Part. Info here */
};

/* help menu. response to '?' or 'h'.					*/
static char *helpmenu[] = { 
	"Microport's Fixed Disk Setup Program\n\n",  /* M000 */
	"Information about FDISK program\n\n\n\n",   /* M000 */
	"FDISK is used to change the 'partition table' on your hard disk.\n",
	"This table defines the regions on the hard disk that different\n",
	"operating systems (like DOS or UNIX) can use.\n\n",
	"If you don't know how to use FDISK, it is highly recommended\n",
	"that you read the documentation. IMPROPER USE OF THE FDISK\n",
	"PROGRAM CAN DESTROY DATA ON YOUR HARD DISK!\n\n",
	"Hit the delete key if you want to be certain of leaving FDISK \n",
	"without updating the partition table.\n",
	"0",
};
