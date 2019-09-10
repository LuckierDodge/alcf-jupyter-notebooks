CC=gcc
CFLAGS=-I/home/rlewis/dev/darshan/include
LDFLAGS=-L/home/rlewis/dev/darshan/lib -Wl,-rpath,/home/rlewis/dev/darshan/lib

all: process-log

process-log: main.o 
	$(CC) -o $@ $^ $(LDFLAGS) -ldarshan-util -lz -lbz2

clean::
	rm -f *.o process-log
