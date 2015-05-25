typedef struct {
	int conntype;
	char hostname[128];
	char macaddr[24];
	char wdev[8];
} Client;

void ubus_listener();
void wps_event(const char *key, const char *val);
void removeNewline(char *buf);
const char *chrCmd(const char *pFmt, ...);

const char *json_parse_and_get(const char *str, char *var);
void json_parse_clients(Client *clnt);
