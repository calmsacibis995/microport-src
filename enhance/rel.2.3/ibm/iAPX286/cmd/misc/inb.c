


#include <sys/io_op.h>

int	memfd;
unsigned char inb();

main(n, args) 
int		n;
char	**args;
	{
	int	port;
	unsigned long atol();

	if ((memfd = open("/dev/mem", 2)) == -1) {
		perror("Open /dev/mem");
		exit(1);
		}
	port = xtoi(args[1]);
	printf("Port %3x = %02x\n", port, inb(port));
	close(memfd);
	}

xtoi(s)
char *s;
{
	int	x;

	sscanf(s, "%x", &x);
	return x;
	}

extern	errno;

unsigned char 
inb(portno) {
	io_op_t		iop;

	iop.io_port = portno;
	errno = 0;
	ioctl(memfd, IOCIOP_RB, &iop);
	if (errno)
		perror("Input byte");
	return iop.io_byte;
	}

outb(portno, data) {
	io_op_t		iop;

	iop.io_port = portno;
	iop.io_byte = data;
	errno = 0;
	ioctl(memfd, IOCIOP_WB, &iop);
	if (errno)
		perror("Output byte");
	}


