
CC=gcc
CFLAGS=-Wall -pthread -g

PROGS= producerconsumer

all: $(PROGS)

% : %.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(PROGS) *.o *~

