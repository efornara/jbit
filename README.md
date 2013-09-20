JBit
====

If you are looking for the mobile (J2ME) version of JBit, you can find
it here:

<http://jbit.sourceforge.net> (XHTML site, optimized for feature phones)

or here:

<http://jbit.sourceforge.net/m> (WAP site, for old phones / very low bandwidth)

This version of JBit targets desktops and browsers.

Compiling
---------

The following should work (tested mostly on debian, but occasionally on
NetBSD):

	cd native
	make

If you are missing ncurses (or have curses), comment out the offending
lines in the Makefile (or edit them). The (n)curses library is needed
for the microio device, and, unless you are coming from the mobile
version of JBit, you are unlikely to need it.

A precompiled EXE for Windows is also available (see the wiki below).

Testing
-------

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

Getting Started
---------------

You can find plenty of information on the
[6502](http://en.wikipedia.org/wiki/MOS_Technology_6502) on the
Internet.  There is a also a
[Beginner's Tutorial](http://jbit.sourceforge.net//doc/tutorial_en.html)
on how to start 6502 programming with JBit (J2ME), but here I assume you
are familiar with 6502 assembly and new to JBit.

The CPU in JBit is pretty similar to a regular 6502. The main difference
is that it lacks BCD mode and interrupts. Code is loaded starting from
page 3 and a "IO chip" is mapped to page 2. BRK (0) terminates the
program.

Type and run:

	$a9 'A' $8d $00 $02

Register $0200 is PUTCHAR and when you write a character into it,
that character is sent to stdout.

Since its roots in feature phones, the prefered notation in JBit is
*decimal*:

	169 'A' 141 0 2

To work around clean numbers ($C000) becoming magic numbers (49152),
JBit uses the non-standard notation *page:offset* for decimal words:

	169 'A' 141 2:0
	
You don't need to remember the location of the IO registers. If you
select a device, you get some constants defined:

	.device "stdout"
	169 'A' 141 PUTCHAR

You can of course write in assembly instead:

	.device "stdout"
	lda #'A'
	sta PUTCHAR

Devices
-------

Different devices are available (all mapped to page 2). Type:

	jbit -d ?
	jbit -s ?

The first command shows which devices can be simulated, and the second
one shows for which devices the assembler can import symbols.

### stdout

The stdout device is very simple and is the only device that can also be
simulated on Windows (you can still develop for all the other devices).

To see which symbols are defined for a device, run:

	jbit -s stdout

You should get:

	PUTCHAR 512
	FRMFPS 529
	FRMDRAW 530
	PUTUINT8 534

You know PUTCHAR. PUTUINT8 prints a decimal number (0-255). Writing
anything into FRMDRAW calls fflush(stdout). If FRMFPS is 0 (the
default), FRMDRAW doesn't wait. If FRMFPS is != 0, FRMDRAW waits to keep
a "refresh rate" of FRMFPS / 4 frames per seconds. For example, this
program:

	.device "stdout"
	      lda #4
	      sta FRMFPS
	      lda #'X'
	loop: sta PUTCHAR
	      sta FRMDRAW
	      jmp loop

keeps printing one X every second, until you press Ctrl-C.

More information about the assembly format is in the samples/format
directory.

### xv65

The device xv65 maps an extended subset of the traditonal Unix V6 API
(fork, exec, pipe, dup, write etc...) and it was inspired by the
beautiful [xv6](http://pdos.csail.mit.edu/6.828/2012/xv6.html).

xv65 is quite a complex device. For an example of use, look at
xtermpal.asm in samples. xtermpal just prints out some escape characters
to produce a color palette, and could have been written for the stdout
device.  However, since sending escape characters might confuse other
terminals, xtermpal uses the ENV request to query the environment
variable TERM and guard against running on a non-xterm terminal.

### microio

A couple of jb files for the microio device are included. Since the jb
format does not include device information, you need to tell jbit to use
the microio device when you start it:

	jbit -d microio charset.jb

### io2

Together with xv65, the other major device supported by JBit is io2.  It
cannot be simulated by jbit, but you can still use jbit to write
programs for it. If you have [java](http://www.java.com) installed, you
can run the J2ME version of JBit using
[microemulator](http://www.microemu.org/) to test them.

I assume every file is placed in a new directory and you are working in
it.

1. Get a io2 sample file (for example, `smile.asm` in the samples
directory).

2. Download `microemulator-2.0.4.zip` from here: <http://code.google.com/p/microemu/downloads/list>

3. Extract `microemulator.jar` from the archive.

4. Check that it runs fine and then close it (the default device is fine):

	`java -jar microemulator.jar`

5. Download `JBit2_microemulator.zip`:
<http://sourceforge.net/projects/jbit/files/jbit/Resources/JBit2_microemulator.zip/download>.

6. Extract the content of the archive (`JBit2_me.jad` and
`JBit2_me.jar`).

7. Convert the assembly source to the *jb* binary format. The resulting file
*must* be named `out.jb` for this setup to work. If everything goes well,
the command is silent. Conversion errors are sent to stderr.

	`jbit -c jb smile.asm >out.jb`

8. The directory should now look like this:

	`JBit2_me.jad`
	`JBit2_me.jar`
	`microemulator.jar`
	`out.jb`
	`smile.asm`

9. Execute the following command (on Windows replace `:` with `;`):

	`java -jar microemulator.jar --appclasspath JBit2_me.jar:.  --propertiesjad JBit2_me.jad JBit`

10. If you press a menu button (one of the two big buttons on either side
of the joypad), you can stop the program, select *Menu* and then select
*Debug* to debug the program.

JavaScript
----------

The JavaScript version uses
[emscripten](https://github.com/kripken/emscripten), but it's not
working at the moment. You can see an older version running here:

<http://jbit.sourceforge.net/gallery/list.php?tag=microio>

Browse the gallery and click on *sim* to run the sample on your browser
and *jb* to download it to run it on the native simulator.
