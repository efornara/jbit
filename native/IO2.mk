#
# VARIABLES
#

CXXFLAGS ?= -fno-exceptions -fno-rtti -Wall -Os -fomit-frame-pointer
LDFLAGS = -s -shared

OBJS = io2retro.o utils.o
LIBS =

#
# PLATFORM SELECTION
#

ifndef PLATFORM
PLATFORM = posix
endif

ifeq ($(PLATFORM),posix)
CXXFLAGS += -fPIC
TARGET = jbitio2.so
endif

ifeq ($(PLATFORM),dll64)
CXX = x86_64-w64-mingw32-g++
TARGET = jbitio2.dll
endif

#
# RULES
#

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(LDFLAGS) -o $(TARGET) $(OBJS) $(LIBS)
