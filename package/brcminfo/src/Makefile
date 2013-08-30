# Makefile for brcminfo utility

%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $<

OBJS = brcminfo.o
all: main

dynamic: all

main: brcminfo.o
	$(CC) $(LDFLAGS) $(EXTRA_CFLAGS) -o brcminfo brcminfo.o -lm

clean:
	rm -f brcminfo ${OBJS}
