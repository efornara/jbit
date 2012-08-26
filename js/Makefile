CC = gcc
CXX = g++
CFLAGS = -Wall -O2 -fomit-frame-pointer
CXXFLAGS = $(CFLAGS)
LDFLAGS = -s
LIBS = -lncurses -lv8 -ldl

OBJS = main.o s_curses.o dynsdl.o frame.o j_v8.o

all: $(OBJS)
	$(CXX) $(LDFLAGS) -o jbit $(OBJS) $(LIBS)

clean:
	rm -f jbit out.ppm $(OBJS)
