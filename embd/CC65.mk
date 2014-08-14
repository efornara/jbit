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
	pc.o \
	jbit.o \
	vm.o \
	fake6502.o \
	opcodes.o

all: jbembd.jb

jbembd.jb: $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o jbembd.jb $(OBJS) $(LIBS)

clean:
	$(RM) *.o jbembd.jb
