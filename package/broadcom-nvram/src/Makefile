# Makefile for broadcom nvram to uci wrapper


%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $<

OBJS = nvram_emu_lib.o ucix.o main.o

all:  libnvram main

dynamic: all

libnvram: nvram_emu_lib.o
	$(CC) -c $(CFLAGS) $(LDFLAGS) -fPIC -o ucix_shared.o ucix.c -luci
	$(CC) -c $(CFLAGS) $(LDFLAGS) -fPIC -o nvram_emu_lib.o nvram_emu_lib.c -luci
	$(CC) $(LDFLAGS) -shared -Wl,-soname,libnvram.so -o libnvram.so   nvram_emu_lib.o ucix_shared.o -luci

main: main.o ucix.o
	$(CC) $(LDFLAGS) -o uci_test main.o ucix.o -luci

clean:
	rm -f libnvram.so ucix_shared.o uci_test ${OBJS}

