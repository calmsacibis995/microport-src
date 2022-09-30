stty erase  echoe -ixany

TERM=ansi; export TERM
if [ $LOGNAME = root ]; then
	news -n
fi

TZ=PST8PDT; export TZ

news() {
	/usr/bin/news $* | pg
}
