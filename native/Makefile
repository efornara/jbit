#
# VARIABLES
#

CXXFLAGS ?= -fno-exceptions -fno-rtti -Wall -Os -fomit-frame-pointer
LDFLAGS ?= -s

SYMS = d_xv65.h d_io2.h d_mio.h d_std.h _jbfmt.h

JBIT_OBJS = main.o std.o random.o cpu.o asm.o passes.o token.o utils.o symdefs.o

IO2SIM_OBJS = io2retro.o io2impl.o layers.o images.o console.o palette.o \
	random.o keybuf.o cpu.o utils.o rom.o

XV65_OBJS = xv65.o keybuf.o

#
# TARGET / PLATFORM DEFAULT
#

ifndef TARGET
TARGET = jbit
endif

ifndef PLATFORM
PLATFORM = posix
endif

#
# TARGET SELECTION
#

ifeq ($(TARGET),jbit)
OBJS = $(JBIT_OBJS)
OUT = jbit
endif

ifeq ($(TARGET),io2sim)
OBJS = $(IO2SIM_OBJS)
LIBS =
endif

#
# PLATFORM SELECTION (jbit)
#

ifeq ($(TARGET),jbit)

ifeq ($(PLATFORM),posix)
OBJS += $(XV65_OBJS)
endif

ifeq ($(PLATFORM),linux)
OBJS += $(XV65_OBJS)
endif

ifeq ($(PLATFORM),osx)
OBJS += $(XV65_OBJS)
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
# OUT
OBJS += $(XV65_OBJS)
OUT = jbit-android-$(ARCH)-$(ELF).bin
endif

ifeq ($(PLATFORM),win32)
CXX = i586-mingw32msvc-g++
LDFLAGS = -mconsole -s
OUT = jbit.exe
endif

ifeq ($(PLATFORM),dosdpmi)
CXX = i586-pc-msdosdjgpp-g++
LDFLAGS = -s
OUT = jbit.exe
endif
ifeq ($(PLATFORM),dos4g)
CXX = wcl386
CXXFLAGS = -q -bt=dos
jbit.exe: $(OBJS)
	wcl386 -q -l=dos4g -fe=jbit $(OBJS)
endif
ifeq ($(PLATFORM),dos16)
CXX = wcl
CXXFLAGS = -q -bt=dos -ml -os
jbit.exe: $(OBJS)
	wcl -q -l=dos -fe=jbit -k8192 $(OBJS)
endif

endif

#
# PLATFORM SELECTION (io2sim)
#

ifeq ($(TARGET),io2sim)

ifeq ($(PLATFORM),dos16)
OBJS += blt08.o
else
OBJS += blt32.o romdata.o
endif

ifeq ($(PLATFORM),osx)
LDFLAGS = -dynamiclib
else
LDFLAGS += -shared
endif

ifeq ($(PLATFORM),posix)
CXXFLAGS += -fPIC
OUT = io2sim.so
endif

ifeq ($(PLATFORM),linux)
CXXFLAGS += -fPIC
OUT = io2sim.so
endif

ifeq ($(PLATFORM),dll64)
CXX = x86_64-w64-mingw32-g++
OUT = io2sim-x86_64.dll
endif

ifeq ($(PLATFORM),dll32)
CXX = i686-w64-mingw32-g++
OUT = io2sim-i686.dll
endif

ifeq ($(PLATFORM),win32)
CXX = i586-mingw32msvc-g++
WINDRES = i586-mingw32msvc-windres
LDFLAGS = -s -mwindows
OBJS += io2win32.o resource.o
LIBS += -lcomctl32 -lopengl32 -lgdi32
OUT = io2sim.exe
endif

ifeq ($(PLATFORM),dos16)
CXX = wcl
CXXFLAGS = -q -0 -bt=dos -ml -os
OBJS += io2dos.o
io2sim.exe: $(OBJS)
	wcl -q -l=dos -fe=io2sim -k4096 $(OBJS)
endif

ifeq ($(PLATFORM),osx)
CXXFLAGS += -fPIC
OUT = io2sim.dylib
endif

endif

#
# LOCAL OVERRIDE
#

include Local.mk

#
# RULES
#

all: $(OUT)

$(OUT): $(OBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $(OUT) $(OBJS) $(LIBS)

gen: jbit.1 BSD.mk $(SYMS)

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

d_std.h: std.inc
	perl ../tools/inc2sym.pl <std.inc >d_std.h
	perl ../tools/inc2h.pl <std.inc >_std.h

_jbfmt.h: jbfmt.inc
	perl ../tools/inc2h.pl <jbfmt.inc >_jbfmt.h

resource.o: resource.rc
	$(WINDRES) resource.rc resource.o

local:
	git update-index --assume-unchanged Local.mk

restore:
	git checkout Local.mk
	git update-index --no-assume-unchanged Local.mk

clean:
	rm -f jbit jbit.exe io2sim.exe *.bin *.dll *.so *.dylib *.o

distclean: clean
	rm -f jbit.1
