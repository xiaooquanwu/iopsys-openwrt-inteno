CC		= gcc
CFLAGS		= -g -Wall
LOCLIBS		= 
LIBS		= -luci -lubus -lubox -lpthread
OBJS		= questd.o dumper.o port.o arping.o usb.o ndisc.o
SRCS		= questd.c dumper.c port.c arping.c usb.c ndisc.c
LIBSRCS		= 
ISRCS		= questd.h

all: questd

questd: ${OBJS}
	${CC} ${LDFLAGS} ${LIBSRCS} -o questd ${OBJS} ${LIBS}

clean:
	rm -f questd *.o

