VPATH = .:../core

CXX = g++
CXXFLAGS = -I../core -std=c++98 -fno-exceptions -fno-rtti -Wall -ggdb
LDFLAGS = -ggdb
LIBS = -lncurses

OBJS = main.o microio.o xv65.o vm.o asm.o

all: jbit

jbit: $(OBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o jbit $(OBJS) $(LIBS)

clean:
	$(RM) jbit *.o
