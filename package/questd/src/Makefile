CC		= gcc
CFLAGS		= -g -Wall
LOCLIBS		= 
LIBS		= -luci -lubus -lubox -lpthread
OBJS		= questd.o dumper.o port.o arp.o
SRCS		= questd.c dumper.c port.c arp.c
LIBSRCS		= 
ISRCS		= questd.h

all: questd

questd: ${OBJS}
	${CC} ${LDFLAGS} ${LIBSRCS} -o questd ${OBJS} ${LIBS}

clean:
	rm -f questd *.o

