CC = cl65
CFLAGS = -DPLATFORM_PC
LDFLAGS =
LIBS =

ifdef TARGET
CFLAGS += -t$(TARGET)
else
CFLAGS += -txv65
endif

OBJS = \
	jbit.o \
	vm.o \
	pc.o \
	fake6502.o

all: jbembd.jb

jbembd.jb: $(OBJS)
	$(CC) $(LDFLAGS) -o jbembd.jb $(OBJS) $(LIBS)

clean:
	$(RM) *.o jbembd.jb
