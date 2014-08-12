CC = gcc
CFLAGS = -DPLATFORM_PC -Wall -ggdb
LDFLAGS = -ggdb
LIBS =

# enable SDL
#CFLAGS += -DPLATFORM_PC_SDL
#LIBS += -lSDL

EM_CC = emcc
EM_CFLAGS = -DPLATFORM_JS -Wall -O2
EM_LDFLAGS = -s EXPORTED_FUNCTIONS="['_keypad_update', '_jbit_init', '_jbit_step', '_lcd_get_bitmap']"

OBJS = \
	pc.o \
	jbit.o \
	demos.o \
	microio.o \
	vm.o \
	fake6502.o \
	opcodes.o \
	ui.o \
	keypad.o \
	lcd.o \
	hwsim.o

EM_SRCS = \
	jbit.c \
	demos.c \
	microio.c \
	vm.c \
	fake6502.c \
	opcodes.c \
	ui.c \
	keypad.c \
	lcd.c \
	hwsim.c

all: jbembd

jbembd: $(OBJS)
	$(CC) $(LDFLAGS) -o jbembd $(OBJS) $(LIBS)

jbit-embd-c.js: $(EM_SRCS)
	$(EM_CC) $(EM_LDFLAGS) $(EM_CFLAGS) -o jbit-embd-c.js $(EM_SRCS)

%.bin: %.jb
	mkdir -p build-cli
	rm -f build-cli/autorun.o
	cp $< AUTORUN_TMP
	avr-objcopy -I binary -O elf32-avr \
	 --rename-section .data=.progmem.data,contents,alloc,load,readonly,data \
	 --redefine-sym _binary_AUTORUN_TMP_start=autorun_jb \
	 AUTORUN_TMP build-cli/autorun.o
	rm -f AUTORUN_TMP

%.bin: %.rom
	mkdir -p build-cli
	rm -f build-cli/rom.o
	cp $< ROM_TMP
	avr-objcopy -I binary -O elf32-avr \
	 --rename-section .data=.progmem.data,contents,alloc,load,readonly,data \
	 --redefine-sym _binary_ROM_TMP_start=rom_start \
	 --redefine-sym _binary_ROM_TMP_size=rom_size \
	 ROM_TMP build-cli/rom.o
	rm -f ROM_TMP

clean:
	$(RM) -r build-cli *.o jbembd jbit-embd-c.*

remote: jbembd
	./jbembd /dev/ttyACM0

uno:
	make -f Arduino.mk

upload: uno
	make -f Arduino.mk upload

size: uno
	avr-size -C --mcu=atmega328p build-cli/embd.elf

full-size: uno
	avr-nm --size-sort --print-size --radix d --demangle build-cli/embd.elf
