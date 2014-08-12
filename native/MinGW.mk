#
# VARIABLES
#

include ../Version.defs

VPATH = .:../core

CC = i586-mingw32msvc-gcc
CXX = i586-mingw32msvc-g++
CFLAGS = -I../core -Wall -O2 -fomit-frame-pointer -DJBIT_VERSION=\"${JBIT_VERSION}\"
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
LDFLAGS = -mwindows -s
LIBS =

OBJS = main.o stdout.o cpu.o asm.o symdefs.o
EXE = jbit.exe

#
# RULES
#

all: $(EXE) jbitw.exe

$(EXE): $(OBJS)
	$(CXX) $(CXXFLAGS) -s -o $(EXE) $(OBJS)

WOBJS = win32.o hwsim_common.o

jbitw.exe: $(WOBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o jbitw.exe $(WOBJS) $(LIBS)

clean:
	$(RM) *.exe *.o
