VPATH = .:../core

CXX = em++
CXXFLAGS = -I../core -std=c++98 -fno-exceptions -fno-rtti -O2
EXPORTS = EXPORTED_FUNCTIONS="['_core_parse', '_core_get_prg_data', '_core_get_prg_length', '_core_get_error_lineno', '_core_get_error_colno', '_core_get_error_msg']"

OBJS = js.o asm.o symdefs.o

all: jbit-core.js

jbit-core.js: $(OBJS)
	$(CXX) $(CXXFLAGS) -o jbit-core.js $(OBJS) -s $(EXPORTS)
	$(CXX) $(CXXFLAGS) -o jbit-core-ie.js $(OBJS) -s $(EXPORTS) -s USE_TYPED_ARRAYS=0

fetch:
	mkdir -p libs
	( cd libs; wget -c https://github.com/ajaxorg/ace-builds/archive/v1.1.01.tar.gz )
	( cd libs; tar xzf v1.1.01.tar.gz )
	( cd libs ; mkdir -p ace )
	( cd libs ; ln -sf `pwd`/ace-builds-1.1.01/src-noconflict/ ace/1.1.01 )

copy_res:
	mkdir -p res
	( cd res ; cp ../../samples/6502/*.asm . )

clean:
	$(RM) jbit-core.js jbit-core-ie.js *.o
