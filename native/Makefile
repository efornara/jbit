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

# simple: minimal dependencies (tested with i586-mingw32msvc and wine)
OBJS += simple.o

# microio: comment out if you don't have ncurses
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

simple.o: xv65.h
xv65.o: xv65.h

xv65.h: xv65.inc
	perl ../tools/inc2h.pl <xv65.inc >xv65.h

test: $(EXE)
	./$(EXE) ../samples/microio/loop1.jb
	./$(EXE) ../samples/xv65/hello.jb

clean:
	$(RM) jbit jbit.exe windows.zip *.o
