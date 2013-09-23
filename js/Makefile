VPATH = .:../core

CXX = em++
CXXFLAGS = -I../core -std=c++98 -fno-exceptions -fno-rtti -O2
EXPORTS = EXPORTED_FUNCTIONS="['_core_parse', '_core_get_prg_data', '_core_get_prg_length', '_core_get_error_lineno', '_core_get_error_colno', '_core_get_error_msg']"

OBJS = js.o asm.o symdefs.o

OUT = jbit-core.js

all: $(OUT)

$(OUT): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(OUT) $(OBJS) -s $(EXPORTS)

clean:
	$(RM) $(OUT) *.o
