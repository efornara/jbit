CXX=em++
CXXFLAGS=-std=c++98 -fno-exceptions -fno-rtti -Wall
EXPORTS=EXPORTED_FUNCTIONS="['_vm_load', '_vm_step']"

OUT=../js/res/vm.js

all: $(OUT)

$(OUT): js.cpp
	$(CXX) $(CXXFLAGS) -o $(OUT) js.cpp -s $(EXPORTS)

clean:
	$(RM) $(OUT)
