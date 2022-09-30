# Thu Jun 19 13:56:08 PDT 1986	.cshrc rev. 1.3
#	-------- .cshrc ----------
# a local "dot-c-shell-r-c" file
#
# change the following:
set DUDE=guest
# hi-tech: understand it before using.
# setenv PATH .:$home/bin:/bin:/usr/bin

# the following sets your prompt to print out the current command number.
set prompt="<uport:\!> "

# setting noclobber will keep you from writing onto files that already exist.
# the history variable denotes the number of commands that csh will remember.
# ignoreeof prevents one from using ^D to loggout (use 'exit' instead).
#	set noclobber history=35 ignoreeof
	set history=35

# csh switch - tells one about new mail if set:
	set mail=(/usr/mail/$DUDE)
# Set local aliases. Everyone should have the following line:
	alias	a	alias
# ls is too slow!!
	a	e	'echo *'
	a	h	'history'
	a	l	'/bin/ls -CF'
	
	a	r	'rehash'
	a	sink	'source $home/.cshrc'
	a	tm	'tail -50 $mail'
