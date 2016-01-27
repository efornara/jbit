JBit
====

There is usually no need for you to build JBit from source.

If you are looking for the J2ME version of JBit (for feature phones),
[pre-built midlets](http://jbit.sourceforge.net/download.html)
are available.

If you have Windows, you can find a precompiled EXE following the
instructions on this
[wiki page](https://github.com/efornara/jbit/wiki/Windows).

However, for Linux(-like) operating systems, building the Native version
of JBit is recommended. Unlike the J2ME version,
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
compile JBit by setting the `WTK_HOME_DIRECTORY` environment variable,
entering midp/jbit and running ant.

## Links

* [JBit XHTML Site](http://jbit.sourceforge.net/)
  (optimized for feature phones).
* [JBit WAP Site](http://jbit.sourceforge.net/m)
  (for old phones / very low bandwidth).
* [JBit Wiki](https://github.com/efornara/jbit/wiki)
  (too heavy for most feature phones).
