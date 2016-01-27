#
# VARIABLES
#

include ../Version.defs

CFLAGS = -Wall
CXXFLAGS = -fno-exceptions -fno-rtti -Wall -DJBIT_VERSION=\"${JBIT_VERSION}\"
LDFLAGS =
LIBS =

DEVSYMS = d_xv65.h d_io2.h d_microio.h
OBJS = main.o devimpl.o cpu.o asm.o symdefs.o
EXE = jbit


#
# COMPILATION FLAGS
#

# development - on djgpp/mingw, remove -std=c++98
#CFLAGS += -ggdb
#CXXFLAGS += -ggdb -std=c++98

# release
CFLAGS += -O2 -fomit-frame-pointer
CXXFLAGS += -O2 -fomit-frame-pointer
LDFLAGS += -s


#
# DEVICE SELECTION
#

# stdout: minimal dependencies (tested with i586-mingw32msvc and wine)
OBJS += stdout.o

# microio: comment out if you don't have ncurses
#          on BSD replace -lncurses with -lcurses
#          on DJGPP replace -lncurses with -lpdcurses (very slow)
OBJS += microio.o
LIBS += -lncurses

# xv65: tested mostly on debian stable; comment out if it doesn't compile
OBJS += xv65.o


#
# RULES
#

all: $(EXE) jbit.1

$(EXE): $(DEVSYMS) $(OBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $(EXE) $(OBJS) $(LIBS)

jbit.1: jbit.1.ronn
	ronn --roff jbit.1.ronn

stdout.o: _xv65.h
xv65.o: _xv65.h

d_xv65.h: xv65.inc
	perl ../tools/inc2sym.pl <xv65.inc >d_xv65.h
	perl ../tools/inc2h.pl <xv65.inc >_xv65.h

d_io2.h: io2.inc
	perl ../tools/inc2sym.pl <io2.inc >d_io2.h
	perl ../tools/inc2h.pl <io2.inc >_io2.h

d_microio.h: microio.inc
	perl ../tools/inc2sym.pl <microio.inc >d_microio.h

clean:
	rm -f jbit windows.zip *.o *.exe *.bin

distclean: clean
	rm -f jbit.1
