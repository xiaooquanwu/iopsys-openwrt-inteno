/*
 * port -- collects port info for questd
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

#include "questd.h"
#include <linux/if_bridge.h>
#include <errno.h>

#define CHUNK		128

static long
get_port_stat(char *dev, char *stat)
{
	FILE *in;
	char cmnd[64];
	char result[32];

	sprintf(cmnd, "/sys/class/net/%s/statistics/%s", dev, stat);
	if ((in = fopen(cmnd, "r"))) {
		fgets(result, sizeof(result), in);
		fclose(in);
	}

	return atoi(result);
}

void
get_port_stats(Port *port)
{
	port->stat.rx_bytes = get_port_stat(port->device, "rx_bytes");
	port->stat.rx_packets = get_port_stat(port->device, "rx_packets");
	port->stat.rx_errors = get_port_stat(port->device, "rx_errors");
	port->stat.tx_bytes = get_port_stat(port->device, "tx_bytes");
	port->stat.tx_packets =get_port_stat(port->device, "tx_packets");
	port->stat.tx_errors = get_port_stat(port->device, "tx_errors");
}

void
get_port_name(Port *port)
{
	FILE *in;
	char buf[32];
	char cmnd[80];

	sprintf(cmnd, ". /lib/network/config.sh && interfacename %s 2>/dev/null", port->device);
	if (!(in = popen(cmnd, "r")))
		exit(1);

	fgets(buf, sizeof(buf), in);
	pclose(in);
	remove_newline(buf);
	strcpy(port->name, buf);

	memset(cmnd, '\0', sizeof(cmnd));
	memset(buf, '\0', sizeof(buf));

	if(!strncmp(port->device, "wl", 2)) {
		sprintf(cmnd, "wlctl -i %s ssid | awk '{print$3}' | sed 's/\"//g' 2>/dev/null", port->device);
		if (!(in = popen(cmnd, "r")))
			exit(1);

		fgets(buf, sizeof(buf), in);
		pclose(in);
		remove_newline(buf);
		strcpy(port->ssid, buf);
	}
}

void
get_bridge_ports(char *bridge, char **ports)
{
	FILE *in;
	char buf[64];
	char cmnd[128];
	
	*ports = "";

	sprintf(cmnd, "brctl showbr %s | awk '{print$NF}' | grep -v interfaces | tr '\n' ' '", bridge);
	if (!(in = popen(cmnd, "r")))
		exit(1);

	fgets(buf, sizeof(buf), in);
	pclose(in);
	*ports = strdup(buf);
}

static int
compare_fdbs(const void *_f0, const void *_f1)
{
	const struct fdb_entry *f0 = _f0;
	const struct fdb_entry *f1 = _f1;

	return memcmp(f0->mac_addr, f1->mac_addr, 6);
}

static inline void
copy_fdb(struct fdb_entry *ent, const struct __fdb_entry *f)
{
	memcpy(ent->mac_addr, f->mac_addr, 6);
	ent->port_no = f->port_no;
	ent->is_local = f->is_local;
}

static int
bridge_read_fdb(const char *bridge, struct fdb_entry *fdbs, unsigned long offset, int num)
{
	FILE *f;
	int i, n;
	struct __fdb_entry fe[num];
	char path[256];
	
	snprintf(path, 256, "/sys/class/net/%s/brforward", bridge);
	f = fopen(path, "r");
	if (f) {
		fseek(f, offset*sizeof(struct __fdb_entry), SEEK_SET);
		n = fread(fe, sizeof(struct __fdb_entry), num, f);
		fclose(f);
	}

	for (i = 0; i < n; i++) 
		copy_fdb(fdbs+i, fe+i);

	return n;
}

void
get_clients_onport(char *bridge, int portno, char **macaddr)
{
	int i, n;
	struct fdb_entry *fdb = NULL;
	int offset = 0;
	char tmpmac[2400];
	char mac[24];
	
	*macaddr = "";

	for(;;) {
		fdb = realloc(fdb, (offset + CHUNK) * sizeof(struct fdb_entry));
		if (!fdb) {
			fprintf(stderr, "Out of memory\n");
			return;
		}
			
		n = bridge_read_fdb(bridge, fdb+offset, offset, CHUNK);
		if (n == 0)
			break;

		if (n < 0) {
			fprintf(stderr, "read of forward table failed: %s\n",
				strerror(errno));
			return;
		}

		offset += n;
	}

	qsort(fdb, offset, sizeof(struct fdb_entry), compare_fdbs);

	for (i = 0; i < offset; i++) {
		const struct fdb_entry *f = fdb + i;
		if (f->port_no == portno && !f->is_local) {
			sprintf(mac, "%.2x:%.2x:%.2x:%.2x:%.2x:%.2x", f->mac_addr[0], f->mac_addr[1], f->mac_addr[2], f->mac_addr[3], f->mac_addr[4], f->mac_addr[5]);
			strcat(tmpmac, " ");
			strcat(tmpmac, mac);
		}
	}
	*macaddr = strdup(tmpmac);

	free(fdb);
	memset(tmpmac, '\0', sizeof(tmpmac));
}
