CC = gcc
CFLAGS = -DPLATFORM_DESKTOP -Wall
LDFLAGS =
LIBS = -lSDL

OBJS = desktop.o

all: jbnano

jbnano: $(OBJS)
	$(CC) $(LDFLAGS) -o jbnano $(OBJS) $(LIBS)

clean:
	$(RM) *.o jbnano
