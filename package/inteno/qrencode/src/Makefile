CC		= gcc
MAKEDEPEND	= makedepend
CDEBUG		= -g
CFLAGS		= ${CDEBUG} ${INCL} -Wall
LDFLAGS		= ${CDEBUG}
LIBDIR		= 
LOCLIBS		= 
LIBS		= ${LOCLIBS} ${SYSLIBS}
OBJS		= bitstream.o  mask.o  qrenc.o  qrencode.o  qrinput.o  qrspec.o  rscode.o  split.o
SRCS		= bitstream.c  mask.c  qrenc.c  qrencode.c  qrinput.c  qrspec.c  rscode.c  split.c
LIBSRCS		= 
ISRCS		= bitstream.h  mask.h  qrencode.h  qrencode_inner.h  qrinput.h  qrspec.h  rscode.h  split.h config.h
ALLSRCS		= ${SRCS} ${ISRCS} ${LIBSRCS}

all: qrencode

qrencode: ${OBJS}
	${CC} ${LDFLAGS} -o qrencode ${OBJS} ${LIBDIR} ${LIBS}

clean:
	rm -f qrencode core *.o *.BAK *.bak *.CKP a.out

install:
	install -c -s -o bin -g bin -m 555 qrencode /usr/local/bin

depend: 
	${MAKEDEPEND} ${INCL} ${SRCS} ${LIBSRCS}

