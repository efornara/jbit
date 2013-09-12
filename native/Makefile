VPATH = .:../core

CXX = g++
CXXFLAGS = -I../core -std=c++98 -fno-exceptions -fno-rtti -Wall -ggdb
LDFLAGS =
LIBS =

OBJS = main.o xv65.o vm.o asm.o

# comment out if you don't have ncurses
LIBS += -lncurses
OBJS += microio.o

all: jbit

jbit: $(OBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o jbit $(OBJS) $(LIBS)

xv65.o: xv65.h
xv65.h: xv65.inc
	perl ../tools/inc2h.pl <xv65.inc >xv65.h

test: jbit
	./jbit ../samples/microio/loop1.jb
	./jbit ../samples/xv65/hello.jb

clean:
	$(RM) jbit *.o
