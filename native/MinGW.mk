#
# VARIABLES
#

include ../Version.defs

VPATH = .:../core

CXX = i586-mingw32msvc-g++
CXXFLAGS = -I../core -std=gnu++98 -fno-exceptions -fno-rtti -Wall -O2 -fomit-frame-pointer -s -DJBIT_VERSION=\"${JBIT_VERSION}\"
LDFLAGS = -mwindows
LIBS =

OBJS = main.o stdout.o cpu.o asm.o symdefs.o
EXE = jbit.exe

#
# RULES
#

all: $(EXE) jbitw.exe

$(EXE): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(EXE) $(OBJS)

jbitw.exe: win32.o
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o jbitw.exe win32.o $(LIBS)

clean:
	$(RM) *.exe *.o
