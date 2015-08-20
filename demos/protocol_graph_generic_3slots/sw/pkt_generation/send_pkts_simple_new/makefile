CC	= gcc
DEBUG   = DEBUG_NONE
CFLAGS	= -O3 -DDEBUG=$(DEBUG) 
LIBS	= -lstdc++ -lncurses

EXECUTABLES = send_pkts_simple

all: $(EXECUTABLES)

send_pkts_simple: send_pkts_simple.o
	$(CC) $^ -o $@ $(CFLAGS) $(LIBS)


clean:
	rm -f *.o $(EXECUTABLES)

distclean: clean

.PHONY: clean
