/*
 * igmp.h
 *
 *  Created on: May 5, 2015
 *      Author: stefan
 */

#ifndef IGMP_H_
#define IGMP_H_
#ifndef NULL
#define NULL   ((void *) 0)
#endif



int igmp_rpc(struct ubus_context *ctx, struct ubus_object *obj,
	struct ubus_request_data *req, const char *method,
	struct blob_attr *msg);


typedef struct igmp_table {
	char bridge[32];
	char device[32];
	char srcdev[32];
	char tags[32];
	char lantci[32];
	char wantci[32];
	char group[32];
	char mode[32];
	char RxGroup[32];
	char source[32];
	char reporter[32];
	char timeout[32];
	char Index[32];
	char ExcludPt[32];

}IGMPtable;
#endif /* IGMP_H_ */
