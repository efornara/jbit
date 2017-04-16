#
# VARIABLES
#

CXXFLAGS ?= -fno-exceptions -fno-rtti -Wall -Os -fomit-frame-pointer
LDFLAGS ?= -s

DEVSYMS = d_xv65.h d_io2.h d_mio.h
OBJS = main.o devimpl.o cpu.o asm.o utils.o symdefs.o
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
OBJS += stdout.o xv65.o
EXE = jbit-android-$(ARCH)-$(ELF).bin
endif

ifeq ($(PLATFORM),win32)
CXX = i586-mingw32msvc-g++
LDFLAGS = -mconsole -s
OBJS += stdout.o
EXE = jbit.exe
endif

ifeq ($(PLATFORM),dos)
ifeq ($(ARCH),dpmi)
CXX = i586-pc-msdosdjgpp-g++
LDFLAGS = -s
OBJS += stdout.o mio.o miodos.o
EXE = jbit.exe
endif
ifeq ($(ARCH),dos4g)
CXX = wcl386
CXXFLAGS = -bt=dos
OBJS = main.o stdout.o cpu.o asm.o utils.o symdefs.o
jbit.exe: $(OBJS)
	wcl386 -l=dos4g -fe=jbit $(OBJS)
endif
ifeq ($(ARCH),dos16)
# broken target: large segments, 16-bit ints, stack overflow, etc...
CXX = wcl
CXXFLAGS = -bt=dos -ml -os
OBJS = main.o stdout.o cpu.o asm.o utils.o symdefs.o
jbit.exe: $(OBJS)
	wcl -l=dos -fe=jbit $(OBJS)
endif
endif

ifeq ($(PLATFORM),local)
include Local.mk
endif

#
# RULES
#

all: $(EXE) jbit.1 BSD.mk

$(EXE): $(DEVSYMS) $(OBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $(EXE) $(OBJS) $(LIBS)

jbit.1: jbit.ron
	ronn <jbit.ron >jbit.1

BSD.mk: Makefile
	perl ../tools/genbsd.pl <Makefile >BSD.mk

stdout.o: _xv65.h
xv65.o: _xv65.h

d_xv65.h: xv65.inc
	perl ../tools/inc2sym.pl <xv65.inc >d_xv65.h
	perl ../tools/inc2h.pl <xv65.inc >_xv65.h

d_io2.h: io2.inc
	perl ../tools/inc2sym.pl <io2.inc >d_io2.h
	perl ../tools/inc2h.pl <io2.inc >_io2.h

d_mio.h: mio.inc
	perl ../tools/inc2sym.pl <mio.inc >d_mio.h

touch:
	touch jbit.1 BSD.mk *.h

clean:
	rm -f jbit jbit*.exe *.bin *.dll *.so *.o

distclean: clean
	rm -f jbit.1
