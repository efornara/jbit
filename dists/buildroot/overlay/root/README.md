Hello World
-----------

To edit a program, you can use
nano (or the busybox version of
vi):

    nano hello.c

To compile a program, invoke cc
(a link to cl65):

    cc hello.c

To run a program, use jbit:

    jbit hello

The C supported by cc65 is
fairly close to ANSI C. Its
main limitation is the lack of
support for floating point
numbers. More information about
cc65 can be found here:

http://oliverschmidt.github.io/cc65/

Store
-----

IMPORTANT: Every file you
create and every change you
make to the system is lost when
you power off your Raspberry Pi.

To keep a copy of a file you
should copy it to/from the
store. The store is just the
store directory in your SD
card, and, if found during
boot, can be found in
/var/store.

    # before power down
    cp hello.c /var/store

    # after startup
    cp /var/store/hello.c .

Configuration
-------------

The console font and the
keyboard mapping are loaded in
/etc/local.rc. If during boot a
file named local.rc is found in
the store, it is sourced
instead.

To change the keyboard mapping,
first find a keymap that works:

    cd /usr/share/keymaps
    ls
    loadkmap <it.map
    echo '#{}'

Then make the change permanent
by editing a copy of local.rc
and saving it into the store:

    cd /etc
    nano local.rc
    . local.rc
    cp local.rc /var/store

