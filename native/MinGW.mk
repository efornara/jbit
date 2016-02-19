#
# VARIABLES
#

CC = i586-mingw32msvc-gcc
CXX = i586-mingw32msvc-g++
CFLAGS = -Wall -Os -fomit-frame-pointer
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ifdef MINGW_STATIC_RUNTIME
LDFLAGS = -s -static-libstdc++ -static-libgcc
else
LDFLAGS = -s
endif
LIBS =

OBJS = main.o stdout.o cpu.o asm.o symdefs.o
EXE = jbit.exe

#
# RULES
#

all: $(EXE)

$(EXE): $(OBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $(EXE) $(OBJS) $(LIBS)

clean:
	$(RM) *.exe *.o
