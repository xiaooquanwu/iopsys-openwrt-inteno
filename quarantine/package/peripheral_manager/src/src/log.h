#ifndef LOG_H
#define LOG_H
/* Use this */
#include <syslog.h>

extern int debug_level;

#define DBG_RAW(...) fprintf( stderr, __VA_ARGS__ );
#define DBG(level,fmt, args...)							\
	do {									\
		if (level <= debug_level)					\
			syslog( LOG_DEBUG,"%-20s: " fmt , __func__, ##args);	\
	} while(0)

#endif /* LOG_H */
