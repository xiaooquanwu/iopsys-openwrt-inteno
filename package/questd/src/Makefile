CC		= gcc
MAKEDEPEND	= makedepend
CDEBUG		= -g
CFLAGS		= ${CDEBUG} ${INCL} -Wall
LDFLAGS		= ${CDEBUG}
LIBDIR		= 
LOCLIBS		= 
LIBS		= ${LOCLIBS} ${SYSLIBS}
OBJS		= questd.o dumper.o arp.o
SRCS		= questd.c dumper.c arp.c
LIBSRCS		= 
ISRCS		= questd.h
ALLSRCS		= ${SRCS} ${ISRCS} ${LIBSRCS}

all: questd

questd: ${OBJS}
	${CC} ${LDFLAGS} -o questd ${OBJS} ${LIBDIR} ${LIBS}

clean:
	rm -f questd core *.o *.BAK *.bak *.CKP a.out

depend: 
	${MAKEDEPEND} ${INCL} ${SRCS} ${LIBSRCS}

