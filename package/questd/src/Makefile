CC		= gcc
CFLAGS		= -g -Wall
CFLAGS 		+= -I$(BCMKERNEL_DIR)/kernel
CFLAGS 		+= -I$(BCMKERNEL_DIR)/userspace/gpl/apps/bridge-utils
LOCLIBS		= 
LIBS		= -luci -lubus -lubox -lpthread
OBJS		= questd.o dumper.o arp.o
OBJS		+= $(BCMKERNEL_DIR)/userspace/gpl/apps/bridge-utils/libbridge/*.o
SRCS		= questd.c dumper.c arp.c
LIBSRCS		= 
ISRCS		= questd.h

all: questd

questd: ${OBJS}
	${CC} ${LDFLAGS} ${LIBSRCS} -o questd ${OBJS} ${LIBS}

clean:
	rm -f questd *.o


