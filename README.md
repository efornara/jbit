JBit
====

JBit comes in two versions and is really two projects in one:

1. **Native**: A 6502 assembler / simulator for Linux(-like) systems
that gives you easy access to a simplified subset of their kernel
(read, fork, exec, etc...). A limited executable is also available for Windows.

2. **J2ME**: A small java application for old feature phones (MIDlet)
that gives you a programmable 8-bit microcomputer. You can edit, save, run and
debug assembly (6502) programs directly on your phone.

[![Build Status](https://api.travis-ci.org/efornara/jbit.svg?branch=master)](https://travis-ci.org/efornara/jbit/builds)

## Getting JBit

There is usually no need for you to build JBit from source:

- **Feature Phones**: pre-build midlets are available
[here](http://jbit.sourceforge.net/download.html).

- **Windows**: a precompiled binary is available following the instructions
[here](https://github.com/efornara/jbit/wiki/Windows).

- **Android**: precompiled binaries are available following the instructions
[here](https://github.com/efornara/jbit/wiki/Android).

However, for Linux(-like) systems, building the Native version of JBit is
recommended.

Building notes can be found in `COMPILE.md`.

## Compatibily

The Native version of JBit is usually tested on the latest *debian stable*.
In the past, it has been tested on:

- Windows 95
- Windows 10
- Wine 1.6.2
- DosBox v0.74
- Slackware 2.2.0
- NetBSD 1.6.2
- OpenBSD 5.8
- Mac OS X 10.8
- Android 1.5+
- Fire OS 5.1

When the J2ME version of JBit is updated, it is usually tested on a
*Samsung E2121B*. Occasionally, it is tested on:

- Nokia 2630
- Sony Ericsson K550
- Motorola V360

## Docker

A few docker images are available to ease cross development
([efornara/jbit](https://hub.docker.com/r/efornara/jbit/)):

	main     development setup for Native (and documentation)
	midp     development setup for J2ME (Oracle WTK still needed)
	droid    development setup for Android (cross compiler)
	win32    development setup for Win32 (cross compiler)
	dos      development setup for MS-DOS (cross compiler)

A small image is also available for trying out JBit:

	busybox  jbit binary hosted in a busybox environment

## Links

* [JBit XHTML Site](http://jbit.sourceforge.net/)
  (optimized for feature phones).
* [JBit WAP Site](http://jbit.sourceforge.net/m)
  (for old phones / very low bandwidth).
* [JBit Wiki](https://github.com/efornara/jbit/wiki)
  (too heavy for most feature phones).
