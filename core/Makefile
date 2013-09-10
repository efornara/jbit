CXX=em++
CXXFLAGS=-std=c++98 -fno-exceptions -fno-rtti -Wall
EXPORTS=EXPORTED_FUNCTIONS="['_vm_load', '_vm_step']"

all: out.js

out.js: js.cpp test.js
	$(CXX) $(CXXFLAGS) js.cpp -s $(EXPORTS)
	cat a.out.js test.js >out.js

run: out.js
	node out.js

clean:
	$(RM) *out*.js
