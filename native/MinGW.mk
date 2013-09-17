VPATH = .:../core

CXX = i586-mingw32msvc-g++
CXXFLAGS = -I../core -std=c++98 -fno-exceptions -fno-rtti -Wall -O2 -s

OBJS = main.o tty.o vm.o asm.o
EXE = jbit.exe
PACK = windows.zip

all: $(EXE)

$(EXE): $(OBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $(EXE) $(OBJS) $(LIBS)

clean:
	$(RM) $(EXE) $(PACK) *.o

CC65 = ../../cc65
DIST =	jbit.exe \
	$(CC65)/asminc/xv65.inc \
	$(CC65)/asminc/jbit.inc \
	$(CC65)/include/xv65.h \
	$(CC65)/include/jbit.h \
	$(CC65)/cfg/xv65.cfg \
	$(CC65)/cfg/jbit.cfg \
	$(CC65)/lib/xv65.lib \
	$(CC65)/lib/jbit.lib \
	$(CC65)/libsrc/jbit/header.s

pack: $(EXE)
	$(RM) $(PACK)
	zip -j $(PACK) $(DIST)
