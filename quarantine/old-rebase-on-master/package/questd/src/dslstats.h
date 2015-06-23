/*
 *  dslstats.c - Quest U-bus daemon IOPSYS
 *
 *  Author: Martin K. Schröder, martin.schroder@inteno.se
 *
 *  Copyright © 2004-2007 Rémi Denis-Courmont.
 *  This program is free software: you can redistribute and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, versions 2 of the license.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
 
#pragma once

enum {
	DSLSTATS_BEARER_0 = 0, 
	DSLSTATS_BEARER_COUNT
}; 

enum {
	DSLSTATS_COUNTER_TOTALS, 
	DSLSTATS_COUNTER_CURRENT_15,
	DSLSTATS_COUNTER_PREVIOUS_15, 
	DSLSTATS_COUNTER_CURRENT_DAY,
	DSLSTATS_COUNTER_PREVIOUS_DAY,
	DSLSTATS_COUNTER_SINCE_LINK,
	DSLSTATS_COUNTER_COUNT
}; 

typedef struct { double up; double down } UpDown; 
typedef struct dsl_bearer {

	UpDown max_rate; 
	UpDown rate; 
	UpDown msgc; 
	UpDown b,m,t,r,s,l,d; 
	UpDown delay; 
	UpDown inp; 
	UpDown sf, sf_err;
	UpDown rs, rs_corr, rs_uncorr;
	UpDown hec, ocd, lcd; 
	UpDown total_cells, data_cells, bit_errors; 
	
} DSLBearer; 

typedef struct dsl_counters {
	UpDown es, ses, uas; 
	UpDown fec, crc; 
} DSLCounters; 

typedef struct dsl_stats {
	char *mode; 
	char *traffic;
	char *status; 
	char *link_power_state; 
	char *line_status; 
	char *vdsl2_profile; 
	UpDown trellis; 
	UpDown snr; 
	UpDown pwr; 
	UpDown attn; 
	DSLBearer bearers[DSLSTATS_BEARER_COUNT]; 
	DSLCounters counters[DSLSTATS_COUNTER_COUNT]; 
} DSLStats; 

void dslstats_init(struct dsl_stats *self); 
void dslstats_load(struct dsl_stats *self); 
void dslstats_to_blob_buffer(struct dsl_stats *self, struct blob_buf *b); 
int dslstats_rpc(struct ubus_context *ctx, struct ubus_object *obj, 
	struct ubus_request_data *req, const char *method, 
	struct blob_attr *msg); 
