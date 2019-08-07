CC=gcc
CFLAGS=-I/soft/perftools/darshan/darshan-2.3.1/include
LDFLAGS=-L/soft/perftools/darshan/darshan-2.3.1/lib -Wl,-rpath,/soft/perftools/darshan/darshan-2.3.1/lib

all: process-log

process-log: main.o 
	$(CC) -o $@ $^ $(LDFLAGS) -ldarshan-util -lz -lbz2

clean::
	rm -f *.o process-log
