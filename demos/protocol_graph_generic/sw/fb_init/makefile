CFLAGS=-O3 -g -Wall -I ../../../../linux/reconos/libreconos-us
CC=microblaze-unknown-linux-gnu-gcc

TARGET=fb_init
APP_OBJS = fb_init.o timing.o

all: $(TARGET) 

$(TARGET): $(APP_OBJS)
	$(CC) $(CFLAGS) -L ../../../../linux/reconos/libreconos-us $(APP_OBJS) -o $(TARGET) -static -lreconos -lpthread -lrt -lm

%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $<


clean:
	rm -f *.o $(TARGET) 
