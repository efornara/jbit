CC = gcc
CFLAGS = -DPLATFORM_PC -Wall
LDFLAGS =
LIBS = -lSDL

EM_CC = emcc
EM_CFLAGS = -DPLATFORM_JS -Wall
EM_LDFLAGS = -s EXPORTED_FUNCTIONS="['_keypad_update', '_jbit_init', '_jbit_step', '_lcd_get_bitmap']"

OBJS = \
	pc.o \
	jbit.o \
	demos.o \
	microio.o \
	vm.o \
	fake6502.o \
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
	ui.c \
	keypad.c \
	lcd.c \
	hwsim.c

all: jbnano

jbnano: $(OBJS)
	$(CC) $(LDFLAGS) -o jbnano $(OBJS) $(LIBS)

jbnano-c.js: $(EM_SRCS)
	$(EM_CC) $(EM_LDFLAGS) $(EM_CFLAGS) -o jbnano-c.js $(EM_SRCS)

%.bin: %.jb
	mkdir -p build-cli
	rm -f build-cli/autorun.o
	cp $< AUTORUN_TMP
	avr-objcopy -I binary -O elf32-avr \
	 --rename-section .data=.progmem.data,contents,alloc,load,readonly,data \
	 --redefine-sym _binary_AUTORUN_TMP_start=autorun_jb \
	 AUTORUN_TMP build-cli/autorun.o
	rm -f AUTORUN_TMP

LOCAL_ARCH = -O elf64-x86-64 --binary-architecture i386
autorun.o: ../midp/jbit/res/ciao.jb
	rm -f autorun.o
	cp $< AUTORUN_TMP
	objcopy -I binary $(LOCAL_ARCH) \
	 --redefine-sym _binary_AUTORUN_TMP_start=autorun_jb \
	 AUTORUN_TMP autorun.o
	rm -f AUTORUN_TMP

clean:
	$(RM) -r build-cli *.o jbnano jbnano-c.*

remote: jbnano
	./jbnano /dev/ttyACM0

uno:
	make -f Arduino.mk

upload: uno
	make -f Arduino.mk upload

size: uno
	avr-size -C --mcu=atmega328p build-cli/nano.elf

full-size: uno
	avr-nm --size-sort --print-size --radix d --demangle build-cli/nano.elf
