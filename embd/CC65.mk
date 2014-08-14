CC = cl65
CFLAGS = -txv65 -DPLATFORM_CC65
LDFLAGS =
LIBS =

OBJS = \
	jbit.o \
	vm.o \
	fake6502.o

all: jbembd.jb

jbembd.jb: $(OBJS)
	$(CC) $(LDFLAGS) -o jbembd.jb $(OBJS) $(LIBS)

clean:
	$(RM) *.o jbembd.jb
