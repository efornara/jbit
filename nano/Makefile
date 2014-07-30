CC = gcc
CFLAGS = -ansi -DPLATFORM_DESKTOP -Wall
LDFLAGS =
LIBS = -lSDL

OBJS = desktop.o sim.o lcd.o hwsim.o

all: jbnano

jbnano: $(OBJS)
	$(CC) $(LDFLAGS) -o jbnano $(OBJS) $(LIBS)

clean:
	$(RM) *.o jbnano
