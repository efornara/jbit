JBit
====

If you are looking for the mobile (J2ME) version of JBit, you can find
it here:

<http://jbit.sourceforge.net> (XHTML site, optimized for feature phones)

or here:

<http://jbit.sourceforge.net/m> (WAP site, for old phones / very low bandwidth)

This version of JBit targets desktops and browsers.

## Native Version

The following should work (tested mostly on debian, but occasionally on
NetBSD):

	cd native
	make

If you are missing ncurses (or have curses), comment out the offending
lines in the Makefile (or edit them). The (n)curses library is needed
for the microio device, and, unless you are coming from the mobile
version of JBit, you are unlikely to need it.

A precompiled EXE for Windows is available here:
<http://sourceforge.net/projects/jbit/files/jbit/Native/v1.2/jbit.exe/download>.

The resulting binary (jbit) is self-contained, so you should be able to
run it from anywhere.  From now on, I assume that you have it in your
path.

Switch to the sample directory and type:

	jbit hello.asm

You should get back the
[usual message](http://en.wikipedia.org/wiki/Hello_world_program):

	Hello, World!

JBit supports a binary format, if you really need it:

	jbit -c jb hello.asm >hello.jb
	jbit hello.jb

More information is available in the included jbit(1).
You can find a copy online here:
[jbit(1)](http://efornara.github.io/jbit/jbit.1.html).

## JavaScript Version

The JavaScript version uses
[emscripten](https://github.com/kripken/emscripten), I assume you have
a working installation. At least, the following command:

    em++

should produce:

    emcc: no input files

Enter the js directory:

    cd js

Fetch some additional libraries (ace and jquery):

    make fetch

Copy some samples into the js directory:

    make copy_res

Build the core:

    make

The resulting directory can now be copied into a web server.

A live version is available here:

<http://jbit.sourceforge.net/webapp/>
