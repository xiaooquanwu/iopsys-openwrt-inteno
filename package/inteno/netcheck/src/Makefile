CC		= gcc
MAKEDEPEND	= makedepend
CDEBUG		= -g
CFLAGS		= ${CDEBUG} ${INCL} -Wall
LDFLAGS		= ${CDEBUG}
LIBDIR		= 
LOCLIBS		= 
LIBS		= ${LOCLIBS} ${SYSLIBS}
OBJS		= netcheck.o arping.o
SRCS		= netcheck.c arping.c
LIBSRCS		= 
ISRCS		= 
ALLSRCS		= ${SRCS} ${ISRCS} ${LIBSRCS}

all: netcheck

netcheck: ${OBJS}
	${CC} ${LDFLAGS} -o netcheck ${OBJS} ${LIBDIR} ${LIBS}

clean:
	rm -f netcheck core *.o *.BAK *.bak *.CKP a.out

depend: 
	${MAKEDEPEND} ${INCL} ${SRCS} ${LIBSRCS}

