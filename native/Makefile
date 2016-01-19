#
# VARIABLES
#

include ../Version.defs

# on std. make, replace space separator with : (or ;)
VPATH = . ../core

CFLAGS = -Wall
CXXFLAGS = -I../core -fno-exceptions -fno-rtti -Wall -DJBIT_VERSION=\"${JBIT_VERSION}\"
LDFLAGS =
LIBS =

OBJS = main.o devimpl.o cpu.o asm.o symdefs.o
EXE = jbit


#
# COMPILATION FLAGS
#

# development - on djgpp/mingw, remove -std=c++98
#CFLAGS += -ggdb
#CXXFLAGS += -ggdb -std=c++98

# android
#LDFLAGS += -fPIE -pie
#CXXFLAGS += -fPIE

# release
CFLAGS += -O2 -fomit-frame-pointer
CXXFLAGS += -O2 -fomit-frame-pointer
LDFLAGS += -s


#
# DEVICE SELECTION
#

# stdout: minimal dependencies (tested with i586-mingw32msvc and wine)
OBJS += stdout.o

# microio: comment out if you don't have ncurses
#          on BSD replace -lncurses with -lcurses
#          on DJGPP replace -lncurses with -lpdcurses (very slow)
OBJS += microio.o
LIBS += -lncurses

# xv65: tested mostly on debian stable; comment out if it doesn't compile
OBJS += xv65.o

# serial: tested only on debian stable; comment out if it doesn't compile
CXXFLAGS += -DENABLE_SENDFILE
OBJS += send_file.o serial.o


#
# RULES
#

all: $(EXE) jbit.1

$(EXE): $(OBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $(EXE) $(OBJS) $(LIBS)

jbit.1: jbit.1.ronn
	ronn --roff jbit.1.ronn

stdout.o: _xv65.h
xv65.o: _xv65.h

clean:
	rm -f jbit windows.zip *.o *.exe

distclean: clean
	rm -f jbit.1
