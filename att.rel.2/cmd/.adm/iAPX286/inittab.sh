fs::sysinit:/etc/bcheckrc </dev/console >/dev/console 2>&1
mt::sysinit:/etc/brc </dev/console >/dev/console 2>&1
ck::sysinit:/etc/bsetdate </dev/console >/dev/console 2>&1
is:2:initdefault: # iAPX286 @(#)inittab.sh	1.3
pf::powerfail:/etc/powerfail >/dev/console 2>&1 #power fail routines
s0:056:wait:/etc/rc0 </dev/console >/dev/console 2>&1
s2:2:wait:/etc/rc2 </dev/console >/dev/console 2>&1
d1:056:wait:/bin/kill -15 -1 > /dev/console < /dev/console 2>&1
d2:056:wait:sleep 5
d3:056:wait:/bin/kill -9 -1 > /dev/console 2>&1
d4:056:wait:sleep 5
d5:056:wait:/etc/umountall > /dev/console 2>&1
d6:056:wait:echo '\nThe system is down.' > /dev/console
r0:0:wait:/etc/uadmin 2 0	# halt and power off if possible
r5:5:wait:/etc/uadmin 2 2	# return to firmware if possible
r6:6:wait:/etc/uadmin 2 1	# reboot if possible
co:1234:respawn:/etc/getty console console
ax:2:off:/etc/getty ttyaux 9600
0:2:off:/etc/getty tty0 9600
1:2:off:/etc/getty tty1 9600
2:2:off:/etc/getty tty2 9600
3:2:off:/etc/getty tty3 9600
4:2:off:/etc/getty tty4 9600
5:2:off:/etc/getty tty5 9600
6:2:off:/etc/getty tty6 9600
7:2:off:/etc/getty tty7 9600
8:2:off:/etc/getty tty8 9600
9:2:off:/etc/getty tty9 9600
