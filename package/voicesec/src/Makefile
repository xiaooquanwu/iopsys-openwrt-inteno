# Makefile for voicesec utility

%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $<

OBJS = voicesec.o
all: main

dynamic: all

main: voicesec.o
	$(CC) $(LDFLAGS) $(EXTRA_CFLAGS) -o voicesec voicesec.o -lm

clean:
	rm -f voicesec ${OBJS}
