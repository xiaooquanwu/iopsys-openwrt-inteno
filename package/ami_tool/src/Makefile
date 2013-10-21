# Makefile for Asterisk management interface listener application

CC		= gcc
MAKEDEPEND	= makedepend
CDEBUG		= -g
CFLAGS		= ${CDEBUG} ${INCL} -Wall
LDFLAGS		= ${CDEBUG}
LIBDIR		= 
LOCLIBS		= 
LIBS		= ${LOCLIBS} ${SYSLIBS}
OBJS		= ami_tool.o ami_connection.o ucix.o
SRCS		= ami_tool.c ami_connection.c ucix.c
LIBSRCS		= 
ISRCS		= ami_tool.h ami_connection.h ucix.h
ALLSRCS		= ${SRCS} ${ISRCS} ${LIBSRCS}

all: ami_tool

ami_tool: ${OBJS}
	${CC} ${LDFLAGS} -o ami_tool ${OBJS} ${LIBDIR} ${LIBS}

clean:
	rm -f ami_tool ${OBJS}

depend: 
	${MAKEDEPEND} ${INCL} ${SRCS} ${LIBSRCS}
