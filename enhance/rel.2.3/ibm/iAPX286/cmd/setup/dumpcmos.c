static char *uportid = "@(#)dumpcmos.c	Microport Rev Id  1.3.2 6/10/86";


/*
 * This program is a simple version of 'setup'.
 * 'nsetup x' prints out the current value of cmos ram byte x
 * 'nsetup x y' sets byte (hex) x to (hex) y, and updates the checksum
 * Hacked to test /dev/cmos 2-14-86
 */

int	cmosfd;

main(n, args) 
int		n;
char	**args;
	{
	unsigned int port, data, cksum, i;
	unsigned long atol();
	unsigned char	cmosbuf[0x40];
	extern	errno;

	if ((cmosfd = open("/dev/cmos", 0)) == -1) {	/* M000 */
		perror("Open /dev/cmos");		/* M000 */
		exit(1);
		}
	read(cmosfd, cmosbuf, 0x40);
	if (errno) {
		perror("Read /dev/cmos");		/* M000 */
		exit(1);
		}
	close(cmosfd);
	if ((cmosfd = open("/dev/cmos", 1)) == -1) {	/* M000 */
		perror("Open /dev/cmos");		/* M000 */
		exit(1);
		}
	switch(n) {
		case 1:
			errno = 0;
			for(i=0; i<0x40; i++) {
				printf("(%2x,%02x)", i, 0xff & cmosbuf[i]);
				if (7 == (i % 8))
					printf("\n");
				}
			break;
		case 3:
			/* Set new value */
			port = xtoi(args[1]);
			data = xtoi(args[2]);
			printf("CMOS byte %x was %x is %x\n", port, cmosbuf[port], data);
			/* Get checksum */
			printf("CMOS checksum was %02x%02x", cmosbuf[0x2e], cmosbuf[0x2f]); 
			/* calculate new checksum */
			cmosbuf[port] = data;
			cksum = 0;
			for(i=0x10; i<0x21; i++) {
				cksum += cmosbuf[i];
				}
			printf(" is %4x\n", cksum);
			/* Set new checksum */
			cmosbuf[ 0x2e ] = cksum >> 8;
			cmosbuf[ 0x2f ] = cksum;

			errno = 0;
			if (0x40 != write(cmosfd, cmosbuf, 0x40))
				perror("Write cmos");
			break;
		default:
			printf("Usage: setup [hex hex]\n");
			exit(1);
		}
	close(cmosfd);
	exit(0);
	}

xtoi(s)
char *s;
{
	int	x;

	sscanf(s, "%x", &x);
	return x;
	}

