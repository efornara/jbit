VPATH = .:../core

CXX = g++
CXXFLAGS = -I../core -std=c++98 -fno-exceptions -fno-rtti -Wall
LDFLAGS = -s
LIBS = -lncurses

OBJS = main.o microio.o vm.o

all: jbit

jbit: $(OBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o jbit $(OBJS) $(LIBS)

clean:
	$(RM) jbit *.o
