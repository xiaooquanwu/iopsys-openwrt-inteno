#include <string.h>
#include <stdio.h>

#include <json-c/json.h>

#include "common.h"

const char*
json_parse_and_get(const char *str, char *var)
{
	json_object *obj;
	char result[128];
	
	obj = json_tokener_parse(str);
	if (is_error(obj) || json_object_get_type(obj) != json_type_object) {
		return NULL;
	}

	json_object_object_foreach(obj, key, val) {
		if(!strcmp(key, var)) {
			switch (json_object_get_type(val)) {
			case json_type_object:
				break;
			case json_type_array:
				break;
			case json_type_string:
				sprintf(result, "%s", json_object_get_string(val));
				break;
			case json_type_boolean:
				sprintf(result, "%d", json_object_get_boolean(val));
				break;
			case json_type_int:
				sprintf(result, "%d", json_object_get_int(val));
				break;
			default:
				break;
			}
		}
	}
	json_object_put(obj);

	if (strlen(result))
		return strdup(result);
	else
		return NULL;
}

static Client clients[128];
static int cno;

static int add_json_element(const char *key, json_object *obj);
static int add_json_client_element(const char *key, json_object *obj);

static int add_json_object(json_object *obj)
{
	int ret = 0;

	json_object_object_foreach(obj, key, val) {
		if (strstr(key, "client-")) {
			cno++;
			ret = add_json_element(key, val);
		} else if (!strcmp(key, "wireless") || !strcmp(key, "wdev") || !strcmp(key, "hostname") || !strcmp(key, "macaddr") || !strcmp(key, "connected")) {
			ret = add_json_client_element(key, val);
		}
		if (ret)
			break;
	}

	return ret;
}

static int add_json_client_element(const char *key, json_object *obj)
{
	char *type;

	if (!obj)
		return -1;

	if (!strcmp(key, "hostname")) {
		strcpy(clients[cno].hostname, json_object_get_string(obj));
	} else if (!strcmp(key, "macaddr")) {
		strcpy(clients[cno].macaddr, json_object_get_string(obj));
	} else if (!strcmp(key, "connected")) {
		clients[cno].conntype = json_object_get_boolean(obj);
	} else if (!strcmp(key, "wireless")) {
		if (json_object_get_boolean(obj))
			clients[cno].conntype = 2;
	} else if (!strcmp(key, "wdev")) {
		strcpy(clients[cno].wdev, json_object_get_string(obj));
	}

	return 0;
}

static int add_json_element(const char *key, json_object *obj)
{
	char *type;

	if (!obj)
		return -1;

	switch (json_object_get_type(obj)) {
	case json_type_object:
		add_json_object(obj);
		break;
	default:
		return -1;
	}

	return 0;
}

void
json_parse_clients(Client *clnt)
{
	json_object *obj;
	FILE *in;
	char clist[50000];

	cno = -1;
	memset(clients, '\0', sizeof(clients));

	if ((in = popen("ubus -S call router clients", "r")))
		fgets(clist, sizeof(clist), in);
	pclose(in);

	if(strlen(clist) < 64)
		return;

	obj = json_tokener_parse(clist);
	if (is_error(obj) || json_object_get_type(obj) != json_type_object) {
		fprintf(stderr, "Failed to parse message data\n");
		return;
	}
	add_json_object(obj);
	json_object_put(obj);

	memcpy(clnt, clients, sizeof(clients));
}
