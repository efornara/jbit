JBit
====

This is a C++/JavaScript version of the JBit Virtual Machine.  It has a
core written in C++, and a two simulators using it: a native one using
it directly, and a JavaScript one using it via
[emscripten](https://github.com/kripken/emscripten).

JBit is a 6502 simulator and development environment. It targets feature
phones, but this version, targeting desktops and browsers is also
available. You can see an old version of this code running here
(browse the gallery and click on *sim* to run the sample on your browser
and *jb* to download it to run it on the native simulator):

[JBit MicroIO Gallery](http://jbit.sourceforge.net/gallery/list.php?tag=microio)

Native
------

To compile the native version of the simulator, beside the usual gcc/g++
toolchain, you need the development versions of ncurses. On debian, this
means having the libncurses5-dev package installed.

The following should work (tested on debian stable):

	cd native
	make

The simulator is self-contained and does not required an installation;
just copy the executable wherever you want.

To run the simulator, type something like this:

	jbit charset.jb

For the charset example, press 2 and 8 to scroll and 0 to halt the
program (you then have to press : or Ctrl+C to end the simulator).

If you want to try writing your own programs, get my fork of
[cc65](http://oliverschmidt.github.io/cc65/) here:

<https://github.com/efornara/cc65>

JavaScript
----------

To compile the javascript version of the simulator, you need
[emscripten](https://github.com/kripken/emscripten). Once you have
checked that your emscripten installation works, generate js/core.js by
typing:

	cd core
	make

You can then try out the simulator by copying the js directory into a
web server.

More info
---------

<http://jbit.sf.net> (XHTML site, optimized for feature phones)

or

<http://jbit.sf.net/m> (WAP site, for old phones / very low bandwidth)

