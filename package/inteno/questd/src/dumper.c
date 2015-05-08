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
get_db_hw_value(char *opt, char **val)
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
get_jif_val(jiffy_counts_t *p_jif)
{
	FILE *file;
	char line[128];
	int ret;

	if ((file = fopen("/proc/stat", "r"))) {
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
	char *val;

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


static void
get_file_contents(char *path, char **val) {
	FILE *in;
	char result[32];

	*val = "";

	if ((in = fopen(path, "r"))) {
		fgets(result, sizeof(result), in);
		remove_newline(result);
		fclose(in);
		*val = strdup(result);
	}
}

void
dump_static_router_info(Router *router)
{
	get_file_contents("/proc/nvram/BoardId", &router->nvramver); 
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

	strcpy(router->name, "");
	if ((file = fopen("/proc/sys/kernel/hostname", "r"))) {
		while(fgets(line, sizeof(line), file) != NULL)
		{
			remove_newline(line);
			if (sscanf(line, "%s", name) == 1)
				break;
		}
		fclose(file);
	}
	strcpy(router->name, name);
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
		sprintf(router->uptime, "%dd %dh %dm %lds", days, hours, minutes, seconds);

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

