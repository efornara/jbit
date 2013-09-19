JBit
====

JBit is a 6502 simulator and development environment. It targets feature
phones and can be found here:

<http://jbit.sf.net> (XHTML site, optimized for feature phones)

or

<http://jbit.sf.net/m> (WAP site, for old phones / very low bandwidth)

This version of JBit targets desktops and browsers and comes in two
flavours: *Native* and *JavaScript*.

Native
------

The following should work (tested mostly on debian, but occasionally on
NetBSD):

	cd native
	make

If you are missing ncurses (or have curses), comment out the offending
lines in the Makefile (or edit them). A binary file for Windows is
also available (see the wiki below).

The simulator / assembler is self-contained and does not required
installation: just copy the executable wherever you want and type
something like this:

	jbit sample

The assembly samples are in the directory format.

JavaScript
----------

The JavaScript version uses
[emscripten](https://github.com/kripken/emscripten), but it's not
working at the moment. You can see an older version running here:

<http://jbit.sourceforge.net/gallery/list.php?tag=microio>

Browse the gallery and click on *sim* to run the sample on your browser
and *jb* to download it to run it on the native simulator.

More info
---------

<https://github.com/efornara/jbit/wiki>
