JBit
====

There is usually no need for you to build JBit from source.

If you are looking for the J2ME version of JBit (for feature phones),
[pre-built midlets](http://jbit.sourceforge.net/download.html)
are available.

If you just want get an idea of what JBit is about and have
a modern browser (IE8+), a (limited)
[webapp](http://jbit.sourceforge.net/webapp/index.html)
is also available.

If you have Windows, you can find a precompiled EXE following the
instructions on this
[wiki page](https://github.com/efornara/jbit/wiki/Windows).

However, for Linux(-like) operating systems, building the Native version
of JBit is recommended. Unlike the Javascript and the J2ME versions,
the Native version has few dependencies and should be fairly easy
to build.

## Native Version

The following should work (tested mostly on debian, but occasionally on
NetBSD):

	cd native
	make

If you are missing ncurses (or have curses), comment out the offending
lines in the Makefile (or edit them), or install the relevant package
(*libncurses5-dev* on debian and ubuntu).

The resulting binary (jbit) is self-contained, so you should be able to
run it from anywhere.  From now on, I will assume that you have it in
your path.

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

The current JavaScript version is being refactored to use nano (see
below), so it might be broken at any give time.

To check the progress, see:
<http://jbit.sourceforge.net/webapp/test/>.

The old version is still available:
<http://jbit.sourceforge.net/webapp/1.2/>.

## Nano

Nano is the new version of the VM that is going to be used in future
Native and JavaScript versions of JBit. It is written in C and includes
a modified version of the [Fake6502 CPU
emulator](http://rubbermallet.org/fake6502.c).

The same code base is used to generate three targets:

An image for (Arduino Uno)[http://arduino.cc/en/Main/arduinoBoardUno].
See [jbit(1)](http://efornara.github.io/jbit/jbit.1.html#PRIMO) for more
information.

A test native program. Just typing make should work, but you need the
development package of SDL 1.2. Note that this target might disappear
entirely, once the consolidated bits of nano are integrated into the
main Native version.

A new JavaScript emulator. Just like the old JavaScript version, you
need [emscripten](https://github.com/kripken/emscripten) installed. If
you have a working installation of emscripten, that is, if:

    emcc

produces something like:

    emcc: no input files

you should be able to type:

    make jbnano-c.js

and point your browser to the included index.html. Beware that I recently
updated emscripten and the suggested script (emsdk-portable) took about
9GB of disk space. This:

    wget -c http://jbit.sourceforge.net/webapp/test/jbnano-c.js

is probably a good enough alternative for most.

## J2ME Version

To compile the J2ME Version from source you need:

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
compile JBit by editing Env.defs, entering midp/jbit and running ant.

## Links

* [JBit XHTML Site](http://jbit.sourceforge.net/)
  (optimized for feature phones).
* [JBit WAP Site](http://jbit.sourceforge.net/m)
  (for old phones / very low bandwidth).
* [JBit Wiki](https://github.com/efornara/jbit/wiki)
  (too heavy for most feature phones).
