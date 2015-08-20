# this makefile is can be used to compile for both desktop and microblaze
# code segments which use reconos, or microblaze specific libraries are only activated when compiled for microblaze 
# (activation with _MICROBLAZE_ constant)

DT_CFLAGS=-O3 -g
DT_CC=gcc
MB_CFLAGS=-O3 -g -Wall -D_MICROBLAZE_ -I ../../../../linux/reconos/libreconos-us
MB_CC=microblaze-unknown-linux-gnu-gcc

TARGET=generic_fb
DT_OBJS = generic_fb_dt.o args_from_file_dt.o # desktop objects
MB_OBJS = generic_fb_mb.o args_from_file_mb.o timing_mb.o # microblaze objects

# compile for desktop
dt: $(DT_OBJS)
	$(DT_CC) $(DT_CFLAGS) -o $(TARGET) -static -pthread $(DT_OBJS) -lrt -lm

# comile for microblaze
mb: $(MB_OBJS)
	$(MB_CC) $(MB_CFLAGS) -L ../../../../linux/reconos/libreconos-us $(MB_OBJS) -o $(TARGET) -static -lreconos -pthread -lrt -lm

%_dt.o: %.c %.h
	$(DT_CC) -c $(DT_CFLAGS) -o $@ $<

%_mb.o: %.c %.h
	$(MB_CC) -c $(MB_CFLAGS) -o $@ $<

clean:
	rm -f *.o $(TARGET) 
