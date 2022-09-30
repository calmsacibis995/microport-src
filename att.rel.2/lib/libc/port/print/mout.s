doprnt.c:
	cc -c -p -O -Ms doprnt.c    && mv doprnt.o doprnt.p
	cc -O -Ms -c doprnt.c
fprintf.c:
	cc -c -p -O -Ms fprintf.c    && mv fprintf.o fprintf.p
	cc -O -Ms -c fprintf.c
printf.c:
	cc -c -p -O -Ms printf.c    && mv printf.o printf.p
	cc -O -Ms -c printf.c
sprintf.c:
	cc -c -p -O -Ms sprintf.c    && mv sprintf.o sprintf.p
	cc -O -Ms -c sprintf.c
vfprintf.c:
	cc -c -p -O -Ms vfprintf.c    && mv vfprintf.o vfprintf.p
	cc -O -Ms -c vfprintf.c
vprintf.c:
	cc -c -p -O -Ms vprintf.c    && mv vprintf.o vprintf.p
	cc -O -Ms -c vprintf.c
vsprintf.c:
	cc -c -p -O -Ms vsprintf.c    && mv vsprintf.o vsprintf.p
	cc -O -Ms -c vsprintf.c
