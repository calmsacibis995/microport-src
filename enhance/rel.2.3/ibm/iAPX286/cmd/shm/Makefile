# makefile for shm stuff
M=-Ml
O=-s -O
CC=/bin/cc
CFLAGS=$M $O $(TEST) -I. -I../..
LFLAGS=$M $O

all: shmcreate

.s.o:
	$(CC) $(CFLAGS) -c $*.s

shmcreate: shmcreate.c shmsys.s
	$(CC) $(CFLAGS) $M shmcreate.c shmsys.s -o shmcreate

clean:
	-rm -f shmcreate *.o

clobber:

