#
# VARIABLES
#

VPATH = .:../core

CXX = g++
CXXFLAGS = -I../core -std=c++98 -fno-exceptions -fno-rtti -Wall -ggdb
LDFLAGS =
LIBS =

OBJS = main.o devimpl.o vm.o asm.o
EXE = jbit


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

all: $(EXE)

$(EXE): $(OBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $(EXE) $(OBJS) $(LIBS)

stdout.o: _xv65.h
xv65.o: _xv65.h

_xv65.h: ../core/xv65.inc
	perl ../tools/inc2h.pl <../core/xv65.inc >_xv65.h

clean:
	rm -f jbit jbit.exe windows.zip *.o
