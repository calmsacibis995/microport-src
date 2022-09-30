doprnt.c:
	cc -c -p -O -Ml doprnt.c    && mv doprnt.o doprnt.p
	cc -O -Ml -c doprnt.c
fprintf.c:
	cc -c -p -O -Ml fprintf.c    && mv fprintf.o fprintf.p
	cc -O -Ml -c fprintf.c
printf.c:
	cc -c -p -O -Ml printf.c    && mv printf.o printf.p
	cc -O -Ml -c printf.c
sprintf.c:
	cc -c -p -O -Ml sprintf.c    && mv sprintf.o sprintf.p
	cc -O -Ml -c sprintf.c
vfprintf.c:
	cc -c -p -O -Ml vfprintf.c    && mv vfprintf.o vfprintf.p
	cc -O -Ml -c vfprintf.c
vprintf.c:
	cc -c -p -O -Ml vprintf.c    && mv vprintf.o vprintf.p
	cc -O -Ml -c vprintf.c
vsprintf.c:
	cc -c -p -O -Ml vsprintf.c    && mv vsprintf.o vsprintf.p
	cc -O -Ml -c vsprintf.c
