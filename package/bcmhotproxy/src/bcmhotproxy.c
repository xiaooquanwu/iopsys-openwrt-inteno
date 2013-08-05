/*
 * bcmhotproxy  -- a proxy to send messages from broadcom drivers to userspace
 *
 * Copyright (C) 2012-2013 Inteno Broadband Technology AB. All rights reserved.
 *
 * Author: Strhuan Blomquist
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 */
#include "bcmhotproxy.h"

int netlink_init() {
	int sock_fd;
	sock_fd=socket(PF_NETLINK, SOCK_RAW, NETLINK_BRCM_MONITOR);
	if(sock_fd<0)
	    return -1;
	return sock_fd;
}
int hotplug_call(struct hotplug_arg arg)
{
  char str[512];
  int ret;
  memset(str, '\0', sizeof(512));
  syslog(LOG_INFO, "ACTION=%s INTERFACE=%s /sbin/hotplug-call %s", arg.action,arg.inteface,arg.subsystem);
  sprintf(str, "ACTION=%s INTERFACE=%s /sbin/hotplug-call %s",arg.action,arg.inteface,arg.subsystem);
  ret=system(str);
  return ret;
}
hotplug_arg createargumetstruct(char *hotplugmsg)
{ 
  hotplug_arg arg;
  char argumets[3][20];
  char * pch;
  int x=0;
  pch = strtok (hotplugmsg," ");
  while (pch != NULL){
    strcpy(argumets[x],pch);
    pch = strtok (NULL, " ");
    x++;
  }
  strncpy(arg.action,argumets[0],sizeof(arg.action));
  strncpy(arg.inteface,argumets[1],sizeof(arg.action));
  strncpy(arg.subsystem,argumets[2],sizeof(arg.action));
  
  return arg;

}

int netlink_bind(int sock_fd) {
  
  struct sockaddr_nl src_addr;
  memset(&src_addr, 0, sizeof(src_addr));
  src_addr.nl_family = AF_NETLINK;
  src_addr.nl_pid = getpid();  /* self pid */
  src_addr.nl_groups = 1; //multicast Group
  
  bind(sock_fd, (struct sockaddr*)&src_addr,sizeof(src_addr));    
  
  if (bind(sock_fd, (struct sockaddr*)&src_addr,sizeof(src_addr))) {
    close(sock_fd);
    return -1;
    }

  return sock_fd;
}

int dispatcher() {
    struct sockaddr_nl dest_addr;
    struct nlmsghdr *nlh = NULL;
    struct iovec iov;
    struct msghdr msg;
    hotplug_arg arg;
    int sock_fd;
    /* Initlize the netlink socket */
    sock_fd=netlink_init();
    if (sock_fd == -1) {
		fprintf(stderr, "Unable to Intitlize netlink socket.\n");
		exit(1);
	}
     /* Bind the netlink socket */
    sock_fd=netlink_bind(sock_fd);
    if (sock_fd == -1) {
		fprintf(stderr, "Unable to Listen to netlink socket.\n");
		exit(1);
	}
    /* destination address to listen to */
    nlh = (struct nlmsghdr *)malloc(NLMSG_SPACE(MAX_PAYLOAD));
    memset(nlh, 0, NLMSG_SPACE(MAX_PAYLOAD));
    nlh->nlmsg_len = NLMSG_SPACE(MAX_PAYLOAD);
    nlh->nlmsg_pid = getpid();
    nlh->nlmsg_flags = 0;
    
    /* Fill the netlink message header */
    iov.iov_base = (void *)nlh;
    iov.iov_len = nlh->nlmsg_len;
    msg.msg_name = (void *)&dest_addr;
    msg.msg_namelen = sizeof(dest_addr);
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;


    /* Read message from kernel */
    while(1){
      recvmsg(sock_fd, &msg, 0);
      switch (nlh->nlmsg_type)
      {
         case MSG_NETLINK_BRCM_WAKEUP_MONITOR_TASK:
         case MSG_NETLINK_BRCM_LINK_STATUS_CHANGED:
            /*process the message */
            fprintf(stderr, "No Handle\n");
            break;
	 case MSG_NETLINK_BRCM_LINK_HOTPLUG:
	    arg=createargumetstruct((char *)NLMSG_DATA(nlh));
	    if(hotplug_call(arg)){
	    fprintf(stderr, "Unable to call hotplug.\n");
	    }
	    break;  
         default:
	    fprintf(stderr, "Unknown type\n");
            break;
      }
      
    }
    close(sock_fd);
    return 0;
    
}
