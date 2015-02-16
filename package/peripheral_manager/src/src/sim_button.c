#include <syslog.h>
#include <string.h>
#include "button.h"
#include "log.h"
#include "server.h"


void sim_button_init(struct server_ctx *s_ctx);

struct sim_data {
	int addr;
	int active;
	int state;
	struct button_drv button;
};

struct drv_button_list{
	struct list_head list;
	struct button_drv *drv;
};

static button_state_t sim_get_state(struct button_drv *drv)
{
//	DBG(1, "state for %s",  drv->name);
	struct sim_data *p = (struct sim_data *)drv->priv;

	return p->state;
}

static struct button_drv_func func = {
	.get_state = sim_get_state,
};

static LIST_HEAD(sim_buttons);


enum {
	SIM_NAME,
	SIM_STATE,
	__SIM_MAX
};

static const struct blobmsg_policy sim_policy[] = {
	[SIM_NAME] =  { .name = "name",  .type = BLOBMSG_TYPE_STRING },
	[SIM_STATE] = { .name = "state", .type = BLOBMSG_TYPE_STRING },
};

static struct button_drv *get_drv_button(char *name)
{
	struct list_head *i;
	list_for_each(i, &sim_buttons) {
		struct drv_button_list *node = list_entry(i, struct drv_button_list, list);
                if (! strcmp(node->drv->name, name))
                        return node->drv;
	}
        return NULL;
}

static int sim_set_method(struct ubus_context *ubus_ctx, struct ubus_object *obj,
                          struct ubus_request_data *req, const char *method,
                          struct blob_attr *msg)
{
	struct blob_attr *tb[__SIM_MAX];
	DBG(1, "");

	blobmsg_parse(sim_policy, ARRAY_SIZE(sim_policy), tb, blob_data(msg), blob_len(msg));

	if ( tb[SIM_NAME] ) {
		if ( tb[SIM_STATE] ) {
			struct button_drv *bt = get_drv_button((char *)blobmsg_data(tb[SIM_NAME]));
			DBG(1," name = %s",(char *)blobmsg_data(tb[SIM_NAME]));
			DBG(1," state = %s",(char *)blobmsg_data(tb[SIM_STATE]));

			if (bt) {
				struct sim_data *p = (struct sim_data *)bt->priv;

				if(!strcasecmp("pressed", (char *)blobmsg_data(tb[SIM_STATE]))){
					p->state = PRESSED;
				}
				if(!strcasecmp("released", (char *)blobmsg_data(tb[SIM_STATE]))){
					p->state = RELEASED;
				}
			}else
			DBG(1," button = %s not found",(char *)blobmsg_data(tb[SIM_NAME]));
		}
	}
	return 0;
}

static int sim_status_method(struct ubus_context *ubus_ctx, struct ubus_object *obj,
                             struct ubus_request_data *req, const char *method,
                             struct blob_attr *msg)
{
	struct blob_buf blob;
	struct list_head *i;

	DBG(1, "");

	memset(&blob,0,sizeof(struct blob_buf));
	blob_buf_init(&blob, 0);

	list_for_each(i, &sim_buttons) {
		struct drv_button_list *node = list_entry(i, struct drv_button_list, list);
		const char *state;
		struct sim_data *p = (struct sim_data *)node->drv->priv;

		if(p->state == 	RELEASED)
			state = "Released";
		else
			state = "Pressed";

		blobmsg_add_string(&blob, node->drv->name, state);
	}
	ubus_send_reply(ubus_ctx, req, blob.head);
	return 0;
}


static const struct ubus_method sim_methods[] = {
	UBUS_METHOD("set", sim_set_method, sim_policy),
	{ .name = "status",
	  .handler = sim_status_method },
};

static struct ubus_object_type sim_object_type =
	UBUS_OBJECT_TYPE("sim", sim_methods);

#define SIM_OBJECTS 1
static struct ubus_object sim_objects[SIM_OBJECTS] = {
    { .name = "button",
      .type = &sim_object_type,
      .methods = sim_methods,
      .n_methods = ARRAY_SIZE(sim_methods),
    },
};

void sim_button_init(struct server_ctx *s_ctx) {
	int i,ret;
	struct ucilist *node;
	LIST_HEAD(buttons);

	DBG(1, "");

	ucix_get_option_list(s_ctx->uci_ctx, "hw" ,"sim_buttons", "buttons", &buttons);
	list_for_each_entry(node, &buttons, list) {
		struct sim_data *data;
		const char *s;

		DBG(1, "value = [%s]",node->val);

		data = malloc(sizeof(struct sim_data));
		memset(data,0,sizeof(struct sim_data));

		data->button.name = node->val;

		s = ucix_get_option(s_ctx->uci_ctx, "hw" , data->button.name, "addr");
		DBG(1, "addr = [%s]", s);
		if (s){
			data->addr =  strtol(s,0,0);
		}

		s = ucix_get_option(s_ctx->uci_ctx, "hw" , data->button.name, "active");
		data->active = -1;
		if (s){
			if (!strncasecmp("hi",s,2))
				data->active =  1;
			else if (!strncasecmp("low",s,3))
				data->active =  0;

		}
		DBG(1, "active = %d", data->active);

		data->button.func = &func;
		data->button.priv = data;

		button_add(&data->button);

		{ /* save button in internal list, we need this for ubus set/status */
			struct drv_button_list *bt = malloc(sizeof(struct drv_button_list));
			memset(bt, 0, sizeof(struct drv_button_list));
			bt->drv = &data->button;
			list_add(&bt->list, &sim_buttons);
		}
	}

	for (i=0 ; i<SIM_OBJECTS ; i++) {
		ret = ubus_add_object(s_ctx->ubus_ctx, &sim_objects[i]);
		if (ret)
			DBG(1,"Failed to add object: %s", ubus_strerror(ret));
	}
}
