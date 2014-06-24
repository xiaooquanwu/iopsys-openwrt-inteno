# Makefile for endpoint configuration application

CC		= gcc
MAKEDEPEND	= makedepend
CDEBUG		= -g
CFLAGS		= ${CDEBUG} ${INCL} -Wall
LDFLAGS		= ${CDEBUG}
LIBDIR		= 
LOCLIBS		= 
LIBS		= ${LOCLIBS} ${SYSLIBS}
OBJS		= endptcfg.o
SRCS		= endptcfg.c
LIBSRCS		= 
ISRCS		=
ALLSRCS		= ${SRCS} ${ISRCS} ${LIBSRCS}

all: endptcfg

endptcfg: ${OBJS}
	${CC} ${LDFLAGS} -o endptcfg ${OBJS} ${LIBDIR} ${LIBS}

clean:
	rm -f endptcfg ${OBJS}

depend: 
	${MAKEDEPEND} ${INCL} ${SRCS} ${LIBSRCS}
