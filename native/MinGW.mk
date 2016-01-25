#
# VARIABLES
#

include ../Version.defs

CC = i586-mingw32msvc-gcc
CXX = i586-mingw32msvc-g++
CFLAGS = -Wall -O2 -fomit-frame-pointer -DJBIT_VERSION=\"${JBIT_VERSION}\"
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
LDFLAGS = -s -static-libstdc++ -static-libgcc
LIBS =

OBJS = main.o stdout.o cpu.o asm.o symdefs.o
EXE = jbit.exe

#
# RULES
#

all: $(EXE)

$(EXE): $(OBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -s -o $(EXE) $(OBJS)

clean:
	$(RM) *.exe *.o
