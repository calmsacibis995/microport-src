#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#	@(#)	1.6
#
#	Curses Library Low Level Makefile
#
#	CFLAGS has -DVIDEO & -DKEYPAD for the libcurses.a library.  They could
#		be put into a separate variable if desired, but I saw no reason
#		for doing so.
#	DFLAGS, TFLAGS, & PFLAGS are for debugging, tracing, & profiling
#		respectively.  They all contain the DEBUG flag because it is
#		thought that any of the styles of compiling is still debugging.
#		Also the DEBUG flag will create very large files and it is not
#		recommended that all of the files be compiled with the DEBUG
#		flag defined.  This will make 'ar' die because the library will
#		exceed 2065 blocks in size.  But I see no reason not to compile
#		individual or groups of files with these flags defined.  These
#		flags can't be used on a pdp11/70 or other small machines
#		because of the huge sizes of .o files produced for the library.
#	FILES is used for the naming of the object files.
#	O is used to destinguish between compiling, debugging, tracing, &
#		profiling.
#	MINICURSES is a flag used to compile a small version of libcurses.
#		It should not be used for the compiling of libcurses.a.  It is
#		strictly for the application programmers convenience.
#		If MINICURSES is desired with ti4 or show then type:
#			"make <[ti4][show] MINICURSES=-DMINICURSES"
#	CC2 is similar to $(CC) except that it allows the cross-compiler people
#		to make the tic compiler in native mode while doing everything
#		else in cross-compilation mode.
#	PFX is a variable used to allow a different lorder command to be used
#		when cross-compiling.  It is only used when archiving libcurses.
#
ROOT		=
INCLUDE		= $(ROOT)/usr/include
CURSES		= libcurses.a
BIN		= $(ROOT)/usr/bin
O		= o
LIB		= $(ROOT)/usr/lib
MINICURSES	=
CC2		= cc
CFLAGS		= -DVIDEO -DKEYPAD -O
DFLAGS		= -DVIDEO -DKEYPAD -DDEBUG -I.
TFLAGS		= -DVIDEO -DKEYPAD -DDEBUG -O -I.
PFLAGS		= -DVIDEO -DKEYPAD -DDEBUG -p -O -I.
FILES		= __cflush.$(O) __sscans.$(O) _blanks.$(O) _c_clean.$(O) \
		_clearhl.$(O) _clearline.$(O) _comphash.$(O) _delay.$(O) \
		_delchars.$(O) _dellines.$(O) _dumpwin.$(O) _ec_quit.$(O) \
		_fixdelay.$(O) _forcehl.$(O) _hlmode.$(O) _id_char.$(O) \
		_init_cost.$(O) _inschars.$(O) _insmode.$(O) _kpmode.$(O) \
		_line_free.$(O) _ll_move.$(O) _outch.$(O) _outchar.$(O) \
		_pos.$(O) _reset.$(O) _scrdown.$(O) _scrollf.$(O) _sethl.$(O) \
		_setmode.$(O) _setwind.$(O) _shove.$(O) _sprintw.$(O) \
		_sputc.$(O) _syncmodes.$(O) _tscroll.$(O) _window.$(O) \
		addch.$(O) addstr.$(O) baudrate.$(O) beep.$(O) box.$(O) \
		capnames.$(O) cbreak.$(O) chktypeahd.$(O) clear.$(O) \
		clearok.$(O) clreolinln.$(O) clrtobot.$(O) clrtoeol.$(O) \
		cntcostfn.$(O) crmode.$(O) curses.$(O) def_prog.$(O) \
		def_shell.$(O) delayoutpt.$(O) delch.$(O) deleteln.$(O) \
		delwin.$(O) doprnt.$(O) doscan.$(O) doupdate.$(O) draino.$(O) \
		echo.$(O) endwin.$(O) erase.$(O) erasechar.$(O) fixterm.$(O) \
		flash.$(O) flushinp.$(O) getch.$(O) getstr.$(O) idlok.$(O) \
		gettmode.$(O) has_ic.$(O) has_il.$(O) idln.getst.$(O) \
		initkeypad.$(O) initscr.$(O) insch.$(O) insertln.$(O) \
		intrflush.$(O) keypad.$(O) killchar.$(O) leaveok.$(O) \
		line_alloc.$(O) ll_refresh.$(O) longname.$(O) m_addch.$(O) \
		m_addstr.$(O) m_clear.$(O) m_erase.$(O) m_move.$(O) meta.$(O) \
		m_refresh.$(O) m_tstp.$(O) makenew.$(O) miniinit.$(O) \
		move.$(O) mvcur.$(O) mvprintw.$(O) mvscanw.$(O) mvwin.$(O) \
		mvwprintw.$(O) mvwscanw.$(O) naps.$(O) newpad.$(O) \
		newterm.$(O) newwin.$(O) nl.$(O) nocbreak.$(O) nocrmode.$(O) \
		nodelay.$(O) noecho.$(O) nonl.$(O) noraw.$(O) nttychktrm.$(O) \
		overlay.$(O) overwrite.$(O) pnoutrfrsh.$(O) prefresh.$(O) \
		printw.$(O) putp.$(O) raw.$(O) reset_prog.$(O) resetshell.$(O) \
		resetterm.$(O) resetty.$(O) restarttrm.$(O) saveterm.$(O) \
		savetty.$(O) scanw.$(O) scroll.$(O) scrollok.$(O) select.$(O) \
		set_term.$(O) setbuffred.$(O) setterm.$(O) setupterm.$(O) \
		showstring.$(O) subwin.$(O) tgetent.$(O) tgetflag.$(O) \
		tgetnum.$(O) tgetstr.$(O) tgoto.$(O) touchwin.$(O) tparm.$(O) \
		tputs.$(O) traceonoff.$(O) tstp.$(O) two.twostr.$(O) \
		typeahead.$(O) unctrl.$(O) vidattr.$(O) vidputs.$(O) \
		vsprintf.$(O) vsscanf.$(O) wattroff.$(O) wattron.$(O) \
		wattrset.$(O) wnoutrfrsh.$(O) wprintw.$(O) wrefresh.$(O) \
		writechars.$(O) wscanw.$(O) wstandend.$(O) wstandout.$(O)

.SUFFIXES:	.o .d .t .p

.c.o:	
			$(CC) $(CFLAGS) -M$(MSIZE) $(MINICURSES) -c $*.c

.c.d:	
			if [ -f $*.o ] ; then mv $*.o tmp ; fi
			$(CC) $(DFLAGS) -M$(MSIZE) $(MINICURSES) -c $*.c
			mv $*.o $*.d
			if [ -f tmp ] ; then mv tmp $*.o
.c.t:	
			if [ -f $*.o ] ; then mv $*.o tmp ; fi
			$(CC) $(TFLAGS) -M$(MSIZE) $(MINICURSES) -c $*.c
			mv $*.o $*.d
			if [ -f tmp ] ; then mv tmp $*.o

.c.p:	
			if [ -f $*.o ] ; then mv $*.o tmp ; fi
			$(CC) $(PFLAGS) -M$(MSIZE) $(MINICURSES) -c $*.c
			mv $*.o $*.d
			if [ -f tmp ] ; then mv tmp $*.o

all:		$(CURSES)

$(CURSES):	$(FILES) curses.h curshdr.h term.h unctrl.h curses.ext curses.c
		$(AR) rv $(CURSES) `$(PFX)lorder *.$(O) | tsort`

debug:
		$(MAKE) O=d CURSES=libdcurses.a

trace:
		$(MAKE) O=t CURSES=libtcurses.a

profile:
		$(MAKE) O=p CURSES=libpcurses.a

setupterm.$(O):	setupterm.c
		${CC} ${CFLAGS} -M$(MSIZE) $(MINICURSES) -DJIOCTLP -c setupterm.c

show:		libcurses.a show.o
		$(CC) $(CFLAGS) -M$(MSIZE) $(MINICURSES) show.o libcurses.a -o show
		/bin/rm show.o

tic:		tic.c capnames.c
		-if pdp11; \
		then \
			$(CC2) -i -s $(CFLAGS) $(MINICURSES) -DROOT="\"$(ROOT)/\"" tic.c capnames.c -o tic; \
		else \
			$(CC2) -s $(CFLAGS) $(MINICURSES) -DROOT="\"$(ROOT)/\"" tic.c capnames.c -o tic;\
		fi

ti4:		libcurses.a ti4.o
		cc $(CFLAGS) $(MINICURSES) ti4$(O) libcurses.a -o ti4
		/bin/rm ti4.o

ti4.o:		
		cc mkti4.c -O -o mkti4
		mkti4 >ti4.c
		cc $(CFLAGS) $(MINICURSES) ti4.c -O -c

clean:	
		/bin/rm -f *.o

clobber:	clean
		/bin/rm -f libcurses.a tic

install:	libcurses.a tic
# libcurses.mk does install		cp libcurses.a $(LIB)/$(MODEL)/libcurses.a
#		cp tic $(BIN)/tic
#		$(CH) chmod 775 $(BIN)/tic $(LIB)/$(MODEL)/libcurses.a
#		$(CH) chgrp bin $(BIN)/tic $(LIB)/$(MODEL)/libcurses.a
#		$(CH) chown bin $(BIN)/tic $(LIB)/$(MODEL)/libcurses.a

tic.native:		tic.c capnames.c
		-if iAPX286; \
		then \
			$(CC) -Ml -s $(CFLAGS) $(MINICURSES) -DROOT="\"$(ROOT)/\"" tic.c capnames.c -o tic; \
		fi

install.tic.native:	tic.native
	install -n $(ROOT)/usr/bin tic
