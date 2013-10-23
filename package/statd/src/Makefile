# Makefile for statd application

CC		= gcc
MAKEDEPEND	= makedepend
CDEBUG		= -g
CFLAGS		= ${CDEBUG} ${INCL} -Wall
LDFLAGS		= ${CDEBUG}
LIBDIR		= 
LOCLIBS		= 
LIBS		= ${LOCLIBS} ${SYSLIBS}
OBJS		= statd.o statd_rules.o
SRCS		= statd.c statd_rules.c
LIBSRCS		= 
ISRCS		= statd.h statd_rules.h
ALLSRCS		= ${SRCS} ${ISRCS} ${LIBSRCS}

all: statd

statd: ${OBJS}
	${CC} ${LDFLAGS} -o statd ${OBJS} ${LIBDIR} ${LIBS}

clean:
	rm -f statd ${OBJS}

depend: 
	${MAKEDEPEND} ${INCL} ${SRCS} ${LIBSRCS}
