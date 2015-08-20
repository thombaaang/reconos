CFLAGS=-O3 -g -Wall -m32 -I . -D DEBUG=1
CC=gcc

TARGET=app_h2s_trace
APP_OBJS = app_h2s_trace.o timing.o fillbuffer.o

all: $(TARGET) 

$(TARGET): $(APP_OBJS)
	$(CC) $(CFLAGS) $(APP_OBJS) -o $(TARGET) -static -lpthread -lrt -lm

%fillbuffer.o: fillbuffer.c
	$(CC) -c $(CFLAGS) -o $@ $<

%.o: ../%.c
	$(CC) -c $(CFLAGS) -o $@ $<


clean:
	rm -f *.o $(TARGET) 
