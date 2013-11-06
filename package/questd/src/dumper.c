/*
 * dumper -- collects router device and system info for questd
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

struct uci_context *db_ctx;
static bool dbLoaded = false;
static struct uci_ptr ptr;

void
init_db_hw_config(void)
{
	db_ctx = uci_alloc_context();
	uci_set_confdir(db_ctx, "/lib/db/config/");
	if(uci_load(db_ctx, "hw", NULL) == UCI_OK)
		dbLoaded = true;
}

static void
get_db_hw_value(char *opt, unsigned char **val)
{
	memset(&ptr, 0, sizeof(ptr));
	ptr.package = "hw";
	ptr.section = "board";
	ptr.value = NULL;

	*val = "";

	if (!dbLoaded)
		return;

	ptr.option = opt;
	if (uci_lookup_ptr(db_ctx, &ptr, NULL, true) != UCI_OK)
		return;

	if (!(ptr.flags & UCI_LOOKUP_COMPLETE))
		return;

	if(!ptr.o->v.string)
		return;

	*val = ptr.o->v.string;
}

void 
remove_newline(char *buf)
{
	int len;
	len = strlen(buf) - 1;
	if (buf[len] == '\n') 
		buf[len] = 0;
}

void
get_jif_val(jiffy_counts_t *p_jif)
{
	FILE *file;
	char line[128];
	int ret;

	if (file = fopen("/proc/stat", "r")) {
		while(fgets(line, sizeof(line), file) != NULL)
		{
			remove_newline(line);
			ret = sscanf(line, "cpu %llu %llu %llu %llu %llu %llu %llu %llu", &p_jif->usr, &p_jif->nic, &p_jif->sys, &p_jif->idle,
				&p_jif->iowait, &p_jif->irq, &p_jif->softirq, &p_jif->steal);

			if (ret >= 4) {
				p_jif->total = p_jif->usr + p_jif->nic + p_jif->sys + p_jif->idle
					+ p_jif->iowait + p_jif->irq + p_jif->softirq + p_jif->steal;

				p_jif->busy = p_jif->total - p_jif->idle - p_jif->iowait;
				break;
			}
		}
		fclose(file);
	}
}

void
dump_specs(Spec *spec)
{
	unsigned char *val;

	spec->wifi = false;
	spec->adsl = false;
	spec->vdsl = false;
	spec->voice = false;
	spec->dect = false;
	spec->vports = 0;
	spec->eports = 0;

	get_db_hw_value("hasWifi", &val);
	if (!strcmp(val, "1")) spec->wifi = true;

	get_db_hw_value("hasAdsl", &val);
	if (!strcmp(val, "1")) spec->adsl = true;

	get_db_hw_value("hasVdsl", &val);
	if (!strcmp(val, "1")) spec->vdsl = true;

	get_db_hw_value("hasVoice", &val);
	if (!strcmp(val, "1")) spec->voice = true;

	get_db_hw_value("hasDect", &val);
	if (!strcmp(val, "1")) spec->dect = true;

	get_db_hw_value("VoicePorts", &val);
	if (spec->voice) spec->vports = atoi(val);

	get_db_hw_value("ethernetPorts", &val);
	spec->eports = atoi(val);
}

void
dump_keys(Key *keys)
{
	get_db_hw_value("authKey", &keys->auth);
	get_db_hw_value("desKey", &keys->des);
	get_db_hw_value("wpaKey", &keys->wpa);
}

void
dump_static_router_info(Router *router)
{
	get_db_hw_value("hardwareVersion", &router->hardware);
	get_db_hw_value("routerModel", &router->model);
	get_db_hw_value("iopVersion", &router->firmware);
	get_db_hw_value("brcmVersion", &router->brcmver);
	get_db_hw_value("socModel", &router->socmod);
	get_db_hw_value("socRevision", &router->socrev);
	get_db_hw_value("cfeVersion", &router->cfever);
	get_db_hw_value("kernelVersion", &router->kernel);
	get_db_hw_value("BaseMacAddr", &router->basemac);
	get_db_hw_value("serialNumber", &router->serialno);
}

void
dump_hostname(Router *router)
{
	FILE *file;
	char line[64];
	char name[64];

	strcpy(&router->name, "");
	if (file = fopen("/proc/sys/kernel/hostname", "r")) {
		while(fgets(line, sizeof(line), file) != NULL)
		{
			remove_newline(line);
			if (sscanf(line, "%s", &name) == 1)
				break;
		}
		fclose(file);
	}
	if(name) strcpy(&router->name, name);
}

void
dump_sysinfo(Router *router, Memory *memory)
{
	struct sysinfo sinfo;
	long int seconds;
	int days, hours, minutes;

	if (sysinfo(&sinfo) == 0) {
		seconds = sinfo.uptime;
		days = seconds / (60 * 60 * 24);
		seconds -= days * (60 * 60 * 24);
		hours = seconds / (60 * 60);
		seconds -= hours * (60 * 60);
		minutes = seconds / 60;
		seconds -= minutes * 60;
		sprintf(&router->uptime, "%dd %dh %dm %ds", days, hours, minutes, seconds);

		router->procs = sinfo.procs;

		memory->total = (sinfo.totalram / 1024);
		memory->free = (sinfo.freeram / 1024);
		memory->used = ((sinfo.totalram - sinfo.freeram) / 1024);
		memory->shared = (sinfo.sharedram / 1024);
		memory->buffers = (sinfo.bufferram / 1024);
	}
}

void
dump_cpuinfo(Router *router, jiffy_counts_t *prev_jif, jiffy_counts_t *cur_jif)
{
	unsigned total_diff, cpu;

	total_diff = (unsigned)(cur_jif->total - prev_jif->total);

	if (total_diff == 0) total_diff = 1;

	cpu = 100 * (unsigned)(cur_jif->usr - prev_jif->usr) / total_diff;

	router->cpu = cpu;
}

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
	char buf[8];
	char cmnd[64];

	sprintf(cmnd, ". /lib/network/config.sh && interfacename %s 2>/dev/null", port->device);
	if (!(in = popen(cmnd, "r")))
		exit(1);

	fgets(buf, sizeof(buf), in);
	remove_newline(&buf);
	strcpy(&port->name, buf);
}

static int
compare_fdbs(const void *_f0, const void *_f1)
{
	const struct fdb_entry *f0 = _f0;
	const struct fdb_entry *f1 = _f1;

	return memcmp(f0->mac_addr, f1->mac_addr, 6);
}

void
get_client_onport(char *brname, int pno, unsigned char **macaddr)
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
			return 1;
		}
			
		n = br_read_fdb(brname, fdb+offset, offset, CHUNK);
		if (n == 0)
			break;

		if (n < 0) {
			fprintf(stderr, "read of forward table failed: %s\n",
				strerror(errno));
			return 1;
		}

		offset += n;
	}

	qsort(fdb, offset, sizeof(struct fdb_entry), compare_fdbs);

	for (i = 0; i < offset; i++) {
		const struct fdb_entry *f = fdb + i;
		if (f->port_no == pno && !f->is_local) {
			sprintf(mac, "%.2x:%.2x:%.2x:%.2x:%.2x:%.2x", f->mac_addr[0], f->mac_addr[1], f->mac_addr[2], f->mac_addr[3], f->mac_addr[4], f->mac_addr[5]);
			strcat(tmpmac, " ");
			strcat(tmpmac, mac);
		}
	}
	*macaddr = strdup(tmpmac);
	memset(tmpmac, '\0', sizeof(tmpmac));
}

