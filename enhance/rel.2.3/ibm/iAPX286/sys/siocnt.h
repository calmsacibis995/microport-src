#define SIBITS	5	/* low 5 bits used to determine 1 of 32 */
#define	UNIT(x)		(x & (0xff>>(8-SIBITS)))
#define NCHAN	1<<SIBITS
#define NINT	1<<SIBITS
#define SIO_CNT 	1<<SIBITS

