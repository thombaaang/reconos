CFLAGS=-O3 -g -Wall -m32 -I . -D DEBUG=1
CC=gcc

TARGET=app_s2h
APP_OBJS = app_s2h.o timing.o packetgen.o

all: $(TARGET) 

$(TARGET): $(APP_OBJS)
	$(CC) $(CFLAGS) $(APP_OBJS) -o $(TARGET) -static -lpthread -lrt -lm

%.o: ../%.c
	$(CC) -c $(CFLAGS) -o $@ $<


clean:
	rm -f *.o $(TARGET) 
