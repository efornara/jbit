#
# VARIABLES
#

VPATH = .:../core

CXXFLAGS = -I../core -std=c++98 -fno-exceptions -fno-rtti -Wall
LDFLAGS =
LIBS =

OBJS = main.o devimpl.o cpu.o asm.o symdefs.o
EXE = jbit


#
# COMPILATION FLAGS
#

# development
#CXXFLAGS += -ggdb

# release
CXXFLAGS += -O2 -fomit-frame-pointer
LDFLAGS += -s


#
# DEVICE SELECTION
#

# stdout: minimal dependencies (tested with i586-mingw32msvc and wine)
OBJS += stdout.o

# microio: comment out if you don't have ncurses
#          on BSD replace -lncurses with -lcurses
OBJS += microio.o
LIBS += -lncurses

# xv65: tested only on debian stable; comment out if it doesn't compile
OBJS += xv65.o


#
# RULES
#

all: $(EXE) jbit.1

$(EXE): $(OBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $(EXE) $(OBJS) $(LIBS)

jbit.1: jbit.1.ronn
	ronn --roff jbit.1.ronn

stdout.o: _xv65.h
xv65.o: _xv65.h

clean:
	rm -f jbit jbit.exe windows.zip *.o

distclean: clean
	rm -f jbit.1
