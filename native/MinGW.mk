VPATH = .:../core

CXX = i586-mingw32msvc-g++
CXXFLAGS = -I../core -std=gnu++98 -fno-exceptions -fno-rtti -Wall -O2 -s

OBJS = main.o stdout.o cpu.o asm.o symdefs.o
EXE = jbit.exe

all: $(EXE)

$(EXE): $(OBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $(EXE) $(OBJS) $(LIBS)

clean:
	$(RM) $(EXE) *.o
