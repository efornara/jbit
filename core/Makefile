CXX = em++
CXXFLAGS = -std=c++98 -fno-exceptions -fno-rtti -Wall
EXPORTS = EXPORTED_FUNCTIONS="['_vm_reset', '_vm_load_begin', '_vm_load_line', '_vm_load_end', '_vm_step']"

OBJS = js.o vm.o asm.o

OUT = ../js/core.js

all: d_xv65.h

d_xv65.h: xv65.inc
	perl ../tools/inc2sym.pl <xv65.inc >d_xv65.h
	perl ../tools/inc2h.pl <xv65.inc >../native/_xv65.h

$(OUT): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(OUT) $(OBJS) -s $(EXPORTS)

clean:
	$(RM) $(OUT) *.o
