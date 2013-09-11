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

clean:
	$(RM) jbit *.o
