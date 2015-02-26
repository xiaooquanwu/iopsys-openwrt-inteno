#include <stdio.h>
#include "log.h"

#include "server.h"

#include "libubus.h"
#include <uci_config.h>
#include <uci.h>
#include "ucix.h"

typedef enum {
        STATE_OFF,
        STATE_OK,
        STATE_NOTICE,
        STATE_ALERT,
        STATE_ERROR,
}state_t;

state_t state;

static struct ubus_context *ubus_ctx;
static char *ubus_socket;

#define CATV_MONITOR_TIME (1000 * 10) /* 10 sec in ms */

void catv_monitor_init(struct server_ctx *s_ctx);
void catv_monitor_set_socket(char *socket_name);

static void set_led(state_t state);
static int is_enabled(void);

static void catv_monitor_handler(struct uloop_timeout *timeout);
struct uloop_timeout catv_monitor_timer = { .cb = catv_monitor_handler };

static void
catv_status_cb(struct ubus_request *req, int type, struct blob_attr *msg)
{
	struct blob_attr *cur;
	uint32_t rem;
	const char *data;

	rem = blob_len(msg);

	__blob_for_each_attr(cur, blob_data(msg), rem) {
                if (!strcmp("RF enable", blobmsg_name(cur))) {
                        data = blobmsg_data(cur);
                        if (!strncasecmp("on", data, 2))
                                *(int*)req->priv = 1;
                                return;
                }
        }
        *(int*)req->priv = 0;
        return;
}

static int is_enabled(void)
{
	uint32_t id;
        struct blob_buf b;
        int enabled = 0;
        int ret;

        if (ubus_lookup_id(ubus_ctx, "catv", &id)) {
                DBG(1, "Failed to look up catv object\n");
                return 0;
        }

        memset(&b, 0, sizeof(struct blob_buf));
        blob_buf_init(&b, 0);

        ret = ubus_invoke(ubus_ctx, id, "status", b.head, catv_status_cb, &enabled, 3000);

        if (ret)
                DBG(1,"catv_monitor: ret = %s", ubus_strerror(ret));

        return enabled;
}

static void
catv_vpd_cb(struct ubus_request *req, int type, struct blob_attr *msg)
{
	struct blob_attr *cur;
	uint32_t rem;
	const char *data = "-inf";

	rem = blob_len(msg);

        /* no response */
        if ( rem == 0 ) {
                state = STATE_ERROR;
                return;
        }

	__blob_for_each_attr(cur, blob_data(msg), rem) {
                if (!strcmp("VPD", blobmsg_name(cur))) {
                        data = blobmsg_data(cur);
                }
        }

        /* no cable */
        if (!strcmp("-inf", data)) {
                state = STATE_ERROR;
        }
}

static void
catv_alarm_cb(struct ubus_request *req, int type, struct blob_attr *msg)
{
	struct blob_attr *cur;
	uint32_t rem;
	const char *data = "-inf";

	rem = blob_len(msg);

        /* no response */
        if ( rem == 0 ) {
                state = STATE_ERROR;
                return;
        }

	__blob_for_each_attr(cur, blob_data(msg), rem) {
                if (!strcasecmp("Alarm VPD HI", blobmsg_name(cur))) {
                        data = blobmsg_data(cur);
                        /* if on  */
                        if (!strcmp("on", data)) {
                                state = STATE_ALERT;
                        }
                }else if (!strcasecmp("Alarm VPD LO", blobmsg_name(cur))) {
                        data = blobmsg_data(cur);
                        /* if on  */
                        if (!strcmp("on", data)) {
                                state = STATE_ALERT;
                        }
                }else if (!strcasecmp("Warning VPD HI", blobmsg_name(cur))) {
                        data = blobmsg_data(cur);
                        /* if on  */
                        if (!strcmp("on", data)) {
                                state = STATE_NOTICE;
                        }
                }else if (!strcasecmp("Warning VPD LO", blobmsg_name(cur))) {
                        data = blobmsg_data(cur);
                        /* if on  */
                        if (!strcmp("on", data)) {
                                state = STATE_NOTICE;
                        }
                }
        }
}


static void catv_monitor_handler(struct uloop_timeout *timeout)
{
	uint32_t id;
        struct blob_buf b;
        int ret;

        /* start to set new state to OK */
        /* then checks turn on different errors */
        state = STATE_OK;

        if (is_enabled()) {

                if (ubus_lookup_id(ubus_ctx, "catv", &id)) {
                        DBG(1, "Failed to look up catv object\n");
                        return;
                }

                memset(&b, 0, sizeof(struct blob_buf));
                blob_buf_init(&b, 0);

                /* first check alarms, they set notice/alert if present */
                ret = ubus_invoke(ubus_ctx, id, "alarm", b.head, catv_alarm_cb, 0, 3000);
                if (ret)
                        DBG(1,"ret = %s", ubus_strerror(ret));

                /* then check cable in, it sets Error,*/
                ret = ubus_invoke(ubus_ctx, id, "vpd", b.head, catv_vpd_cb, 0, 3000);
                if (ret)
                        DBG(1,"ret = %s", ubus_strerror(ret));
        }else
                state = STATE_OFF;

        set_led(state);

        uloop_timeout_set(&catv_monitor_timer, CATV_MONITOR_TIME);
}

static void set_led(state_t lstate)
{
	uint32_t id;
        struct blob_buf b;
        int ret;
        static state_t old_state = -1;

        if ( lstate == old_state )
                return;
        old_state = lstate;

        memset(&b, 0, sizeof(struct blob_buf));
	blob_buf_init(&b, 0);

        switch (lstate) {
        case STATE_OFF:
                blobmsg_add_string(&b, "state", "off"); break;
        case STATE_OK:
                blobmsg_add_string(&b, "state", "ok"); break;
        case STATE_NOTICE:
                blobmsg_add_string(&b, "state", "notice"); break;
        case STATE_ALERT:
                blobmsg_add_string(&b, "state", "alert"); break;
        case STATE_ERROR:
                blobmsg_add_string(&b, "state", "error"); break;
        }

        if (ubus_lookup_id(ubus_ctx, "led.ext", &id)) {
		DBG(1, "Failed to look up led.ext object\n");
		return;
	}

	ret = ubus_invoke(ubus_ctx, id, "set", b.head, NULL, 0, 3000);

        if ( ret )
                DBG(1,"catv_moitor: set led failed [%s]", ubus_strerror(ret));
}

void catv_monitor_init(struct server_ctx *s_ctx)
{
        ubus_ctx = ubus_connect(ubus_socket);
        if (!ubus_ctx) {
                syslog(LOG_ERR,"catv monitor: Failed to connect to ubus\n");
                return;
        }

        /* start monitor timer */
        uloop_timeout_set(&catv_monitor_timer, CATV_MONITOR_TIME);
}

void catv_monitor_set_socket(char *socket_name)
{
        if (socket_name)
                ubus_socket = strdup(socket_name);
}
