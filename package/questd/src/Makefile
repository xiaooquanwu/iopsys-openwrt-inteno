CC		= gcc
CFLAGS		= -g -Wall
LOCLIBS		= 
LIBS		= -luci -lubus -lubox -lpthread
OBJS		= questd.o dumper.o port.o arp.o usb.o
SRCS		= questd.c dumper.c port.c arp.c usb.c
LIBSRCS		= 
ISRCS		= questd.h

all: questd

questd: ${OBJS}
	${CC} ${LDFLAGS} ${LIBSRCS} -o questd ${OBJS} ${LIBS}

clean:
	rm -f questd *.o

