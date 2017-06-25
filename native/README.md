NATIVE
======

The native version of JBit is written in old-style C++. It targets
the subset of C++98 excluding exceptions and run time information,
and compatible with Open Watcom.

The build system is non-standard, uses GNU make and it is driven by
two environment variables: `TARGET` (to select jbit or io2sim) and
`PLATFORM` (to select among the supported platforms).

If you are building on a Unix-like system and you target the same
machine, you shouldn't need any additional dependencies beside a C++
compiler. Otherwise, see the files in `../dists/docker` for hints on
what you might need to install.

On a typical Linux system, this should work:

	make

The Makefile is not particularly clever, so when you change `PLATFORM`
or `TARGET`, you should manually clean the working directory. For
example, to generate both the executables for Windows, type:

	make clean
	make TARGET=jbit PLATFORM=win32
	cp jbit.exe <somewhere>
	make clean
	make TARGET=io2sim PLATFORM=win32
	cp io2sim.exe <somewhere>

A limited (i.e. without support for compiling for other platforms)
`BSD.mk` is also available for BSD systems without GNU make installed.
For example, this should work on a bare OpenBSD installation:

	make -f BSD.mk clean
	make -f BSD.mk TARGET=jbit

## TARGET jbit

Available `PLATFORM`s:

	std       Generic C++ system (no xv65 device)
	posix     Generic Unix(-like) system (with xv65 device)
	linux     Linux (same as posix)
	osx       Mac OS X
	android   Android NDK
	win32     Windows 95+
	dosdpmi   32-bit MS-DOS (DJGPP compiler)
	dos4g     32-bit MS-DOS (Open Watcom compiler)
	dos16     16-bit MS-DOS (Open Watcom compiler)

For android, see also `../dists/docker/droid` and `../dists/android`.

The resulting binary (`jbit`) is self-contained, so you should be able to
run it from anywhere.  Assuming that you have it in your path, if you
switch to the sample directory and type:

	jbit hello.asm

You should get back the usual message:

	Hello, World!

JBit has also a binary format:

	jbit -c jb hello.jb hello.asm
	jbit hello.jb

More information is available in the included jbit(1).

## io2sim

Available `PLATFORM`s:

	posix     libretro - Generic Unix(-like) system
	linux     libretro - Linux (tested on RetroArch)
	osx       libretro - Mac OS X (untested)
	dll64     libretro - Windows 64-bit (tested on Windows 10 / RetroArch)
	dll32     libretro - Windows 32-bit (untested)
	win32     Self-contained 32-bit Windows executable
	dos16     Self-contained 16-bit MS-DOS executable (Open Watcom compiler)

Here is an example on how you might use the resulting libretro core:

	retroarch -L io2sim.so ../samples/extra/pngtest.jb
