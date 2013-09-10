CXX = em++
CXXFLAGS = -std=c++98 -fno-exceptions -fno-rtti -Wall
EXPORTS = EXPORTED_FUNCTIONS="['_vm_reset', '_vm_put', '_vm_step']"

OBJS = js.o vm.o

OUT = ../js/core.js

all: $(OUT)

$(OUT): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(OUT) $(OBJS) -s $(EXPORTS)

clean:
	$(RM) $(OUT) *.o
