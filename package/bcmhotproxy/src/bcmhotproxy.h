#ifndef BCMHOTPROXY_H
#define BCMHOTPROXY_H 1
#include <sys/socket.h>
#include <linux/netlink.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <syslog.h>
#define MSG_NETLINK_BRCM_WAKEUP_MONITOR_TASK 0X1000

#define MSG_NETLINK_BRCM_LINK_STATUS_CHANGED 0X2000

#define MSG_NETLINK_BRCM_LINK_HOTPLUG 0X3000
#define MAX_PAYLOAD 1024  /* maximum payload size*/
#ifndef NETLINK_BRCM_MONITOR
#define NETLINK_BRCM_MONITOR 25
#endif
typedef struct hotplug_arg hotplug_arg;

struct hotplug_arg
{
	char action[20];
	char inteface[20];
	char subsystem[20];
  
}; 
#endif
