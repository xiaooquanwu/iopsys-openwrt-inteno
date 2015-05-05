#include <ctype.h>
#include <syslog.h>
#include "log.h"
#include "led.h"

static struct blob_buf bblob;

typedef enum {
    LED_OFF,
    LED_OK,
    LED_EOK,
    LED_NOTICE,
    LED_ALERT,
    LED_ERROR,
    LED_CUSTOM,
    LED_ACTION_MAX,
} led_action_t;

typedef enum {
    LEDS_NORMAL,
    LEDS_PROXIMITY,
    LEDS_SILENT,
    LEDS_INFO,
    LEDS_TEST,
    LEDS_PROD,
    LEDS_RESET,
    LEDS_ALLON,
    LEDS_ALLOFF,
    LEDS_MAX,
} leds_state_t;

/* Names for led_action_t */
static const char * const fn_actions[LED_ACTION_MAX] =
{ "off", "ok", "eok", "notice", "alert", "error", "custom"};

/* Names for led_state_t */
static const char* const led_states[LED_STATES_MAX] =
{ "off", "on", "flash_slow", "flash_fast","pulsing", "fadeon", "fadeoff" };

/* Names for leds_state_t */
static const char* const leds_states[LEDS_MAX] =
{ "normal", "proximity", "silent", "info", "test", "production", "reset", "allon" , "alloff"};

/* lowest level, contain states, timers,pointer to driver for a single physical led.*/
struct led {
	struct list_head list;
	led_state_t state;		/* state that this led should have, set from the config file */
	struct led_drv *drv;
};

struct super_functions {
	struct list_head list;
	led_action_t state;		/* state that the function need to match */
	struct function_led *function;
};

struct super_list {
	struct list_head list;
	struct list_head sf_list;	/* this list contains states that needs to match for this super fuction action to be active */
};

/*middle layer contains lists of leds /buttons/... that should be set to a specific state */
struct function_action {
	const char *name;		/* If name is set this led action is in use by the board. */
	struct list_head led_list;
	struct list_head button_list;
	struct list_head super_list;    /* list of super function lists */
};

/* main struct for the function leds.*/
struct function_led {
	const char *name;		/* If name is set this led function is in use by the board.        */
	led_action_t state;		/* state of the function led. contain what action is currently set */
	int press_indicator;		/* record if this is part of press indictor */
	struct function_action actions[LED_ACTION_MAX];
};



struct function_led *leds;		/* Array of functions, LED_FUNCTIONS + super_functions */
static int total_functions;		/* number of entries in leds array */

static leds_state_t global_state;	/* global state for the leds,overrids individual states */
static leds_state_t press_state;	/* global state for the press indicator */

int get_index_by_name(const char *const*array, int max, const char *name);
int get_index_for_function(const char *name);
struct led_drv *get_drv_led(char *name);
static void dump_drv_list(void);
static void dump_led(void);
static void all_leds_off(void);
static void all_leds_on(void);
static void all_leds(led_state_t state);
static const char * get_function_action( const char *s, struct function_led **function, int *action);
static void super_update(void);

/* we find out the index for a match in an array of char pointers containing max number of pointers */
int get_index_by_name(const char *const*array, int max, const char *name)
{
	int i;
	for (i=0; i < max ; i++ ){
		if (!strcasecmp(name, array[i]))
			return i;
	}
	return -1;
}

int get_index_for_function(const char *name)
{
	int i;
	for (i=0 ; i < total_functions; i++) {
		if (!strcasecmp(name, leds[i].name))
			return i;
	}
	return -1;
}

/* PUT every led from drivers into a list */
struct drv_led_list{
	struct list_head list;
	struct led_drv *drv;
};
LIST_HEAD(drv_leds_list);

void led_add( struct led_drv *drv)
{
	struct drv_led_list *drv_node = malloc(sizeof(struct drv_led_list));

	DBG(1,"called with led name [%s]", drv->name);
	drv_node->drv = drv;

	list_add(&drv_node->list, &drv_leds_list);
}

static void all_leds(led_state_t state) {
	struct drv_led_list *node;
	DBG(1, "set to state %d",state);

	list_for_each_entry(node, &drv_leds_list, list) {
		node->drv->func->set_state( node->drv, state);
	}
}

static void all_leds_off(void) {
	all_leds(OFF);
}

static void all_leds_on(void) {
	all_leds(ON);
}

#define TEST_TIMEOUT 250
static void test_handler(struct uloop_timeout *timeout);
static struct uloop_timeout test_inform_timer = { .cb = test_handler };

static void test_handler(struct uloop_timeout *timeout) {

	static int cnt = 0;
	static led_state_t state = OFF;

	static struct drv_led_list *led;
	DBG(1,"cnt = %d state %d",cnt,state);

	/* flash all leads 2 times.*/
	if ( cnt < 4) {
		cnt++;
		if (state == OFF){
			all_leds_on();
			state = ON;
		}else{
			all_leds_off();
			state = OFF;
		}
		goto done;
	}

	if (global_state == LEDS_RESET){
		cnt = 0;
		goto done;
	}

	/* cycle through every led once */
	if (cnt == 4 ) {
		cnt++;
		led = list_first_entry(&drv_leds_list, struct drv_led_list, list );
	}
	if (cnt == 5 ) {
		if (state == OFF){
			led->drv->func->set_state(led->drv, ON);
			state = ON;
		}else{
			led->drv->func->set_state(led->drv, OFF);
			/* was this the last led ? if so stop */
			if ( list_is_last(&led->list, &drv_leds_list) ){
				cnt = 0;
				state = OFF;
				goto done;
			}else{ /* nope more leds in list. get next and turn it on */
				led = (struct drv_led_list *)led->list.next;
				led->drv->func->set_state(led->drv, ON);
				state = ON;
			}
		}
	}
done:

    if (global_state == LEDS_TEST || global_state == LEDS_RESET)
	    uloop_timeout_set(&test_inform_timer, TEST_TIMEOUT);
    else{
	    cnt = 0;
	    state = OFF;
    }
}

/* go over the driver list for any led name that matches name and returna pointer to driver. */
struct led_drv *get_drv_led(char *name)
{
	struct list_head *i;
	list_for_each(i, &drv_leds_list) {
		struct drv_led_list *node = list_entry(i, struct drv_led_list, list);
		if (!strcmp(node->drv->name, name))
			return node->drv;
	}
	return NULL;
}

static void dump_drv_list(void)
{
	struct list_head *i;
	list_for_each(i, &drv_leds_list) {
		struct drv_led_list *node = list_entry(i, struct drv_led_list, list);
		DBG(1,"led name = [%s]",node->drv->name);
	}
}

static void dump_led(void)
{
	int i,j;
	for (i = 0; i < total_functions ; i++) {
		for (j = 0 ; j < LED_ACTION_MAX; j++ ) {
			if ( leds[i].actions[j].name != NULL ) {
				struct led *led;
				struct super_list *sl;

				/* print out action list */
				list_for_each_entry(led, &leds[i].actions[j].led_list, list) {
					DBG(1,"%-15s %-8s %-15s %-10s",
					    leds[i].name,
					    leds[i].actions[j].name,
					    led->drv->name,
					    led_states[led->state]);
				}
				/* print out super function list */
				list_for_each_entry(sl, &leds[i].actions[j].super_list, list) {
					struct super_functions *sf;
					DBG(1,"   AND list");
					list_for_each_entry(sf, &sl->sf_list, list) {
						DBG(1,"\tfunction [%s] action [%s]",sf->function->name, fn_actions[sf->state]);
					}
				}
			}
		}
	}
}

/* loop over every function, if it is a super function update the state */
static void super_update(void)
{
	int i,j;
	for (i = 0; i < total_functions ; i++) {
		for (j = 0 ; j < LED_ACTION_MAX; j++ ) {
			if ( leds[i].actions[j].name != NULL ) {
				struct super_list *sl;
				list_for_each_entry(sl, &leds[i].actions[j].super_list, list) {
					struct super_functions *sf;
					int status = 0;
//					DBG(1,"   AND list");
					list_for_each_entry(sf, &sl->sf_list, list) {
//						DBG(1,"\tfunction [%s] action [%s]",sf->function->name, fn_actions[sf->state]);
						if (sf->function->state == sf->state ) {
							status = 1;
						} else {
							status = 0;
							break;
						}
					}
					if (status){
						leds[i].state = j;
						DBG(1,"\tSet super function [%s] to action [%s]",leds[i].name, fn_actions[j]);
					}
				}
			}
		}
	}
}

/* return 0 = OK, -1 = error */
static int set_function_led(const char* fn_name, const char* action) {
	int led_idx = get_index_for_function(fn_name);
	int act_idx = get_index_by_name(fn_actions   , LED_ACTION_MAX, action );
	struct led *led;

	if(led_idx == -1) {
		syslog(LOG_WARNING, "called over ubus with non valid led name [%s]", fn_name);
		return -1;
	}
	if(act_idx == -1) {
		syslog(LOG_WARNING, "called over ubus with non valid action [%s] for led [%s]", action, fn_name);
		return -1;
	}

	leds[led_idx].state = act_idx;

	list_for_each_entry(led, &leds[led_idx].actions[act_idx].led_list, list) {
		if (led->drv){
			led->drv->func->set_state(led->drv, led->state);
		}
	}

	return 0;
}

enum {
	LED_STATE,
	__LED_MAX
};

static const struct blobmsg_policy led_policy[] = {
	[LED_STATE] = { .name = "state", .type = BLOBMSG_TYPE_STRING },
};

static int led_set_method(struct ubus_context *ubus_ctx, struct ubus_object *obj,
                          struct ubus_request_data *req, const char *method,
                          struct blob_attr *msg)
{
	struct blob_attr *tb[__LED_MAX];
	char* state;

	blobmsg_parse(led_policy, ARRAY_SIZE(led_policy), tb, blob_data(msg), blob_len(msg));
	if (tb[LED_STATE]) {
		char *fn_name = strchr(obj->name, '.') + 1;
		state = blobmsg_data(tb[LED_STATE]);
		DBG(1,"set led [%s]->[%s]", fn_name, state);
//		syslog(LOG_INFO, "Led %s method: %s state %s", fn_name, method, state);
		if (set_function_led(fn_name, state) ){
			return UBUS_STATUS_NO_DATA;
		}
	}else {
		DBG(1,"(%s) not implemented",method);
		return UBUS_STATUS_NO_DATA;
	}
	return 0;
}

static int led_status_method(struct ubus_context *ubus_ctx, struct ubus_object *obj,
                             struct ubus_request_data *req, const char *method,
                             struct blob_attr *msg)
{
	char *fn_name = strchr(obj->name, '.') + 1;
	int led_idx = get_index_for_function(fn_name);
	DBG(1,"for led %s",leds[led_idx].name);

	blob_buf_init (&bblob, 0);
	blobmsg_add_string(&bblob, "state",fn_actions[leds[led_idx].state]);
	ubus_send_reply(ubus_ctx, req, bblob.head);

	return 0;
}

static int leds_set_method(struct ubus_context *ubus_ctx, struct ubus_object *obj,
                           struct ubus_request_data *req, const char *method,
                           struct blob_attr *msg)
{
	struct blob_attr *tb[__LED_MAX];

	blobmsg_parse(led_policy, ARRAY_SIZE(led_policy), tb, blob_data(msg), blob_len(msg));

	if (tb[LED_STATE]) {
		char* state;
		int state_idx;

		state = blobmsg_data(tb[LED_STATE]);
		state_idx = get_index_by_name(leds_states, LEDS_MAX , state);

		if (state_idx == -1) {
			syslog(LOG_WARNING, "leds_set_method: Unknown state %s.", state);
			return 0;
		}

		global_state = state_idx;

		if (global_state == LEDS_INFO) {
			all_leds_off();
		}

		if (global_state == LEDS_PROD) {
			all_leds_off();
		}

		if (global_state == LEDS_TEST || global_state == LEDS_RESET) {
			all_leds_off();
			uloop_timeout_set(&test_inform_timer, TEST_TIMEOUT);
		}
		if (global_state == LEDS_ALLON) {
			all_leds_on();
		}
		if (global_state == LEDS_ALLOFF) {
			all_leds_off();
		}

		DBG(1,"led global state set to [%s] wanted [%s]", leds_states[global_state], state);
	}else
		syslog(LOG_WARNING, "Unknown attribute [%s]", (char *)blob_data(msg));

	return 0;
}

static int leds_status_method(struct ubus_context *ubus_ctx, struct ubus_object *obj,
                              struct ubus_request_data *req, const char *method,
                              struct blob_attr *msg)
{

	blob_buf_init (&bblob, 0);
	blobmsg_add_string(&bblob, "state", leds_states[global_state]);
	DBG(1,"leds global state is [%s]",leds_states[global_state]);
	ubus_send_reply(ubus_ctx, req, bblob.head);
	return 0;
}

static const struct ubus_method led_methods[] = {
	UBUS_METHOD("set", led_set_method, led_policy),
	{ .name = "status", .handler = led_status_method },
};

static struct ubus_object_type led_object_type =
	UBUS_OBJECT_TYPE("led", led_methods);

static const struct ubus_method leds_methods[] = {
	UBUS_METHOD("set", leds_set_method, led_policy),
	{ .name = "status", .handler = leds_status_method },
//    { .name = "proximity", .handler = leds_proximity_method },
};

static struct ubus_object_type leds_object_type =
	UBUS_OBJECT_TYPE("leds", leds_methods);

#define LED_OBJECTS 1
static struct ubus_object led_objects[LED_OBJECTS] = {
    { .name = "leds",	        .type = &leds_object_type, .methods = leds_methods, .n_methods = ARRAY_SIZE(leds_methods), },
};


#define FLASH_TIMEOUT 250
static void flash_handler(struct uloop_timeout *timeout);
static struct uloop_timeout flash_inform_timer = { .cb = flash_handler };

static void flash_handler(struct uloop_timeout *timeout)
{
	static int counter = 1; /* bit 0 is fast flash bit 2 is slow flash */
	int i;
	led_state_t slow=OFF,fast=OFF;
	counter++;

	if (counter & 1 )
		fast = ON;
	if (counter & 4 )
		slow = ON;

	super_update();

	if (global_state == LEDS_NORMAL ||
	    global_state == LEDS_INFO ) {
		for (i = 0; i < total_functions ; i++) {
			struct led *led;
			if (leds[i].press_indicator & press_state) {
//				DBG(1,"INDICATE_PRESS on %s",leds[i].name);
				list_for_each_entry(led, &leds[i].actions[leds[i].state].led_list, list) {
					if (led->drv)
						led->drv->func->set_state(led->drv, fast);
				}

				/* normal operation, flash else reset state */
			} else {
				led_action_t action_state = leds[i].state;

				/* in case of info mode suppress OK state. that is if OK -> turn it into OFF */
				if (global_state == LEDS_INFO) {
					if (action_state == LED_OK)
						action_state = LED_OFF;
				}
				list_for_each_entry(led, &leds[i].actions[action_state].led_list, list) {

					if (led->state == FLASH_FAST) {
						if (led->drv) {
							if (led->drv->func->support) {
								if (led->drv->func->support(led->drv, FLASH_FAST)) {
									/* hardware support flash */
									led->drv->func->set_state(led->drv, FLASH_FAST);
									continue;
								}
							}
							/* emulate flash with on/off */
							led->drv->func->set_state(led->drv, fast);
						}
					}else if (led->state == FLASH_SLOW) {
						if (led->drv) {
							if (led->drv->func->support) {
								if (led->drv->func->support(led->drv, FLASH_SLOW)) {
									/* hardware support flash */
									led->drv->func->set_state(led->drv, FLASH_SLOW);
									continue;
								}
							}
							/* emulate flash with on/off */
							led->drv->func->set_state(led->drv, slow);
						}
					}else{
						if (led->drv)
							led->drv->func->set_state(led->drv, led->state);
					}
				}
			}
		}
	}
	uloop_timeout_set(&flash_inform_timer, FLASH_TIMEOUT);
}

void led_pressindicator_set(void){
	press_state = 1;
}

void led_pressindicator_clear(void){
	press_state = 0;
}

/*
  input: s, string of comma separated function_action names, 'wifi_ok, wps_ok'

  return:
	NULL if no valid function actionpair found.
	pointer to start of unused part of string.

	fills in the function pointer with NULL or a real function
	fills in action with 0 or real index (include 0),

*/
const char * get_function_action( const char *s, struct function_led **function, int *action)
{
	const char *first, *last, *end, *p;
	char func[100], act[20];

	DBG(1,"start [%s]", s);

	*function = NULL;
	*action = 0;

	/* if string is zero length give up. */
	if ( 0 == strlen(s))
		return NULL;

	end = s + strlen(s);

	/* find first alpha char */
	while (!isalnum(*s)){
		s++;
		if (s == end)
			return NULL;
	}
	first = s;

	/* find , or end of string or space/tab */
	while ( (*s != ',') && (!isblank(*s))) {
		s++;
		if (s == end)
			break;
	}
	last = s;

	/* scan backwards for _ */
	while (*s != '_' && s > first){
		s--;
	}

	/* if we could not find a _ char bail out */
	if (*s != '_')
		return NULL;

	/* extract function name */
	p = first;
	while (p != s) {
		func[p-first] = *p;
		p++;
	}
	func[p-first] = 0;

	/* extract action name */
	p = s + 1;
	while (p != last) {
		act[p-(s+1)] = *p;
		p++;
	}
	act[p-(s+1)] = 0;

	DBG(1,"function[%s] action[%s] func_idx %d", func, act, get_index_for_function(func));
	*function = &leds[get_index_for_function(func)];
	*action = get_index_by_name(fn_actions   , LED_ACTION_MAX, act );

	if (*last == ',')
		last++;

	return last;
}

void led_init( struct server_ctx *s_ctx)
{
	int i,j;
	LIST_HEAD(led_map_list);
	struct ucilist *map_node;

	dump_drv_list();

	/* register leds with ubus */
	for (i=0 ; i<LED_OBJECTS ; i++) {
		int ret = ubus_add_object(s_ctx->ubus_ctx, &led_objects[i]);
		if (ret)
			DBG(1,"Failed to add object: %s", ubus_strerror(ret));
	}

	/* read out the function leds */
	ucix_get_option_list( s_ctx->uci_ctx, "hw", "led_map", "functions" , &led_map_list);

	total_functions = 0;
	list_for_each_entry(map_node, &led_map_list, list) {
		total_functions++;
	}

	leds = malloc(sizeof(struct function_led) * total_functions);
	memset(leds, 0, sizeof(struct function_led) * total_functions);

	/* set function name & regiter with ubus */
	i = 0;
	list_for_each_entry(map_node, &led_map_list, list) {
		char name[100];
		int ret;
		struct ubus_object *ubo;
		ubo = malloc(sizeof(struct ubus_object));
		memset(ubo, 0, sizeof(struct ubus_object));

		leds[i].name = strdup(map_node->val);

		sprintf(name, "led.%s", leds[i].name);
		ubo->name      = strdup(name);
		ubo->methods   = led_methods;
		ubo->n_methods = ARRAY_SIZE(led_methods);
		ubo->type      = &led_object_type;

		ret = ubus_add_object(s_ctx->ubus_ctx, ubo);
		if (ret)
			DBG(1,"Failed to add object: %s", ubus_strerror(ret));
		i++;
	}

	/* we create a top list of led functions */
	/* that list contains a new list of actions */
	/* every action contains led actions lists */
	/* the led states is attached to the drv_leds lists */

	for (i = 0; i < total_functions ; i++) {
		for (j = 0 ; j < LED_ACTION_MAX; j++ ) {
			char led_fn_name[256];
			char led_action[256];

			LIST_HEAD(led_action_list);
			struct ucilist *node;
			snprintf(led_fn_name, 256, "led_%s", leds[i].name);
			snprintf(led_action, 256, "led_action_%s", fn_actions[j]);
			ucix_get_option_list( s_ctx->uci_ctx, "hw", led_fn_name, led_action , &led_action_list);

			INIT_LIST_HEAD( &leds[i].actions[j].led_list );

			if (!list_empty(&led_action_list)) {

				/* Found led with action, init structs */
				leds[i].state = LED_OFF;

				leds[i].actions[j].name    = fn_actions[j];

				/* fill in led actions */
				DBG(2,"%-15s has led actions %s -> ", led_fn_name, fn_actions[j]);
				list_for_each_entry(node, &led_action_list, list) {
					char led_name[256],led_state[256];
					struct led *led;
					struct led_drv *drv;
					char *c;
					char *s = strdup(node->val);
					led_name[0]=0;
					led_state[0]=0;

					/* get pointer to low level led driver. by removing the = sign and
					   storing the remaining two strings.
					*/
					c = strchr(s,'=');
					if( c == NULL)
						continue;	/* no = found, abort */
					*c = ' ';

					sscanf(s, "%s %s", led_name, led_state);
					drv = get_drv_led(led_name);

					if (drv) {
						led = malloc(sizeof(struct led));
						led->drv = drv;
						led->state = get_index_by_name(led_states, LED_STATES_MAX, led_state);
						list_add(&led->list, &leds[i].actions[j].led_list);
					}else {
						syslog(LOG_ERR,"Config specified use of led name [%s]. But it's not registerd with a led driver.", led_name);
					}
					DBG(2, "%-35s%s","",node->val);
					free(s);
				}

				/* fill in button actions */

				/* fill in xxx actions */
			}else {
				DBG(2,"%-15s has no actions -> %s", led_fn_name, fn_actions[j]);
			}
		}
	}

	/* Read in functions that have function list (super functions) */
	for (i = 0; i < total_functions ; i++) {
		for (j = 0 ; j < LED_ACTION_MAX; j++ ) {
			char led_fn_name[256];
			char super_action[256];
			LIST_HEAD(super_action_list);
			struct ucilist *node;
			snprintf(led_fn_name, 256, "led_%s", leds[i].name);
			snprintf(super_action, 256, "super_%s", fn_actions[j]);
			ucix_get_option_list( s_ctx->uci_ctx, "hw", led_fn_name, super_action , &super_action_list);
			INIT_LIST_HEAD( &leds[i].actions[j].super_list );

			if (!list_empty(&super_action_list)) {
				DBG(1,"A:%s %s is a super function ",led_fn_name,super_action);

				list_for_each_entry(node, &super_action_list, list) {
					char *s;
					struct function_led *function;
					int action_ix;
					struct super_list *sl = malloc(sizeof(struct super_list));
					memset(sl, 0, sizeof(struct super_list));
					list_add(&sl->list, &leds[i].actions[j].super_list);
					INIT_LIST_HEAD( &sl->sf_list );

					DBG(1,"add to super list  %s",node->val);

					s = node->val;
					while (s = get_function_action(s, &function, &action_ix)){
						if (function) {
							struct super_functions *sf = malloc(sizeof(struct super_functions));
							memset(sf, 0, sizeof(struct super_functions));
							sf->state = action_ix;
							sf->function = function;
							list_add(&sf->list, &sl->sf_list);
							DBG(1,"C %s %s",function->name, fn_actions[action_ix]);
						}
					}
				}
			}else
				DBG(1,"A:%s %s is a normal function ",led_fn_name,super_action);
		}
	}

	{
		struct ucilist *node;
		/* read function buttons from section button_map */
		LIST_HEAD(press_indicator);

		/* read in generic configuration. press indicator list, dimm list default params..... */

		ucix_get_option_list(s_ctx->uci_ctx, "hw" ,"led_map", "press_indicator", &press_indicator);
		list_for_each_entry(node, &press_indicator, list) {
			char *s;
			int ix;
			s = node->val;
			s +=4;	/*remove 'led_' from string */
			DBG(1,"press indicator %s [%s]",node->val, s);
			ix  = get_index_for_function(s);
//			DBG(1,"press indicator %s [%s]->%d",node->val, s, ix);
			leds[ix].press_indicator = 1;
		}
	}
	uloop_timeout_set(&flash_inform_timer, FLASH_TIMEOUT);

	dump_led();
	all_leds_off();
}

