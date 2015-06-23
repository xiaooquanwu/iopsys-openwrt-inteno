/*
 * netcheck.c -- netcheck utility for Inteno CPE's
 *
 * Copyright (C) 2012-2013 Inteno Broadband Technology AB. All rights reserved.
 *
 * Author: sukru.senli@inteno.se
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include <uci.h>
#include <json-c/json.h>

#include <netinet/in.h>
#include <arpa/inet.h>

static struct uci_context *uci_ctx;
static struct uci_package *uci_network;

static void
remove_newline(char *buf)
{
	int len;
	len = strlen(buf) - 1;
	if (buf[len] == '\n')
		buf[len] = 0;
}

static struct uci_package *
init_package(const char *config)
{
	struct uci_context *ctx = uci_ctx;
	struct uci_package *p = NULL;

	if (!ctx) {
		ctx = uci_alloc_context();
		uci_ctx = ctx;
	} else {
		p = uci_lookup_package(ctx, config);
		if (p)
			uci_unload(ctx, p);
	}

	if (uci_load(ctx, config, &p))
		return NULL;

	return p;
}

static void
display_json_var(json_object *obj, char *var)
{
	int ret = 0;
	json_object_object_foreach(obj, key, val) {
		if(!strcmp(key, var)) {
			switch (json_object_get_type(val)) {
			case json_type_object:
				break;
			case json_type_array:
				break;
			case json_type_string:
				fprintf(stdout, "%s\n", json_object_get_string(val));
				break;
			case json_type_boolean:
				fprintf(stdout, "%d\n", json_object_get_boolean(val));
				break;
			case json_type_int:
				fprintf(stdout, "%d\n", json_object_get_int(val));
				break;
			default:
				break;
			}
		}
	}
}

static void
json_parse_and_display(const char *str, char *var)
{
	json_object *obj;

	obj = json_tokener_parse(str);
	if (is_error(obj) || json_object_get_type(obj) != json_type_object) {
		exit(1);
	}
	display_json_var(obj, var);
}

static void
connection_status(char *ipaddr)
{
	json_object *new_obj, *my_string;
	FILE *in;
	char cmnd[256];
	char result[2048];

	sprintf(cmnd, "ubus -S call router host \"{ \\\"ipaddr\\\" : \\\"%s\\\" }\"", ipaddr);

	if ((in = popen(cmnd, "r")))
		fgets(result, sizeof(result), in);

	pclose(in);

	json_parse_and_display(result, "connected");
}

static void
match_host_to_network(struct uci_section *s, char *netname, char *hostaddr, unsigned int *local, unsigned char **dev)
{
	const char *is_lan = NULL;
	const char *ifname = NULL;
	struct in_addr ip, mask, snet, host, rslt;
	const char *type = NULL;
	const char *ipaddr = NULL;
	const char *netmask = NULL;
	char devbuf[16];

	is_lan = uci_lookup_option_string(uci_ctx, s, "is_lan");
	ifname = uci_lookup_option_string(uci_ctx, s, "ifname");

	if (is_lan && !strcmp(is_lan, "1")) {

	    if (ifname && !strcmp(ifname, "lo"))
		return;

	    type = uci_lookup_option_string(uci_ctx, s, "type");
	    ipaddr = uci_lookup_option_string(uci_ctx, s, "ipaddr");
	    netmask = uci_lookup_option_string(uci_ctx, s, "netmask");

	    inet_pton(AF_INET, ipaddr, &(ip.s_addr));
	    inet_pton(AF_INET, netmask, &(mask.s_addr));
	    inet_pton(AF_INET, hostaddr, &(host.s_addr));

	    snet.s_addr = (ip.s_addr & mask.s_addr);
	    rslt.s_addr = (host.s_addr & mask.s_addr);

	    if((snet.s_addr ^ rslt.s_addr) == 0) {
		*local = 1;
		if (type && !strcmp(type, "bridge")) {
		    snprintf(devbuf, 16, "br-%s", netname);
		    ifname = strdup(devbuf);
		}
		*dev = ifname;
	    }
	}
}

static void
get_portname(char *ifname, unsigned char **lanport)
{
	FILE *in;
	char buf[24];
	char cmnd[256];

	sprintf(cmnd, ". /lib/network/config.sh && interfacename %s", ifname);
	if (!(in = popen(cmnd, "r")))
		exit(1);

	fgets(buf, sizeof(buf), in);

	remove_newline(&buf);

	*lanport = strdup(buf);
}

static void
get_statistics(char *dev, char *stat)
{
	FILE *in;
	char cmnd[256];
	char result[64];

	sprintf(cmnd, "/sys/class/net/%s/statistics/%s", dev, stat);

	if ((in = fopen(cmnd, "r")))
		fgets(result, sizeof(result), in);

	fclose(in);

	fprintf(stdout, "\t\t%s %s", stat, result);
}

static void
get_clientinfo(char *macaddr, char *val)
{
	FILE *in;
	char cmnd[256];
	char result[2048];

	sprintf(cmnd, "ubus -S call router host \"{ \\\"macaddr\\\": \\\"%s\\\" }\"", macaddr);

	if ((in = popen(cmnd, "r")))
		fgets(result, sizeof(result), in);

	pclose(in);

	json_parse_and_display(result, val);
}

static void
find_clients(char *ipaddr, char *netmask, bool checkcon)
{
    FILE *leases = fopen("/var/dhcp.leases", "r");
    char line[256];
    char leaseno[24];
    char macaddr[24];
    char hostaddr[24];
    char hostname[128];
    char hwaddr[24];
    struct in_addr ip, mask, snet, host, rslt;
    int cno = 0;

	while(fgets(line, sizeof(line), leases) != NULL)
	{
		int len = strlen(line) - 1;

		if (line[len] == '\n') 
			line[len] = 0;

		if (sscanf(line, "%s %s %s %s %s", leaseno, macaddr, hostaddr, hostname, hwaddr) != 5)
			exit(1);

		inet_pton(AF_INET, ipaddr, &(ip.s_addr));
		inet_pton(AF_INET, netmask, &(mask.s_addr));
		inet_pton(AF_INET, hostaddr, &(host.s_addr));

		snet.s_addr = (ip.s_addr & mask.s_addr);
		rslt.s_addr = (host.s_addr & mask.s_addr);

		if((snet.s_addr ^ rslt.s_addr) == 0) {
		    if (checkcon) {
			fprintf(stdout, "\t------\n");
			fprintf(stdout, "\tClient\n");
			fprintf(stdout, "\t------\n");
			fprintf(stdout, "\t\tHostname: ");
			get_clientinfo(macaddr, "hostname");
			fprintf(stdout, "\t\tIP Address: ");
			get_clientinfo(macaddr, "ipaddr");
			fprintf(stdout, "\t\tMAC Address: %s\n", macaddr);
			fprintf(stdout, "\t\tConnected: ");
			get_clientinfo(macaddr, "connected");
		    }
		    else {
		        fprintf(stdout, "Lease[%d]\n", cno);
		        fprintf(stdout, "\thostname[%d]: %s\n", cno, hostname);
		        fprintf(stdout, "\thostaddr[%d]: %s\n", cno, hostaddr);
		        fprintf(stdout, "\tmacaddr[%d]: %s\n", cno, macaddr);
		    }
		    cno++;
		}
	}
	fclose(leases);

	if(!checkcon)
		exit(0);
}

static void
find_ports(char *interface, char *ifname, char *ipaddr, char *netmask, bool bridge)
{
	FILE *in;
	char line[256];
	int portno;
	char macaddr[24];
	char islocal[24];
	char ageing[24];
	char cmnd[256];

	char *spl, ports[6][10];
	int i = 1;

	spl = strtok (ifname, " ");
	while (spl != NULL)
	{
	    strcpy(ports[i], spl);
	    spl = strtok (NULL, " ");
	    i++;
	}

	fprintf(stdout, "\nETHERNET PORTS of '%s' interface\n", interface);
	fprintf(stdout, "___________________________________________\n");

	for(i=1; i<=5; i++)
	{

		char ethno[10];
		char *lanport;
		strncpy(ethno, ports[i]+3, strlen(ports[i]));

		if(!strstr(ports[i], "eth") || strchr(ports[i], '.'))
			continue;

		get_portname(ports[i], &lanport);

		fprintf(stdout, "|%s|", lanport);

		if(!bridge) {
			find_clients(ipaddr, netmask, true);
			goto stats;
		}

		sprintf(cmnd, "brctl showmacs br-%s | grep -v ageing | grep -v yes", interface);
		if (!(in = popen(cmnd, "r")))
			exit(1);

		while(fgets(line, sizeof(line), in) != NULL)
		{
			remove_newline(&line);

			if (sscanf(line, "%d %s %s %s", &portno, macaddr, islocal, ageing) != 4)
				exit(1);

			if (i == portno) {
				fprintf(stdout, "\t------\n");
				fprintf(stdout, "\tClient\n");
				fprintf(stdout, "\t------\n");
				fprintf(stdout, "\t\tHostname: ");
				get_clientinfo(macaddr, "hostname");
				fprintf(stdout, "\t\tIP Address: ");
				get_clientinfo(macaddr, "ipaddr");
				fprintf(stdout, "\t\tMAC Address: %s\n", macaddr);
				//fprintf(stdout, "\t\tConnected: ");
				//get_clientinfo(macaddr, "connected");
			}
		}

		pclose(in);

stats:

		fprintf(stdout, "\t----------\n");
		fprintf(stdout, "\tStatistics\n");
		fprintf(stdout, "\t----------\n");
		get_statistics(ports[i], "rx_bytes");
		get_statistics(ports[i], "rx_packets");
		get_statistics(ports[i], "rx_errors");
		get_statistics(ports[i], "tx_bytes");
		get_statistics(ports[i], "tx_packets");
		get_statistics(ports[i], "tx_errors");
		fprintf(stdout, "_____________________________________________\n");
	}
	exit(0);
}

static void
arpscan(char *netname, char *ipaddr, char *netmask, char *device, long timeout)
{
	struct in_addr ip, nmask, fmask, rmask, host;
	uint32_t last, addr;
	char str[INET_ADDRSTRLEN];

	inet_pton(AF_INET, ipaddr, &(ip.s_addr));
	inet_pton(AF_INET, netmask, &(nmask.s_addr));
	inet_pton(AF_INET, "255.255.255.255", &(fmask.s_addr));

	rmask.s_addr = (nmask.s_addr ^ fmask.s_addr);
	last = ntohl(ip.s_addr | rmask.s_addr);

        fprintf(stdout, "Scanning network '%s'\n", netname);
	for(addr = ntohl(ip.s_addr); addr <= last; addr++) {
		host.s_addr = htonl(addr);
		inet_ntop(AF_INET, &(host.s_addr), str, INET_ADDRSTRLEN);
		if(arping(str, device, timeout))
			fprintf(stdout, "Host found: %s\n", str);
	}
}

static void
handle_network(char *interface, int qaddr, int qmask, int qclnt, int qport, int qscan, long timeout)
{
	struct uci_element *e;
	uci_network = init_package("network");
	const char *ipaddr = NULL;
	const char *netmask = NULL;
	const char *ifname = NULL;
	const char *type = NULL;
	const char *islan = NULL;
	char device[32];
	bool bridge = false;

	uci_foreach_element(&uci_network->sections, e) {
		struct uci_section *s = uci_to_section(e);

		if (!strcmp(s->type, "interface") && !strcmp(s->e.name, interface)) {
		    ipaddr = uci_lookup_option_string(uci_ctx, s, "ipaddr");
		    netmask = uci_lookup_option_string(uci_ctx, s, "netmask");
		    ifname = uci_lookup_option_string(uci_ctx, s, "ifname");
		    type = uci_lookup_option_string(uci_ctx, s, "type");
		    islan = uci_lookup_option_string(uci_ctx, s, "is_lan");

		    if(!islan || strcmp(islan, "1")) {
			fprintf(stdout, "%s is not a LAN interface\n", interface);
			exit(1);
		    }

		    if (type && !strcmp(type, "bridge")) {
			bridge = true;
			sprintf(device, "br-%s", s->e.name);
		    }
		    else {
			strcpy(device, ifname);
		    }

		    if (qaddr == 1)
			fprintf(stdout, "%s\n", ipaddr);
		    if (qmask == 1)
			fprintf(stdout, "%s\n", netmask);
		    if (qclnt == 1)
			    find_clients(ipaddr, netmask, false);
		    if (qport == 1)
			    find_ports(interface, ifname, ipaddr, netmask, bridge);
		    if (qscan == 1)
			    arpscan(s->e.name, ipaddr, netmask, device, timeout);

		    exit(0);
		}
	}
}

static void
handle_ip(char *host, int qnet, int qlocal, int qdev, int qstat)
{
	struct uci_element *e;
	unsigned int local = 0;
	unsigned char *dev;
	int ip[4];

	if (sscanf(host, "%d.%d.%d.%d", &ip[0], &ip[1], &ip[2], &ip[3]) != 4) {
	    fprintf(stderr, "Wrong IP address format\n");
	    exit(1);
	}

	uci_network = init_package("network");

	uci_foreach_element(&uci_network->sections, e) {
		struct uci_section *s = uci_to_section(e);

		if (!strcmp(s->type, "interface")) {
			match_host_to_network(s, s->e.name, host, &local, &dev);
			if (local == 1) {
			    if (qnet == 1)
			        fprintf(stdout, "%s\n", s->e.name);
			    if (qlocal == 1)
			        fprintf(stdout, "1\n");
			    if (qdev == 1)
			        fprintf(stdout, "%s\n", dev);
			    if (qstat == 1)
			        connection_status(host);
			    exit(0);
			}
		}
	}
	if (qlocal == 1)
	    fprintf(stdout, "0\n");
}

static void usage(void)
{
	fprintf(stderr, "Usage: netcheck -[i|h] <interface|hostaddr> <option>\n");
	fprintf(stderr, "\tnetcheck -i <interface> <option>\n");
	fprintf(stderr, "\t\t-a\t\treturn ip address of the network interface\n");
	fprintf(stderr, "\t\t-c\t\treturn DHCP leases of the network interface\n");
	fprintf(stderr, "\t\t-m\t\treturn netmask of the network interface\n");
	fprintf(stderr, "\t\t-p\t\treturn ports information of the network interface\n");
	fprintf(stderr, "\t\t-q <timeout>\tarpscan the network with given timeout value in miliseconds\n");
	fprintf(stderr, "\tnetcheck -h <hostaddr> <option>\n");
	fprintf(stderr, "\t\t-d\t\treturn the device the host connects to\n");
	fprintf(stderr, "\t\t-l\t\treturn 1 if the host address is local otherwise return 0\n");
	fprintf(stderr, "\t\t-n\t\treturn the network interface the host belongs to\n");
	fprintf(stderr, "\t\t-s\t\treturn 1 if the host is reachable\n");
	exit(1);
}

int main(int argc, char **argv)
{

	int opt;
	int qinf = 0;
	int qhost = 0;
	int qaddr = 0;
	int qmask = 0;
	int qclnt = 0;
	int qport = 0;
	int qscan = 0;
	int qnet = 0;
	int qlocal = 0;
	int qdev = 0;
	int qstat = 0;
	long timeout = 100;
	const char *interface = NULL;
	const char *host = NULL;

	if (argc < 2)
		usage();

	while ((opt = getopt(argc, argv, "i:h:q:amcpnlds")) != -1) {

		switch (opt) {
			case 'i':
				qinf = 1;
				interface = optarg;
				break;
			case 'h':
				qhost = 1;
				host = optarg;
				break;
			case 'a':
				qaddr = 1;
				break;
			case 'm':
				qmask = 1;
				break;
			case 'c':
				qclnt = 1;
				break;
			case 'p':
				qport = 1;
				break;
			case 'q':
				qscan = 1;
				timeout = atoi(optarg);
				break;
			case 'n':
				qnet = 1;
				break;
			case 'l':
				qlocal = 1;
				break;
			case 'd':
				qdev = 1;
				break;
			case 's':
				qstat = 1;
				break;
			default:
				usage();
		}
	}
	argc -= optind;
	argv += optind;

	if (qinf == 1 && qhost == 1)
		usage();
	else if (qinf == 1 && (qaddr + qmask + qclnt + qport + qscan) == 1)
		handle_network(interface, qaddr, qmask, qclnt, qport, qscan, timeout);
	else if (qhost == 1 && (qnet + qlocal + qdev + qstat) == 1)
		handle_ip(host, qnet, qlocal, qdev, qstat);
	else
		usage();

	return 0;
}

