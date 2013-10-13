JBit
====

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
[jbit.exe](http://sourceforge.net/projects/jbit/files/jbit/Native/v1.2/jbit.exe/download).

The resulting binary (jbit) is self-contained, so you should be able to
run it from anywhere.  From now on, I assume that you have it in your
path.

Switch to the sample directory and type:

	jbit hello.asm

You should get back the usual message:

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

Fetch some additional libraries (ace and jquery) and some images (not kept
in the git repository):

    make fetch

Copy some samples into the js directory:

    make copy_res

Build the core:

    make

The resulting directory can now be copied into a web server.

A live version is available here:

<http://jbit.sourceforge.net/webapp/>

## J2ME Version

To compile the J2ME Version from source you will need:

* [Java SE Development Kit](http://www.oracle.com/technetwork/java/javase/downlo
ads/index.html).
  I use version 1.6.0; any version should be fine.
* [Sun Java Wireless Toolkit](http://www.oracle.com/technetwork/java/download-13
5801.html).
  I use versions 1.0.4 and 2.5; again, any version should be fine.
* [Ant](http://ant.apache.org/).
  I use versions 1.6.5 and 1.7.0; I have no idea if you can use other versions.
* [Antenna](http://antenna.sourceforge.net/).
  I use version 1.2.1beta; older versions should work.
* [ProGuard](http://proguard.sourceforge.net/).
  I use version 4.4; older versions should work.

If your environment is setup correctly, you should be able to
compile JBit by entering j2me/jbit, editing build.xml and running ant.

## Links

* [JBit XHTML Site](http://jbit.sourceforge.net/)
  (optimized for feature phones).
* [JBit WAP Site](http://jbit.sourceforge.net/m)
  (for old phones / very low bandwidth).
* [JBit Wiki](https://github.com/efornara/jbit/wiki)
  (too heavy for most feature phones).
