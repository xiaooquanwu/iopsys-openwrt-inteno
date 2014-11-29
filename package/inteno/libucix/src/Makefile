# Makefile for libucix

OBJS		= libucix.o
SRCS		= libucix.c

all: libucix

libucix: ${OBJS}
	${CC} ${LDFLAGS} -o libucix.so.${PKG_VERSION} ${OBJS} ${LIBDIR} ${LIBS}

clean:
	rm -f libucix.so.* libucix.a ${OBJS}

depend: 
	${MAKEDEPEND} ${INCL} ${SRCS} ${LIBSRCS}
