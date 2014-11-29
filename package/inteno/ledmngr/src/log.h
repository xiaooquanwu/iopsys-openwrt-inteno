#ifndef LOG_H
extern int daemonize;

#define DEBUG_PRINT_RAW(...) if (!daemonize) fprintf( stderr, __VA_ARGS__ );
#define DEBUG_PRINT(fmt, args...)                               \
    do {                                                        \
        if (!daemonize)                                         \
            fprintf( stderr,"%-20s: " fmt , __func__, ##args);  \
    } while(0)

#endif /* LOG_H */
