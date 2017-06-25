JBit
====

JBit comes in two main versions:

- **midp**: A small java application for old feature phones (MIDlet)
that gives you a programmable 8-bit microcomputer. You can edit, save, run and
debug assembly (6502) programs directly on your phone.
Note that this is *not* an app for modern Android/iPhone phones.

- **native**: A 6502 assembler / simulator for the command line.
On Linux(-like) systems it gives you easy access to a simplified subset
of their kernel (read, fork, exec, etc...). This is also available to
use on Android terminal apps.

Sharing some code with the native version is a new version of JBit:

- **io2sim**: A [libretro](https://www.libretro.com/) core simulating an
useable subset of the midp version. A self-contained win32 executable
linked with a minimal libretro front-end is also available.

[![Build Status](https://api.travis-ci.org/efornara/jbit.svg?branch=master)](https://travis-ci.org/efornara/jbit/builds)

## Getting JBit

There is usually no need for you to build JBit from source.

A binary archive is available from
[sourceforge](https://sourceforge.net/projects/jbit/files/jbit/) and it
contains the most common binaries for midp, native (win32,
linux/android) and io2sim (win32), together with some documentation
and a few samples.
See [dists/bin/template/README.txt](dists/bin/template/README.txt) for
more information.

If you need to download JBit directly from a feature phone, there is
a WAP site that you can use: [jbit.sf.net/m](http://jbit.sf.net/m).
The [main JBit web site](http://jbit.sourceforge.net/) is a XHTML site
designed with feature phones in mind and it is also very lightweight.

Building notes to generate the io2sim libretro core or to build the native
version of JBit on a Linux(-like) system can be found in
[native/README.md](native/README.md).

## Compatibily

The native version of JBit is usually tested on the latest *debian stable*
and/or on *ubuntu xenial* (Bash on Windows).
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

The io2sim version of JBit is usually tested on Windows 10.
Occasionally, it is tested on:

- RetroArch
- Windows 95
- DosBox v0.74

When the midp version of JBit is updated, it is usually tested on a
*Samsung E2121B*.
Occasionally, it is tested on:

- Nokia 2630
- Sony Ericsson K550
- Motorola V360

## Docker

A few [docker](https://www.docker.com/) images are available to ease
cross development ([efornara/jbit](https://hub.docker.com/r/efornara/jbit/)):

	main     Development setup for native (and basic documentation)
	midp     Development setup for midp (Oracle WTK still needed)
	droid    Development setup for Android (NDK cross compiler)
	win32    Development setup for Win32 (mingw32 cross compiler)
	dos      Development setup for MS-DOS (djgpp/watcom cross compilers)
	doc      Development setup for extra documentation (sheets and mobi)

A small image is also available for trying out the native version of
jbit:

	busybox  The jbit binary hosted in a busybox environment

Even if you don't use docker, reading the files in
[dists/docker](dists/docker) can give you a few hints on how to setup
your own development environment.

