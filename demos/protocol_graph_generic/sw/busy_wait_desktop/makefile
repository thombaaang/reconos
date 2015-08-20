CFLAGS=-O3 -g
CC=gcc

TARGET=busy_wait
APP_OBJS = busy_wait.o

all: $(TARGET) 

$(TARGET): $(APP_OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) -static -lpthread -lrt -lm $(APP_OBJS)

%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $<


clean:
	rm -f *.o $(TARGET) 
