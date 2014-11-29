#ifndef BRCMDAEMON_H
#define BRCMDAEMON_H 1
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <syslog.h>
#include <string.h>
#include <assert.h>
#include <signal.h>
 
#define DAEMON_NAME "brcmnetlink"
#define PID_FILE "/var/run/brcmnetlink.pid"
#endif