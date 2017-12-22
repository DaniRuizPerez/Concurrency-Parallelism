
CC=gcc
CFLAGS=-Wall -pthread -g

PROGS= barber

all: $(PROGS)

%: %.c cola.o
	$(CC) $(CFLAGS) -o $@ $< cola.o

cola.o: cola.c
	gcc -c -g cola.c
clean:
	rm -f $(PROGS) *.o *h.ghc *~


