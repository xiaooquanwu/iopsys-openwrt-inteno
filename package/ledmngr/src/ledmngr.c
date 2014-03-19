/*
 * ledmngr.c -- Led and button manager for Inteno CPE's
 *
 * Copyright (C) 2012-2014 Inteno Broadband Technology AB. All rights reserved.
 *
 * Author: benjamin.larsson@inteno.se
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

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <syslog.h>
#include <unistd.h>

#include <libubox/blobmsg_json.h>
#include "libubus.h"

#include <board.h>
#include "ucix.h"

static struct ubus_context *ubus_ctx = NULL;
static struct blob_buf b;

static struct leds_configuration* led_cfg;
static struct button_configuration* butt_cfg;
static struct uci_context *uci_ctx = NULL;

int fd;
extern int daemonize;
#define DEBUG_PRINT(...) if (!daemonize) fprintf( stderr, __VA_ARGS__ );

#define LED_FUNCTIONS 14
#define MAX_LEDS 20
#define SR_MAX 16
#define MAX_BUTTON 10

enum {
    OFF,
    ON,
    BLINK_SLOW,
    BLINK_FAST,
    LED_STATES_MAX,
};

enum {
    RED,
    GREEN,
    BLUE,
    YELLOW,
};

enum {
    GPIO,
    LEDCTL,
    SHIFTREG2,
    SHIFTREG3,
};

enum {
    ACTIVE_HIGH,
    ACTIVE_LOW,
};

enum {
    LED_OK,
    LED_NOTICE,
    LED_ALERT,
    LED_ERROR,
    LED_OFF,
    LED_ACTION_MAX,
};

enum {
    LEDS_NORMAL,
    LEDS_INFO,
    LEDS_TEST,
    LEDS_MAX,
};

struct led_config {
    /* Configuration */
    char*   name;
    char*   function;
    int     color;
    int     type;
    int     address;
    int     active;
    /* State */
    int     state;
    int     blink_state;
} led_config;

struct led_action {
    int     led_index;
    int     led_state;
} led_action;

struct led_map {
    char*   led_function;
    char*   led_name;
    int     led_actions_nr;
    struct led_action led_actions[LED_ACTION_MAX];
};


static char* fn_actions[LED_ACTION_MAX] = { "ok", "notice", "alert", "error", "off",};
static char* led_functions[LED_FUNCTIONS] = { "dsl", "wifi", "wps", "lan", "status", "dect", "tv", "usb", "wan", "internet", "voice1", "voice2", "eco", "gbe"};
static char* led_states[LED_STATES_MAX] = { "off", "on", "blink_slow", "blink_fast" };
static char* leds_states[LEDS_MAX] = { "normal", "info", "test" };

struct leds_configuration {
    int             leds_nr;
    struct led_config**  leds;
    int fd;
    int shift_register_state[SR_MAX];
    int led_fn_action[LED_FUNCTIONS];
    struct led_map led_map_config[LED_FUNCTIONS][LED_ACTION_MAX];
    int leds_state;
    int test_state;
} leds_configuration;

struct button_config {
    char*   name;
    int     address;
    int     active;
    char*   command;
    int     pressed_state;
};

struct button_configuration {
    int         button_nr;
    struct button_config**  buttons;
    
} button_configuration;



static int get_led_index_by_name(struct leds_configuration* led_cfg, char* led_name);
static int led_set(struct leds_configuration* led_cfg, int led_idx, int state);

static int add_led(struct leds_configuration* led_cfg, char* led_name, const char* led_config, int color) {

    if (!led_config) {
//        printf("Led %s: not configured\n",led_name);
        return -1;
    } else {
        struct led_config* lc = malloc(sizeof(struct led_config));
        char type[256];
        char active[256];
        char function[256];
        int  address;

        DEBUG_PRINT("Led %s: %s\n",led_name, led_config);
        lc->name = strdup(led_name);
        // gpio,39,al
        sscanf(led_config, "%s %d %s %s", type, &address, active, function);
//        printf("Config %s,%d,%s,%s\n", type, address, active, function);
        
        if (!strcmp(type, "gpio")) lc->type = GPIO;
        if (!strcmp(type, "sr"))   lc->type = SHIFTREG2;
        if (!strcmp(type, "csr"))  lc->type = SHIFTREG3;

        lc->address = address;
        lc->color = color;

        if (!strcmp(active, "al"))   lc->active = ACTIVE_LOW;
        if (!strcmp(active, "ah"))   lc->active = ACTIVE_HIGH;

        //realloc(led_cfg->leds, (led_cfg->leds_nr+1) * sizeof(struct led_config*));
        if (led_cfg->leds_nr >= MAX_LEDS) {
            DEBUG_PRINT("Too many leds configured! Only adding the %d first\n", MAX_LEDS);
            return -1;
        }
        led_cfg->leds[led_cfg->leds_nr] = lc;
        led_cfg->leds_nr++;
        return 0;
    }
}


void open_ioctl() {

    fd = open("/dev/brcmboard", O_RDWR);
    if ( fd == -1 ) {
        DEBUG_PRINT("failed to open: /dev/brcmboard\n");
        return;
    }
    DEBUG_PRINT("ioctl fd %d allocated\n", fd);
    return;
}


static int get_state_by_name(char* state_name) {
    int i;

    for (i=0 ; i<LED_STATES_MAX ; i++) {
        if (!strcasecmp(state_name, led_states[i]))
            return i;
    }


    DEBUG_PRINT("state name %s not found!\n", state_name);
    return -1;
}


static void all_leds_off(struct leds_configuration* led_cfg) {
    int i;
    for (i=0 ; i<led_cfg->leds_nr ; i++) {
        
        led_set(led_cfg, i, OFF);
    }
}


static struct leds_configuration* get_led_config(void) {
    int i,j,k;

    const char *led_names;
    const char *led_config;
    char *p, *ptr, *rest;

    struct leds_configuration* led_cfg = malloc(sizeof(struct leds_configuration));
    memset(led_cfg,0,sizeof(struct leds_configuration));
    led_cfg->leds_nr = 0;
    led_cfg->leds = malloc(MAX_LEDS * sizeof(struct led_config*));
    memset(led_cfg->leds, 0, MAX_LEDS * sizeof(struct led_config*));

    /* Initialize */
	uci_ctx = ucix_init_path("/lib/db/config/", "hw");
    if(!uci_ctx) {
        DEBUG_PRINT("Failed to load config file \"hw\"\n");
        return NULL;
    }
    
    led_names = ucix_get_option(uci_ctx, "hw", "board", "lednames");
//    printf("Led names: %s\n", led_names);

    /* Populate led configuration structure */
    ptr = (char *)led_names;
    p = strtok_r(ptr, " ", &rest);
    while(p != NULL) {
        char led_name_color[256] = {0};

        DEBUG_PRINT("%s\n", p);

        snprintf(led_name_color, 256, "%s_green", p);
        led_config = ucix_get_option(uci_ctx, "hw", "leds", led_name_color);
        add_led(led_cfg, led_name_color, led_config, GREEN);
        //printf("%s_green = %s\n", p, led_config);

        snprintf(led_name_color,   256, "%s_red", p); 
        led_config = ucix_get_option(uci_ctx, "hw", "leds", led_name_color);
        add_led(led_cfg, led_name_color, led_config, RED);
        //printf("%s_red = %s\n", p, led_config);

        snprintf(led_name_color,  256, "%s_blue", p);
        led_config = ucix_get_option(uci_ctx, "hw", "leds", led_name_color);
        add_led(led_cfg, led_name_color, led_config, BLUE);
        //printf("%s_blue = %s\n", p, led_config);

        snprintf(led_name_color,  256, "%s_yellow", p);
        led_config = ucix_get_option(uci_ctx, "hw", "leds", led_name_color);
        add_led(led_cfg, led_name_color, led_config, YELLOW);

        /* Get next */
        ptr = rest;
        p = strtok_r(NULL, " ", &rest);
    }
//    printf("%d leds added to config\n", led_cfg->leds_nr);

    //open_ioctl();

    //reset shift register states
    for (i=0 ; i<SR_MAX ; i++) led_cfg->shift_register_state[i] = 0;

    //populate led mappings
    for (i=0 ; i<LED_FUNCTIONS ; i++) {
        const char *led_fn_actions;
        char fn_name_action[256];
        char l1[256],s1[256];
        for (j=0 ; j<LED_ACTION_MAX ; j++) {
            snprintf(fn_name_action, 256, "%s_%s", led_functions[i], fn_actions[j]);
            led_fn_actions = ucix_get_option(uci_ctx, "hw", "led_map", fn_name_action);
            DEBUG_PRINT("fn name action |%s| = %s\n", fn_name_action, led_fn_actions);


            // reset led actions
            for(k=0 ; k<LED_ACTION_MAX ; k++) {
                led_cfg->led_map_config[i][j].led_actions[k].led_index = -1;
                led_cfg->led_map_config[i][j].led_actions[k].led_state = -1;
            }

            if (led_fn_actions) {
                int l=0, m;
                ptr = (char *)led_fn_actions;
                p = strtok_r(ptr, " ", &rest);
                while(p != NULL) {
                    m = sscanf(ptr, "%[^=]=%s", l1, s1);
                    DEBUG_PRINT("m=%d ptr=%s l1=%s s1=%s\n", m,ptr,l1,s1);

                    led_cfg->led_map_config[i][j].led_actions[l].led_index = get_led_index_by_name(led_cfg, l1);
                    led_cfg->led_map_config[i][j].led_actions[l].led_state = get_state_by_name(s1);
                    led_cfg->led_map_config[i][j].led_actions_nr++;
                    DEBUG_PRINT("[%d] -> %d, %d\n", led_cfg->led_map_config[i][j].led_actions_nr, led_cfg->led_map_config[i][j].led_actions[l].led_index, led_cfg->led_map_config[i][j].led_actions[l].led_state);
                    /* Get next */
                    ptr = rest;
                    p = strtok_r(NULL, " ", &rest);
                    l++;
                }

            }
        }
    }

    led_cfg->leds_state = LEDS_NORMAL;
    led_cfg->test_state = 0;

    /* Turn off all leds */
    DEBUG_PRINT("Turn off all leds\n");
    all_leds_off(led_cfg);

    /* Set all function states to off */
    DEBUG_PRINT("Set all function states to off\n");
    for (i=0 ; i<LED_FUNCTIONS ; i++) {
        led_cfg->led_fn_action[i] = LED_OFF;
    }

    return led_cfg;
}


void print_config(struct leds_configuration* led_cfg) {
    int i;
    DEBUG_PRINT("\n\n\n Leds: %d\n", led_cfg->leds_nr);

    for (i=0 ; i<led_cfg->leds_nr ; i++) {
        struct led_config* lc = led_cfg->leds[i];
        DEBUG_PRINT("%s: type: %d, adr:%d, color:%d, act:%d\n", lc->name, lc->type, lc->address, lc->color, lc->active);
    }
}


static int get_led_index_by_name(struct leds_configuration* led_cfg, char* led_name) {
    int i;
    for (i=0 ; i<led_cfg->leds_nr ; i++) {
        struct led_config* lc = led_cfg->leds[i];
        if (!strcmp(led_name, lc->name))
            return i;
    }
    DEBUG_PRINT("Led name %s not found!\n", led_name);
    return -1;
}

static int get_led_index_by_function_color(struct leds_configuration* led_cfg, char* function, int color) {
    int i;
    for (i=0 ; i<led_cfg->leds_nr ; i++) {
        struct led_config* lc = led_cfg->leds[i];
        if (!strcmp(function, lc->function) && (lc->color == color))
            return i;
    }
    return -1;
}

static int board_ioctl(int fd, int ioctl_id, int action, int hex, char* string_buf, int string_buf_len, int offset) {
    BOARD_IOCTL_PARMS IoctlParms = {0};
    IoctlParms.string = string_buf;
    IoctlParms.strLen = string_buf_len;
    IoctlParms.offset = offset;
    IoctlParms.action = action;
    IoctlParms.buf    = "";
    if ( ioctl(fd, ioctl_id, &IoctlParms) < 0 ) {
        syslog(LOG_INFO, "ioctl: %d failed\n", ioctl_id);
        exit(1);
    }
    return IoctlParms.result;
}

static void shift_register3_set(struct leds_configuration* led_cfg, int address, int state, int active) {
    int i;

    if (address>=SR_MAX-1) {
        DEBUG_PRINT("address index %d too large\n", address);
        return;
    }
    // Update internal register copy
    led_cfg->shift_register_state[address] = state^active;

    // pull down shift register load (load gpio 23)
    board_ioctl(fd, BOARD_IOCTL_SET_GPIO, 0, 0, NULL, 23, 0);

    //for (i=0 ; i<SR_MAX ; i++) DEBUG_PRINT("%d ", led_cfg->shift_register_state[SR_MAX-1-i]);
    //DEBUG_PRINT("\n");

    // clock in bits
    for (i=0 ; i<SR_MAX ; i++) {

        //set clock low
        board_ioctl(fd, BOARD_IOCTL_SET_GPIO, 0, 0, NULL, 0, 0);
        //place bit on data line
        board_ioctl(fd, BOARD_IOCTL_SET_GPIO, 0, 0, NULL, 1, led_cfg->shift_register_state[SR_MAX-1-i]);
        //set clock high
        board_ioctl(fd, BOARD_IOCTL_SET_GPIO, 0, 0, NULL, 0, 1);
    }

    // issue shift register load
    board_ioctl(fd, BOARD_IOCTL_SET_GPIO, 0, 0, NULL, 23, 1);
}

static int led_set(struct leds_configuration* led_cfg, int led_idx, int state) {
    struct led_config* lc;

    if ((led_idx == -1) || (led_idx > led_cfg->leds_nr-1)) {
        DEBUG_PRINT("Led index: %d out of bounds, nr_leds = %d\n", led_idx, led_cfg->leds_nr);
        return 0;
    }

    lc = led_cfg->leds[led_idx];

    //printf("Led index: %d\n", led_idx);
    // DEBUG_PRINT("Led index: %d\n", led_idx);

    if (lc->type == GPIO) {
        board_ioctl(fd, BOARD_IOCTL_SET_GPIO, 0, 0, NULL, lc->address, state^lc->active);
    } else if (lc->type == SHIFTREG3) {
        shift_register3_set(led_cfg, lc->address, state, lc->active);
    } else if (lc->type == SHIFTREG2) {
        board_ioctl(fd, BOARD_IOCTL_LED_CTRL, 0, 0, NULL, lc->address, state^lc->active);
    }
    lc->blink_state = state;

    return 0;
}

static void led_set_state(struct leds_configuration* led_cfg, int led_idx, int state) {
    struct led_config* lc;

    if ((led_idx == -1) || (led_idx > led_cfg->leds_nr-1)) {
        DEBUG_PRINT("led_set_state: Led index: %d out of bounds, nr_leds = %d\n", led_idx, led_cfg->leds_nr);
        return 0;
    }

    lc = led_cfg->leds[led_idx];
    lc->state = state;
}

static void all_leds_on(struct leds_configuration* led_cfg) {
    int i;
    for (i=0 ; i<led_cfg->leds_nr ; i++) {
        
        led_set(led_cfg, i, ON);
    }
}

static void all_leds_test(struct leds_configuration* led_cfg) {
    int i;
    //all_leds_off(led_cfg);
    for (i=0 ; i<led_cfg->leds_nr ; i++) {
        led_set(led_cfg, i, ON);
        sleep(1);        
        led_set(led_cfg, i, OFF);
    }
    all_leds_off(led_cfg);
    sleep(1);
    all_leds_on(led_cfg);
    sleep(1);
    all_leds_off(led_cfg);
    sleep(1);
    all_leds_on(led_cfg);
    sleep(1);
    all_leds_off(led_cfg);
}


void blink_led(struct leds_configuration* led_cfg, int state) {
    int i;
    for (i=0 ; i<led_cfg->leds_nr ; i++) {
        struct led_config* lc = led_cfg->leds[i];
        if (lc->state == state) {
            //printf("Blinking %s\n", lc->name);
            led_set(led_cfg, i, lc->blink_state?0:1);
        }
    }
}

static void leds_test(struct leds_configuration* led_cfg) {
    if (led_cfg->test_state == 0)
        all_leds_on(led_cfg);
    if (led_cfg->test_state == 1)
        all_leds_off(led_cfg);
    if (led_cfg->test_state == 2)
        all_leds_on(led_cfg);
    if (led_cfg->test_state == 3)
        all_leds_off(led_cfg);

    if ((led_cfg->test_state > 4) && (led_cfg->test_state < led_cfg->leds_nr+4)) {
        led_set(led_cfg, led_cfg->test_state-5, OFF);
    }

    if ((led_cfg->test_state > 3) && (led_cfg->test_state < led_cfg->leds_nr+4)) {
        led_set(led_cfg, led_cfg->test_state-4, ON);
    }

    if (led_cfg->test_state == (led_cfg->leds_nr+5))
        all_leds_on(led_cfg);
    if (led_cfg->test_state == (led_cfg->leds_nr+6))
        all_leds_off(led_cfg);
    if (led_cfg->test_state == (led_cfg->leds_nr+7))
        all_leds_on(led_cfg);
    if (led_cfg->test_state == (led_cfg->leds_nr+8))
        all_leds_off(led_cfg);

    //printf("T state = %d\n", led_cfg->test_state);

    led_cfg->test_state++;
    if (led_cfg->test_state >  led_cfg->leds_nr+8)
        led_cfg->test_state = 0;
}

static void blink_handler(struct uloop_timeout *timeout);
static struct uloop_timeout blink_inform_timer = { .cb = blink_handler };
static unsigned int cnt = 0;

static void check_buttons(int initialize) {
    int button, i;
    struct button_config* bc;

    for (i=0 ; i<butt_cfg->button_nr ; i++) {
        bc = butt_cfg->buttons[i];
        button = board_ioctl(fd, BOARD_IOCTL_GET_GPIO, 0, 0, NULL, bc->address, 0);
        if (!initialize) {
            if (button^bc->active) {
                DEBUG_PRINT("Button %s pressed\n",bc->name);
                //syslog(LOG_INFO, "Button %s pressed\n",bc->name);
                bc->pressed_state = 1;
            }
            if ((!(button^bc->active)) && (bc->pressed_state)) {
                char str[512] = {0};
                DEBUG_PRINT("Button %s released, executing hotplug button command: %s\n",bc->name, bc->command);
                snprintf(str, 512, "ACTION=register INTERFACE=%s /sbin/hotplug-call button &",bc->command);
                system(str);
                syslog(LOG_INFO, "ACTION=register INTERFACE=%s /sbin/hotplug-call button", bc->command);
                bc->pressed_state = 0;
            }
        }
    }

}

static void blink_handler(struct uloop_timeout *timeout)
{
    cnt++;

    if (led_cfg->leds_state == LEDS_TEST) {
        if (!(cnt%3))
            leds_test(led_cfg);
    } else if (led_cfg->leds_state == LEDS_NORMAL){
        if (!(cnt%4))
            blink_led(led_cfg, BLINK_FAST);

        if (!(cnt%8))
            blink_led(led_cfg, BLINK_SLOW);
    }

    if (!(cnt%4))
        check_buttons(0);

	uloop_timeout_set(&blink_inform_timer, 100);
    
    //printf("Timer\n");
}

static int index_from_action(const char* action) {
    int i;
    for (i=0 ; i<LED_ACTION_MAX ; i++) {
        if (!strcasecmp(action, fn_actions[i]))
            return i;
    }

    DEBUG_PRINT("action %s not found!\n", action);
    return -1;
};


static void set_function_led(struct leds_configuration* led_cfg, char* fn_name, const char* action) {
    int i;
    char* led_name = NULL;
    int led_fn_idx = -1;
    int action_idx;
    struct led_map *map;

    DEBUG_PRINT("set_function_led\n");
    for (i=0 ; i<LED_FUNCTIONS ; i++) {
        if (!strcmp(fn_name, led_functions[i])) {
            led_name = led_functions[i];
            led_fn_idx = i;
        }
    }
    DEBUG_PRINT("set_function_led_mid\n");
    if (!(led_name)) return;

// printf("Action\n");
    DEBUG_PRINT("set_function_led_mid2\n");


//    snprintf(led_name_color, 256, "%s_%s", led_name, color);  
    action_idx = index_from_action(action);
    if (action_idx == -1) return;

    led_cfg->led_fn_action[led_fn_idx] = action_idx;

    map = &led_cfg->led_map_config[led_fn_idx][action_idx];
    for (i=0 ; i<map->led_actions_nr ; i++) {
            DEBUG_PRINT("[%d] %d %d\n", map->led_actions_nr,  map->led_actions[i].led_index, map->led_actions[i].led_state);
        if (led_cfg->leds_state != LEDS_INFO) {
            led_set(led_cfg, map->led_actions[i].led_index, map->led_actions[i].led_state);
        }
            led_set_state(led_cfg, map->led_actions[i].led_index, map->led_actions[i].led_state);
    }
    DEBUG_PRINT("set_function_led_end\n");

}


enum {
	LED_STATE,
	__LED_MAX
};

static const struct blobmsg_policy led_policy[] = {
	[LED_STATE] = { .name = "state", .type = BLOBMSG_TYPE_STRING },
};

struct hello_request {
	struct ubus_request_data req;
	struct uloop_timeout timeout;
	char data[];
};



static int led_set_method(struct ubus_context *ubus_ctx, struct ubus_object *obj,
		      struct ubus_request_data *req, const char *method,
		      struct blob_attr *msg)
{
	struct blob_attr *tb[__LED_MAX];
    char* state;
	DEBUG_PRINT("led_set_method\n");

	blobmsg_parse(led_policy, ARRAY_SIZE(led_policy), tb, blob_data(msg), blob_len(msg));

	if (tb[LED_STATE]) {
        char *fn_name = strchr(obj->name, '.') + 1;
		state = blobmsg_data(tb[LED_STATE]);
//    	fprintf(stderr, "Led %s method: %s state %s\n", fn_name, method, state);
        syslog(LOG_INFO, "Led %s method: %s state %s", fn_name, method, state);
        
        set_function_led(led_cfg, fn_name, state);
    }

	return 0;
}


static void led_status_reply(struct uloop_timeout *t)
{
	struct hello_request *req = container_of(t, struct hello_request, timeout);
	DEBUG_PRINT("led_status_reply\n");

	blob_buf_init(&b, 0);
	blobmsg_add_string(&b, "state", req->data);
	ubus_send_reply(ubus_ctx, &req->req, b.head);
	ubus_complete_deferred_request(ubus_ctx, &req->req, 0);
	free(req);
}

static int led_status_method(struct ubus_context *ubus_ctx, struct ubus_object *obj,
		      struct ubus_request_data *req, const char *method,
		      struct blob_attr *msg)
{
	struct hello_request *hreq;
    int action, i, led_fn_idx=0;
    char *fn_name = strchr(obj->name, '.') + 1;

    for (i=0 ; i<LED_FUNCTIONS ; i++) {
        if (!strcmp(fn_name, led_functions[i])) {
            led_fn_idx = i;
        }
    }

    action = led_cfg->led_fn_action[led_fn_idx];

    DEBUG_PRINT( "Led %s method: %s action %d\n", fn_name, method, action);

	hreq = calloc(1, sizeof(*hreq) +  100);
	sprintf(hreq->data, "%s", fn_actions[action]);
	ubus_defer_request(ubus_ctx, req, &hreq->req);
	hreq->timeout.cb = led_status_reply;
	uloop_timeout_set(&hreq->timeout, 1000);

	return 0;
}


static int leds_set_method(struct ubus_context *ubus_ctx, struct ubus_object *obj,
		      struct ubus_request_data *req, const char *method,
		      struct blob_attr *msg)
{
	struct blob_attr *tb[__LED_MAX];
    char* state;
    int i,j;
	DEBUG_PRINT("leds_set_method\n");

	blobmsg_parse(led_policy, ARRAY_SIZE(led_policy), tb, blob_data(msg), blob_len(msg));

	if (tb[LED_STATE]) {
		state = blobmsg_data(tb[LED_STATE]);

        for (i=0 ; i<LEDS_MAX ; i++) {
            if (!strcasecmp(state, leds_states[i]))
                break;
        }

        if (i == LEDS_INFO) {
            all_leds_off(led_cfg);
            set_function_led(led_cfg, "eco", "ok");
        }


        led_cfg->leds_state = i;


        if (i == LEDS_TEST) {
            all_leds_off(led_cfg);
        }


        if (i == LEDS_NORMAL) {
            all_leds_off(led_cfg);
            set_function_led(led_cfg, "eco", "off");
            for (j=0 ; j<LED_FUNCTIONS ; j++) {
                set_function_led(led_cfg, led_functions[j], fn_actions[led_cfg->led_fn_action[j]]);
            }
        }

    }

	return 0;
}


static int leds_status_method(struct ubus_context *ubus_ctx, struct ubus_object *obj,
		      struct ubus_request_data *req, const char *method,
		      struct blob_attr *msg)
{
	struct hello_request *hreq;
	DEBUG_PRINT("leds_status_method\n");
	hreq = calloc(1, sizeof(*hreq) +  100);
	sprintf(hreq->data, "%s", leds_states[led_cfg->leds_state]);
	ubus_defer_request(ubus_ctx, req, &hreq->req);
	hreq->timeout.cb = led_status_reply;
	uloop_timeout_set(&hreq->timeout, 1000);

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
};

static struct ubus_object_type leds_object_type =
	UBUS_OBJECT_TYPE("leds", leds_methods);


#define LED_OBJECTS 15

static struct ubus_object led_objects[LED_OBJECTS] = {
    { .name = "leds",	    .type = &leds_object_type, .methods = leds_methods, .n_methods = ARRAY_SIZE(leds_methods), },    
    { .name = "led.dsl",	.type = &led_object_type, .methods = led_methods, .n_methods = ARRAY_SIZE(led_methods), },
    { .name = "led.wifi",	.type = &led_object_type, .methods = led_methods, .n_methods = ARRAY_SIZE(led_methods), },
    { .name = "led.wps",	.type = &led_object_type, .methods = led_methods, .n_methods = ARRAY_SIZE(led_methods), },
    { .name = "led.lan",	.type = &led_object_type, .methods = led_methods, .n_methods = ARRAY_SIZE(led_methods), },
    { .name = "led.status",	.type = &led_object_type, .methods = led_methods, .n_methods = ARRAY_SIZE(led_methods), },
    { .name = "led.dect",	.type = &led_object_type, .methods = led_methods, .n_methods = ARRAY_SIZE(led_methods), },
    { .name = "led.tv",	.type = &led_object_type, .methods = led_methods, .n_methods = ARRAY_SIZE(led_methods), },
    { .name = "led.usb",	.type = &led_object_type, .methods = led_methods, .n_methods = ARRAY_SIZE(led_methods), },
    { .name = "led.wan",	.type = &led_object_type, .methods = led_methods, .n_methods = ARRAY_SIZE(led_methods), },
    { .name = "led.internet",	.type = &led_object_type, .methods = led_methods, .n_methods = ARRAY_SIZE(led_methods), },
    { .name = "led.voice1",	.type = &led_object_type, .methods = led_methods, .n_methods = ARRAY_SIZE(led_methods), },
    { .name = "led.voice2",	.type = &led_object_type, .methods = led_methods, .n_methods = ARRAY_SIZE(led_methods), },
    { .name = "led.eco",	.type = &led_object_type, .methods = led_methods, .n_methods = ARRAY_SIZE(led_methods), },
    { .name = "led.gbe",	.type = &led_object_type, .methods = led_methods, .n_methods = ARRAY_SIZE(led_methods), },
};



static void server_main(struct leds_configuration* led_cfg)
{
	int ret, i;

    for (i=0 ; i<LED_OBJECTS ; i++) {
	    ret = ubus_add_object(ubus_ctx, &led_objects[i]);
	    if (ret)
		    DEBUG_PRINT("Failed to add object: %s\n", ubus_strerror(ret));
    }


    uloop_timeout_set(&blink_inform_timer, 100);

	uloop_run();
}


static struct button_configuration* get_button_config(void) {
    int i;
    const char *butt_names;
    const char *butt_config;
    char *p, *ptr, *rest;

    butt_cfg = malloc(sizeof(struct button_configuration));
    butt_cfg->button_nr = 0;
    butt_cfg->buttons = malloc(MAX_BUTTON * sizeof(struct button_config*));
    /* Initialize */

    if(!uci_ctx) {
        DEBUG_PRINT("Failed to load uci config file \"hw\"\n");
        return NULL;
    }

    butt_names = ucix_get_option(uci_ctx, "hw", "board", "buttonnames");
    if (!butt_names) {
        DEBUG_PRINT("No hw.board.buttonnames entry found\n");
        return NULL;
    }

    /* Populate button configuration structure */
    DEBUG_PRINT("Populate button configuration structure\n");
    ptr = (char *)butt_names;
    p = strtok_r(ptr, " ", &rest);
    while(p != NULL) {
        struct button_config* bc;
        char active[256];
        char command[256];
        int  address;

        butt_config = ucix_get_option(uci_ctx, "hw", "buttons", p);

        bc = malloc(sizeof(struct button_config));
        bc->name = strdup(p);
        sscanf(butt_config, "%d %s %s", &address, active, command);
        DEBUG_PRINT("butt_config %d %s %s\n",address, active, command);

        if (!strcmp(active, "al"))   bc->active = ACTIVE_LOW;
        if (!strcmp(active, "ah"))   bc->active = ACTIVE_HIGH;

        bc->command = strdup(command);
        bc->address = address;
        bc->pressed_state = 0;

        /* Get next */
        ptr = rest;
        p = strtok_r(NULL, " ", &rest);

        if (butt_cfg->button_nr >= MAX_BUTTON) {
            DEBUG_PRINT("Too many buttons configured! Only adding the %d first\n", MAX_BUTTON);
            return NULL;
        }
        butt_cfg->buttons[butt_cfg->button_nr] = bc;
        butt_cfg->button_nr++;
    }

    for (i=0 ; i<butt_cfg->button_nr ; i++) {
        DEBUG_PRINT("%s button adr: %d active:%d command: %s\n",butt_cfg->buttons[i]->name, butt_cfg->buttons[i]->address, butt_cfg->buttons[i]->active, butt_cfg->buttons[i]->command);
    }

    /* Initialize the buttons, sometimes the button gpios are left in a pressed state, reading them 10 times should fix that */
    for (i=0 ; i<10 ; i++)
        check_buttons(1);
    DEBUG_PRINT("Buttons initialized\n");

    return butt_cfg;
}

int ledmngr(void) {
	const char *ubus_socket = NULL;

    open_ioctl();

    DEBUG_PRINT("get_led_config\n");
    led_cfg  = get_led_config();
    DEBUG_PRINT("get_button_config\n");
    butt_cfg = get_button_config();


    /* initialize ubus */
	DEBUG_PRINT("initialize ubus\n");

	uloop_init();

	ubus_ctx = ubus_connect(ubus_socket);
	if (!ubus_ctx) {
		DEBUG_PRINT("Failed to connect to ubus\n");
		return -1;
	}

	ubus_add_uloop(ubus_ctx);

	server_main(led_cfg);


    //all_leds_test(led_cfg);

	ubus_free(ubus_ctx);
	uloop_done();

	return 0;
}

