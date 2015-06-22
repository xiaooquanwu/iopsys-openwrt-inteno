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

#define MAX_IGMP_ENTRY 128

int igmp_rpc(struct ubus_context *ctx, struct ubus_object *obj,
	struct ubus_request_data *req, const char *method,
	struct blob_attr *msg);


typedef struct igmp_table {
	bool exists;
	char bridge[32];
	char device[32];
	char srcdev[32];
	char tags[32];
	int lantci;
	int wantci;
	int group;
	char mode[32];
	int RxGroup;
	int source;
	int reporter;
	int timeout;
	int Index;
	int ExcludPt;

}IGMPtable;
#endif /* IGMP_H_ */
