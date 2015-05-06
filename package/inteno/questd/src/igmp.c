/*
 * igmp.c
 *
 *  Created on: May 5, 2015
 *      Author: stefan
 */

#include "questd.h"
#include "igmp.h"


static void tokenize_line(char *str, const char **tokens, size_t tokens_size) {
	char *ptr = str;
	const char **token = tokens; // = line;
	while (*ptr) {
		if (*ptr == '\t' || *ptr == ' ' || *ptr == '\n') {
			*ptr++ = 0;
			continue;
		} else {
			*token++ = ptr;
			tokens_size--;
			if (tokens_size == 0) {
				return;
			}
			while (*ptr && *ptr != '\t' && *ptr != ' ' && *ptr != '\n')
				ptr++;
			continue;
		}
		ptr++;
	}  
}

int igmp_rpc(struct ubus_context *ctx, struct ubus_object *obj,
		struct ubus_request_data *req, const char *method,
		struct blob_attr *msg) {
	struct blob_buf bb;
	int row = 0, idx = 0;
	FILE *in;
	char line[256];
	IGMPtable table[128];
	const char *tokens[32] = { 0 };
	void *object, *array;

	blob_buf_init(&bb, 0);
	if (!(in = fopen("/proc/net/igmp_snooping", "r")))
		return 1;
	array = blobmsg_open_array(&bb, "tables");

	struct element {
		char *string;
		struct element *next;
	};
	while (fgets(line, sizeof(line), in)) {
		if (row < 2) {
			row++;
			continue;
		}
		//remove_newline(line);

		tokenize_line(line, tokens, sizeof(tokens) / sizeof(char*));
		int tok_pos = 0;
		const char **token = tokens;
		//const char *names[] = { "bridge", "dev", "srcdev", "tags", "lantci",
		//		"wantci", "group", "mode", "rxgroup", "source", "reporter",
		//		"timeout", "index", "excludept" };

		while (*token) {
			printf("<%d>\n",tok_pos);
			switch (tok_pos) {
			case 0:

				sprintf(table[row-2].bridge,"%s",*token);
				printf("%s\n%s\n",table[row-2].bridge,*token);
				break;
			case 1:
				sprintf(table[row-2].device,"%s",*token);
				break;
			case 2:
				sprintf(table[row - 2].tags ,"%s",*token);
				break;
			case 3: {
				uint32_t ip;
				sscanf(*token, "%8x", &ip);
				sprintf(table[row - 2].lantci, "%d.%d.%d.%d", (ip >> 16),
						(ip >> 8) & 0xff, ip & 0xff, (ip >> 24) & 0xff);
			}
				break;
			case 4: {
				uint32_t ip;
				sscanf(*token, "%8x", &ip);
				sprintf(table[row - 2].wantci, "%d.%d.%d.%d", (ip >> 16),
						(ip >> 8) & 0xff, ip & 0xff, (ip >> 24) & 0xff);
			}
				break;
			case 5: {
				uint32_t ip;
				sscanf(*token, "%8x", &ip);
				sprintf(table[row - 2].group, "%d.%d.%d.%d", (ip >> 16),
						(ip >> 8) & 0xff, ip & 0xff, (ip >> 24) & 0xff);
			}
				break;
			case 6:
				sprintf(table[row - 2].mode,"%s",*token);
				break;
			case 7: {
				uint32_t ip;

				sscanf(*token, "%8x", &ip);
				sprintf(table[row - 2].RxGroup, "%d.%d.%d.%d", (ip >> 16),
						(ip >> 8) & 0xff, ip & 0xff, (ip >> 24) & 0xff);
			}
				break;
			case 8: {
				uint32_t ip;
				sscanf(*token, "%8x", &ip);
				sprintf(table[row - 2].source, "%d.%d.%d.%d", (ip >> 16),
						(ip >> 8) & 0xff, ip & 0xff, (ip >> 24) & 0xff);
			}
				break;
			case 9: {
				uint32_t ip;
				sscanf(*token, "%8x", &ip);
				sprintf(table[row - 2].reporter, "%d.%d.%d.%d", (ip >> 16),
						(ip >> 8) & 0xff, ip & 0xff, (ip >> 24) & 0xff);
			}
				break;
			case 10:
				sprintf(table[row - 2].timeout,"%s",*token);

				break;
			case 11:
				sprintf(table[row - 2].Index,"%s",*token);

				break;
			case 12:
				sprintf(table[row - 2].ExcludPt,"%s",*token);

				break;
			default:
				break;

			}
			token++;
			tok_pos++;
			printf("</%d\n>",tok_pos);
		}

		row++;

	}
	for (idx = 0; idx < (row - 2); idx++) {
		object = blobmsg_open_table(&bb, NULL);
		blobmsg_add_string(&bb,"bridge", table[idx].bridge);
		blobmsg_add_string(&bb,"device", table[idx].device);
		blobmsg_add_string(&bb,"srcdev", table[idx].srcdev);
		blobmsg_add_string(&bb,"tags", table[idx].tags);
		blobmsg_add_string(&bb,"lantci", table[idx].lantci);
		blobmsg_add_string(&bb,"wantci", table[idx].wantci);
		blobmsg_add_string(&bb,"group", table[idx].group);
		blobmsg_add_string(&bb,"mode", table[idx].mode);
		blobmsg_add_string(&bb,"rxgroup", table[idx].RxGroup);
		blobmsg_add_string(&bb,"source", table[idx].source);
		blobmsg_add_string(&bb,"reporter", table[idx].reporter);
		blobmsg_add_string(&bb,"timeout", table[idx].timeout);
		blobmsg_add_string(&bb,"index", table[idx].Index);
		blobmsg_add_string(&bb,"excludpt", table[idx].ExcludPt);
		blobmsg_close_table(&bb, object);
	}
	blobmsg_close_array(&bb, array);

	ubus_send_reply(ctx, req, bb.head);

	return 0;
}

