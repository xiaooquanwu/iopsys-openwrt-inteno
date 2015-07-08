CC		= gcc
CFLAGS		= -g -Wall
LOCLIBS		= 
LIBS		= -lubus -lubox -lblobmsg_json
OBJS		= ifsi.o
SRCS		= ifsi.c
LIBSRCS		= 
ISRCS		= 

all: ifsi

ifsi: ${OBJS}
	${CC} ${LDFLAGS} ${LIBSRCS} -o ifsi ${OBJS} ${LIBS}

clean:
	rm -f ifsi *.o

