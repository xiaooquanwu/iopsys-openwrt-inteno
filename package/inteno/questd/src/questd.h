#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <stdbool.h>

#include <sys/sysinfo.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>

#include <uci.h>


#include <libubox/blobmsg.h>
#include <libubox/uloop.h>
#include <libubox/ustream.h>
#include <libubox/utils.h>

#include <libubus.h>

#include "dslstats.h"

#define MAX_VIF		8
#define MAX_NETWORK	16
#define MAX_CLIENT	128
#define MAX_PORT	8
#define MAX_USB		18

typedef struct {
	const char *vif;
	const char *device;
	const char *ssid;
	const char *network;
} Wireless;

typedef struct {
	int connum;
	int idle;
	int in_network;
	long tx_bytes;
	long rx_bytes;
	int tx_rate;
	int rx_rate;
	int snr;
} Detail;

typedef struct {
	bool exists;
	bool local;
	bool dhcp;
	char leaseno[24];
	char macaddr[24];
	char hostaddr[24];
	char hostname[64];
	char network[32];
	char device[32];
	bool wireless;
	char wdev[8];
	bool connected;
} Client;

typedef struct {
	bool exists;
	char ip6addr[128];
	char macaddr[24];
	char hostname[64];
	char duid[64];
	char device[32];
	bool wireless;
	char wdev[8];
	bool connected;
} Client6;

typedef struct {
        unsigned long rx_bytes;
        unsigned long rx_packets;
        unsigned long rx_errors;
        unsigned long tx_bytes;
        unsigned long tx_packets;
        unsigned long tx_errors;
} Statistic;

typedef struct {
	char name[16];
	char ssid[32];
	char device[32];
	Statistic stat;
	Client client[MAX_CLIENT];
} Port;

typedef struct {
	bool exists;
	bool is_lan;
	const char *name;
	const char *type;
	const char *proto;
	const char *ipaddr;
	const char *netmask;
	char ifname[128];
	Port port[MAX_PORT];
	bool ports_populated;
} Network;

typedef struct {
	char name[64];
	char *hardware;
	char *model;
	char *nvramver;
	char *firmware;
	char *brcmver;
	char *socmod;
	char *socrev;
	char *cfever;
	char *kernel;
	char *basemac;
	char *serialno;
	char uptime[64];
	unsigned int procs;
	unsigned int cpu;
} Router;

typedef struct {
	unsigned long total;
	unsigned long used;
	unsigned long free;
	unsigned long shared;
	unsigned long buffers;
} Memory;

typedef struct {
	char *auth;
	char *des;
	char *wpa;
} Key;

typedef struct {
	bool wifi;
	bool adsl;
        bool vdsl;
        bool voice;
        bool dect;
        int vports;
	int eports;
} Spec;

typedef struct {
	char mount[64];
	char product[64];
	char no[8];
	char name[8];
	unsigned long size;
	char *device;
	char *vendor;
	char *serial;
	char *speed;
	char *maxchild;
} USB;

typedef struct jiffy_counts_t {
	unsigned long long usr, nic, sys, idle;
	unsigned long long iowait, irq, softirq, steal;
	unsigned long long total;
	unsigned long long busy;
} jiffy_counts_t;

struct fdb_entry
{
	u_int8_t mac_addr[6];
	u_int16_t port_no;
	unsigned char is_local;
};

void recalc_sleep_time(bool calc, int toms);
void init_db_hw_config(void);
bool arping(char *target, char *device, int toms);
void remove_newline(char *buf);
void replace_char(char *buf, char a, char b);
void runCmd(const char *pFmt, ...);
const char *chrCmd(const char *pFmt, ...);
void get_jif_val(jiffy_counts_t *p_jif);
void dump_keys(Key *keys);
void dump_specs(Spec *spec);
void dump_static_router_info(Router *router);
void dump_hostname(Router *router);
void dump_sysinfo(Router *router, Memory *memory);
void dump_cpuinfo(Router *router, jiffy_counts_t *prev_jif, jiffy_counts_t *cur_jif);
void get_port_name(Port *port);
void get_port_stats(Port *port);
void get_bridge_ports(char *network, char **ifname);
void get_clients_onport(char *bridge, int portno, char **macaddr);
void dump_usb_info(USB *usb, char *usbno);
void clear_macaddr(void);
char *get_macaddr(void);
bool ndisc (const char *name, const char *ifname, unsigned flags, unsigned retry, unsigned wait_ms);
