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
	sim.o \
	fake6502.o \
	ui.o \
	keypad.o \
	lcd.o \
	hwsim.o

EM_SRCS = \
	jbit.c \
	demos.c \
	microio.c \
	sim.c \
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

clean:
	$(RM) -r build-cli *.o jbnano jbnano-c.js

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
