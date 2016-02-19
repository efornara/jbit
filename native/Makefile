#
# VARIABLES
#

CXXFLAGS ?= -fno-exceptions -fno-rtti -Wall -Os -fomit-frame-pointer
LDFLAGS ?= -s

DEVSYMS = d_xv65.h d_io2.h d_microio.h
OBJS = main.o devimpl.o cpu.o asm.o symdefs.o
EXE = jbit

#
# PLATFORM SELECTION
#

ifndef PLATFORM
PLATFORM = posix
endif

ifeq ($(PLATFORM),std)
OBJS += stdout.o
endif

ifeq ($(PLATFORM),posix)
OBJS += stdout.o xv65.o
endif

ifeq ($(PLATFORM),linux)
OBJS += stdout.o xv65.o
endif

ifeq ($(PLATFORM),osx)
OBJS += stdout.o xv65.o
LDFLAGS =
endif

ifeq ($(PLATFORM),android)
# ARCH
ifeq ($(ARCH),arm)
CXX = arm-linux-androideabi-g++
endif
ifeq ($(ARCH),x86)
CXX = i686-linux-android-g++
endif
ifeq ($(ARCH),mips)
CXX = mipsel-linux-android-g++
endif
# ELF
ifeq ($(ELF),pie)
CXXFLAGS += -fPIE
LDFLAGS += -fPIE -pie
endif
ifeq ($(ELF),static)
LDFLAGS += -static
endif
# EXE
EXE = jbit-android-$(ARCH)-$(ELF).bin
endif

ifeq ($(PLATFORM),win32)
CXX = i586-mingw32msvc-g++
LDFLAGS = -mconsole -s
OBJS += stdout.o
EXE = jbit.exe
endif

ifeq ($(PLATFORM),dos)
CXX = i586-pc-msdosdjgpp-g++
LDFLAGS = -s
OBJS += stdout.o
EXE = jbit.exe
endif

ifeq ($(PLATFORM),local)
include Local.mk
endif

#
# RULES
#

all: $(EXE) jbit.1

$(EXE): $(DEVSYMS) $(OBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $(EXE) $(OBJS) $(LIBS)

jbit.1: jbit.1.ronn
	ronn --roff jbit.1.ronn

stdout.o: _xv65.h
xv65.o: _xv65.h

d_xv65.h: xv65.inc
	perl ../tools/inc2sym.pl <xv65.inc >d_xv65.h
	perl ../tools/inc2h.pl <xv65.inc >_xv65.h

d_io2.h: io2.inc
	perl ../tools/inc2sym.pl <io2.inc >d_io2.h
	perl ../tools/inc2h.pl <io2.inc >_io2.h

d_microio.h: microio.inc
	perl ../tools/inc2sym.pl <microio.inc >d_microio.h

clean:
	rm -f jbit windows.zip *.o *.exe *.bin

distclean: clean
	rm -f jbit.1
