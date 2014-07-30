CC = gcc
CFLAGS = -ansi -DPLATFORM_DESKTOP -Wall
LDFLAGS =
LIBS = -lSDL

EM_CC = emcc
EM_CFLAGS = -ansi -DPLATFORM_JS -Wall
EM_LDFLAGS = -s EXPORTED_FUNCTIONS="['_keypad_update', '_sim_init', '_sim_step', '_lcd_get_bitmap']"

OBJS = desktop.o sim.o lcd.o hwsim.o
EM_SRCS = sim.c lcd.c hwsim.c

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
	avr-size build-cli/nano.elf

full-size: arduino
	avr-nm --size-sort --print-size --radix d --demangle build-cli/nano.elf
