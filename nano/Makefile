CC = gcc
CFLAGS = -DPLATFORM_PC -Wall
LDFLAGS =
LIBS = -lSDL

EM_CC = emcc
EM_CFLAGS = -DPLATFORM_JS -Wall
EM_LDFLAGS = -s EXPORTED_FUNCTIONS="['_keypad_update', '_sim_init', '_sim_step', '_lcd_get_bitmap']"

OBJS = pc.o sim.o microio.o keypad.o lcd.o hwsim.o fake6502.o
EM_SRCS = sim.c microio.c keypad.c lcd.c hwsim.c fake6502.c

all: jbnano

jbnano: $(OBJS)
	$(CC) $(LDFLAGS) -o jbnano $(OBJS) $(LIBS)

jbit-c.js: $(EM_SRCS)
	$(EM_CC) $(EM_LDFLAGS) $(EM_CFLAGS) -o jbit-c.js $(EM_SRCS)

clean:
	$(RM) -r build-cli *.o jbnano jbit-c.js

arduino:
	make -f Arduino.mk

size: arduino
	avr-size -C --mcu=atmega328p build-cli/nano.elf

full-size: arduino
	avr-nm --size-sort --print-size --radix d --demangle build-cli/nano.elf
