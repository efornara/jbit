COMPILE
=======

## Native

If you have GNU make, the following should work:

	cd native
	make

If you have BSD make, a limited `BSD.mk` is included.

The resulting binary (`jbit`) is self-contained, so you should be able to
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
