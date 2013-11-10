## buildroot

At this stage the process is not automated.
Familiarity with buildroot is assumed.

Tested with [buildroot-2013.08.1]
(http://buildroot.uclibc.org/downloads/buildroot-2013.08.1.tar.bz2).

Here are the steps:

1.  Import the included config file and overlay directory.
2.  Run `make xconfig` (or equivalent) and check the configuration.
3.  Compile the jbit fork of cc65 as usual and copy asminc, include, cfg
    and lib/xv65.lib into overlay/usr/lib/cc65.
    They are platform independent.
4.  Run `make` to build the toolchain.
5.  Re-compile the cc65 binaries using the cross-compiler, for example:
    `cd $CC65/src ; make clean ; CC=arm-linux-gcc make`.
6.  Copy $CC65/bin/* into overlay/usr/bin.
7.  Compile the native version of jbit using the cross-compiler, for example:
    `cd $JBIT/native ; make clean ; CXX=arm-linux-g++ make`. 
8.  Copy $JBIT/native/jbit into overlay/usr/bin.
9.  Retrieve Lat15-TerminusBold28x14.psf (plus any additional fonts you
    might need) from /usr/share/consolefonts and copy it into
    overlay/usr/share/consolefonts.
10. Generate a few busybox kmaps. For example, to generate an italian kmap
    from a uk keyboard on a debian system, type something like this:
    `cd /usr/share/keymaps/i386/qwerty ; loadkeys it.kmap.gz ; busybox dumpkmap >/tmp/it.map ; loadkeys uk.kmap.gz`
    and copy the result into overlay/usr/share/keymaps.
11. Re-run `make` to build the final image. Note that the kernel and
    the firmware are not downloaded / generated.
