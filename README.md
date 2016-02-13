JBit
====

JBit is really two projects in one:

1. **Native**: A 6502 assembler / simulator for Linux(-like) computers
and gadgets that gives you easy access to a simplified subset of their kernel
(read, fork, exec, etc...). A limited executable is also available for Windows.

2. **J2ME**: A small java application for old feature phones (MIDlet)
that gives you a programmable 8-bit microcomputer. You can edit, save, run and
debug assembly (6502) programs directly on your phone.

[![Build Status](https://api.travis-ci.org/efornara/jbit.svg?branch=master)](https://travis-ci.org/efornara/jbit/builds)

## Getting JBit

There is usually no need for you to build JBit from source.

If you are looking for the J2ME version of JBit (for feature phones),
[pre-built midlets](http://jbit.sourceforge.net/download.html)
are available.

If you have Windows, you can find a precompiled binary following the
instructions found
[here](https://github.com/efornara/jbit/wiki/Windows).

If you have an Android phone/tablet, you can find precompiled binaries
following the instructions found
[here](https://github.com/efornara/jbit/wiki/Android).

However, for Linux(-like) computers, building the Native version of JBit is
recommended. Unlike the J2ME version, the Native version has few dependencies
and should be fairly easy to build.

## Native

The following should work:

	cd native
	make

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

## J2ME

To compile from source you need:

* [Java SE Development Kit](http://www.oracle.com/technetwork/java/javase/downlo
ads/index.html).
  Tested with OpenJDK version 1.7.0.
* [Oracle Java Wireless Toolkit](http://www.oracle.com/technetwork/java/download-135801.html).
  Tested with version 2.5.2.
* [Ant](http://ant.apache.org/).
  Tested with versions 1.9.6.
* [Antenna](http://antenna.sourceforge.net/).
  Tested with version 1.2.1beta.
* [ProGuard](http://proguard.sourceforge.net/).
  Tested with version 5.2.1.

If your environment is setup correctly, you should be able to
compile JBit by setting the `WTK_HOME_DIRECTORY` environment variable,
entering midp/jbit and running ant.

## Docker

A few docker images are available ([efornara/jbit](https://hub.docker.com/r/efornara/jbit/)):

	busybox  jbit binary hosted in a busybox environment
	main     development setup for Native (and documentation)
	midp     development setup for J2ME (Oracle WTK still needed)
	droid    development setup for Android (cross compiler)
	win32    development setup for Win32 (cross compiler)

## Links

* [JBit XHTML Site](http://jbit.sourceforge.net/)
  (optimized for feature phones).
* [JBit WAP Site](http://jbit.sourceforge.net/m)
  (for old phones / very low bandwidth).
* [JBit Wiki](https://github.com/efornara/jbit/wiki)
  (too heavy for most feature phones).
