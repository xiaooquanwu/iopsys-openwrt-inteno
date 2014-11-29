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
#include <time.h>
#include <math.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <syslog.h>
#include <unistd.h>

#include <libubox/blobmsg_json.h>
#include "libubus.h"

#include <board.h>
#include "ucix.h"

#include "log.h"
#include "catv.h"
#include "sfp.h"


#include "button.h"
#include "led.h"
#include "spi.h"
#include "touch_sx9512.h"

static struct ubus_context *ubus_ctx = NULL;
static struct blob_buf bblob;

static struct leds_configuration* led_cfg;
static struct button_configuration* butt_cfg;
static struct uci_context *uci_ctx = NULL;

struct catv_handler *catv_h;
struct sfp_handler *sfp_h;
struct i2c_touch *i2c_touch;

int brcmboard = -1;

struct led_config  led_config;

/* Names for led_action_t */
static const char * const fn_actions[LED_ACTION_MAX] =
{ "off", "ok", "notice", "alert", "error",};

static const char* const led_functions[LED_FUNCTIONS] =
{ "dsl", "wifi", "wps", "lan", "status", "dect", "tv", "usb",
  "wan", "internet", "voice1", "voice2", "eco", "gbe"};

/* Names for led_state_t */
static const char* const led_states[LED_STATES_MAX] =
{ "off", "on", "blink_slow", "blink_fast" };

/* Names for leds_state_t */
static const char* const leds_states[LEDS_MAX] =
{ "normal", "proximity", "silent", "info", "test", "production", "reset", "allon" , "alloff"};


int get_led_index_by_name(struct leds_configuration* led_cfg, char* led_name);
int led_set(struct leds_configuration* led_cfg, int led_idx, int state);
int board_ioctl(int ioctl_id, int action, int hex, char* string_buf, int string_buf_len, int offset);
static void proximity_light(struct leds_configuration* led_cfg, int all);
void proximity_dim(struct leds_configuration* led_cfg, int all);

void sx9512_reset_handler(struct uloop_timeout *timeout);

struct uloop_timeout i2c_touch_reset_timer = { .cb = sx9512_reset_handler };

/* sx9512 needs a reset every 30 minutes. to recalibrate the touch detection */
void sx9512_reset_handler(struct uloop_timeout *timeout)
{
    int i;

    sx9512_reset(i2c_touch);

    for (i=0 ; i<led_cfg->leds_nr ; i++)
        if (led_cfg->leds[i]->type == I2C)
            led_set(led_cfg, i, -1);

    uloop_timeout_set(&i2c_touch_reset_timer, I2C_RESET_TIME);
}

void open_ioctl() {

    brcmboard = open("/dev/brcmboard", O_RDWR);
    if ( brcmboard == -1 ) {
        DEBUG_PRINT("failed to open: /dev/brcmboard\n");
        return;
    }
    DEBUG_PRINT("fd %d allocated\n", brcmboard);
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
    DEBUG_PRINT("leds_nr %d\n", led_cfg->leds_nr);
    for (i=0 ; i<led_cfg->leds_nr ; i++) {
        led_set(led_cfg, i, OFF);
    }
}


static struct leds_configuration* get_led_config(void) {
    int i,j,k;

    const char *led_names;
    const char *led_config;
    const char *p;
    char *ptr, *rest;

    struct leds_configuration* led_cfg = malloc(sizeof(struct leds_configuration));
    memset(led_cfg,0,sizeof(struct leds_configuration));

    led_cfg->leds = malloc(MAX_LEDS * sizeof(struct led_config*));
    memset(led_cfg->leds, 0, MAX_LEDS * sizeof(struct led_config*));

    led_names = ucix_get_option(uci_ctx, "hw", "board", "lednames");

    led_cfg->button_feedback_led = -1;

    /* Populate led configuration structure              */
    /* led_names is a space separated list of led names. */
    ptr = (char *)led_names;
    p = strtok_r(ptr, " ", &rest);

    while(p != NULL) {
        char led_name_color[256] = {0};

        DEBUG_PRINT("Add led colors for led name [%s]\n", p);

        snprintf(led_name_color, 256, "%s_green", p);
        led_config = ucix_get_option(uci_ctx, "hw", "leds", led_name_color);
        add_led(led_cfg, led_name_color, led_config, GREEN);

        snprintf(led_name_color,   256, "%s_red", p);
        led_config = ucix_get_option(uci_ctx, "hw", "leds", led_name_color);
        add_led(led_cfg, led_name_color, led_config, RED);

        snprintf(led_name_color,  256, "%s_blue", p);
        led_config = ucix_get_option(uci_ctx, "hw", "leds", led_name_color);
        add_led(led_cfg, led_name_color, led_config, BLUE);

        snprintf(led_name_color,  256, "%s_yellow", p);
        led_config = ucix_get_option(uci_ctx, "hw", "leds", led_name_color);
        add_led(led_cfg, led_name_color, led_config, YELLOW);

        snprintf(led_name_color, 256, "%s_white", p);
        led_config = ucix_get_option(uci_ctx, "hw", "leds", led_name_color);
        add_led(led_cfg, led_name_color, led_config, WHITE);

        /* Get next */
        p = strtok_r(NULL, " ", &rest);
    }

    //reset shift register states
    for (i=0 ; i<SR_MAX ; i++) {
        led_cfg->shift_register_state[i] = 0;
    }

    //populate led mappings
    for (i=0 ; i<LED_FUNCTIONS ; i++) {
        const char *led_fn_actions;
        char fn_name_action[256];
        char l1[256],s1[256];
        for (j=0 ; j<LED_ACTION_MAX ; j++) {

            snprintf(fn_name_action, 256, "%s_%s", led_functions[i], fn_actions[j]);
            led_fn_actions = ucix_get_option(uci_ctx, "hw", "led_map", fn_name_action);

            // reset led actions
            for(k=0 ; k<LED_ACTION_MAX ; k++) {
                led_cfg->led_map_config[i][j].led_actions[k].led_index = -1;
                led_cfg->led_map_config[i][j].led_actions[k].led_state = -1;
            }

            if (led_fn_actions) {
                int l=0;
                /* space separated list of actions */
                ptr = strtok_r((char *)led_fn_actions , " ", &rest);
                while(ptr != NULL) {
                    sscanf(ptr, "%[^=]=%s", l1, s1);
                    led_cfg->led_map_config[i][j].led_actions[l].led_index = get_led_index_by_name(led_cfg, l1);
                    led_cfg->led_map_config[i][j].led_actions[l].led_state = get_state_by_name(s1);
                    led_cfg->led_map_config[i][j].led_actions_nr++;

                    DEBUG_PRINT("%-15s -> nr=%d idx=%d,state=%d -> %-15s = %s\n",
                                fn_name_action,
                                led_cfg->led_map_config[i][j].led_actions_nr,
                                led_cfg->led_map_config[i][j].led_actions[l].led_index,
                                led_cfg->led_map_config[i][j].led_actions[l].led_state,
                                l1,
                                s1);

                    /* Get next */
                    ptr = strtok_r(NULL, " ", &rest);
                    l++;
                }

            }
        }
    }

    p = ucix_get_option(uci_ctx, "hw", "board", "hardware");
    if (p && !strcmp(p, "CG300"))
        led_cfg->leds_state = LEDS_PROXIMITY;
    else
        led_cfg->leds_state = LEDS_NORMAL;
    led_cfg->test_state = 0;
    led_cfg->proximity_timer = 0;
    led_cfg->proximity_all_timer = 0;

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

#if 0
static int led_need_type(const struct leds_configuration* led_cfg, led_type_t type)
{
    int i;
    for (i=0 ; i<led_cfg->leds_nr ; i++)
        if (led_cfg->leds[i]->type == type)
            return 1;
    return 0;
}
#endif

void print_config(struct leds_configuration* led_cfg) {
    int i;
    DEBUG_PRINT("\n\n\n Leds: %d\n", led_cfg->leds_nr);

    for (i=0 ; i<led_cfg->leds_nr ; i++) {
        struct led_config* lc = led_cfg->leds[i];
        DEBUG_PRINT("%s: type: %d, adr:%d, color:%d, act:%d\n", lc->name, lc->type, lc->address, lc->color, lc->active);
    }
}


int get_led_index_by_name(struct leds_configuration* led_cfg, char* led_name) {
    int i;
    for (i=0 ; i<led_cfg->leds_nr ; i++) {
        struct led_config* lc = led_cfg->leds[i];
        if (!strcmp(led_name, lc->name))
            return i;
    }
    DEBUG_PRINT("Led name %s not found!\n", led_name);
    return -1;
}

#if 0
static int get_led_index_by_function_color(struct leds_configuration* led_cfg, char* function, int color) {
    int i;
    for (i=0 ; i<led_cfg->leds_nr ; i++) {
        struct led_config* lc = led_cfg->leds[i];
        if (!strcmp(function, lc->function) && (lc->color == color))
            return i;
    }
    return -1;
}
#endif

int board_ioctl(int ioctl_id, int action, int hex, char* string_buf, int string_buf_len, int offset) {
    BOARD_IOCTL_PARMS IoctlParms = {0};
    IoctlParms.string = string_buf;
    IoctlParms.strLen = string_buf_len;
    IoctlParms.offset = offset;
    IoctlParms.action = action;
    IoctlParms.buf    = "";
    if ( ioctl(brcmboard, ioctl_id, &IoctlParms) < 0 ) {
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
    board_ioctl(BOARD_IOCTL_SET_GPIO, 0, 0, NULL, 23, 0);

    //for (i=0 ; i<SR_MAX ; i++) DEBUG_PRINT("%d ", led_cfg->shift_register_state[SR_MAX-1-i]);
    //DEBUG_PRINT("\n");

    // clock in bits
    for (i=0 ; i<SR_MAX ; i++) {


        //set clock low
        board_ioctl( BOARD_IOCTL_SET_GPIO, 0, 0, NULL, 0, 0);
        //place bit on data line
        board_ioctl( BOARD_IOCTL_SET_GPIO, 0, 0, NULL, 1, led_cfg->shift_register_state[SR_MAX-1-i]);
        //set clock high
        board_ioctl( BOARD_IOCTL_SET_GPIO, 0, 0, NULL, 0, 1);
    }

    // issue shift register load
    board_ioctl( BOARD_IOCTL_SET_GPIO, 0, 0, NULL, 23, 1);
}

/* Sets a led on or off (doesn't handle the blinking states). state ==
   -1 means update from the led's stored state. */
int led_set(struct leds_configuration* led_cfg, int led_idx, int state) {
    struct led_config* lc;

    if ((led_idx == -1) || (led_idx > led_cfg->leds_nr-1)) {
        DEBUG_PRINT("Led index: %d out of bounds, nr_leds = %d\n", led_idx, led_cfg->leds_nr);
        return 0;
    }

    lc = led_cfg->leds[led_idx];

    if (state < 0)
        state = (lc->state != OFF);

    //printf("Led index: %d\n", led_idx);
    // DEBUG_PRINT("Led index: %d\n", led_idx);

    if (lc->type == GPIO) {
        board_ioctl( BOARD_IOCTL_SET_GPIO, 0, 0, NULL, lc->address, state^lc->active);
    } else if (lc->type == SHIFTREG3) {
        shift_register3_set(led_cfg, lc->address, state, lc->active);
    } else if (lc->type == SHIFTREG2) {
        board_ioctl( BOARD_IOCTL_LED_CTRL, 0, 0, NULL, lc->address, state^lc->active);
    } else if (lc->type == I2C) {
        sx9512_led_set(lc, state);
	} else if (lc->type == SPI) {
		spi_led_set(lc, state);
    } else
        DEBUG_PRINT("Wrong type of bus (%d)\n",lc->type);

    lc->blink_state = state;

    return 0;
}

static void led_set_state(struct leds_configuration* led_cfg, int led_idx, led_state_t state) {
    struct led_config* lc;

    if ((led_idx == -1) || (led_idx > led_cfg->leds_nr-1)) {
        DEBUG_PRINT("Led index: %d out of bounds, nr_leds = %d\n", led_idx, led_cfg->leds_nr);
        return;
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

#if 0
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
#endif

void blink_led(struct leds_configuration* led_cfg, led_state_t state,
               int dimmed) {
    int i;
    for (i=0 ; i<led_cfg->leds_nr ; i++) {
        struct led_config* lc = led_cfg->leds[i];
        if (lc->state == state
            && (!lc->use_proximity || !dimmed)) {
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

static void leds_production(struct leds_configuration* led_cfg) {
    all_leds_off(led_cfg);
}

static void leds_reset(struct leds_configuration* led_cfg) {
    if (led_cfg->test_state == 0)
        all_leds_on(led_cfg);
    if (led_cfg->test_state == 1)
        all_leds_off(led_cfg);

    if (led_cfg->test_state == 2)
        all_leds_on(led_cfg);
    if (led_cfg->test_state == 3)
        all_leds_off(led_cfg);

    if (led_cfg->test_state == 4)
        all_leds_on(led_cfg);
    if (led_cfg->test_state == 5)
        all_leds_off(led_cfg);

    led_cfg->test_state++;
    if (led_cfg->test_state >  5) {
        led_cfg->test_state = 0;
//        led_cfg->leds_state = LEDS_NORMAL;
    }
}

static void blink_handler(struct uloop_timeout *timeout);
static struct uloop_timeout blink_inform_timer = { .cb = blink_handler };
static unsigned int cnt = 0;

static void blink_handler(struct uloop_timeout *timeout)
{
    cnt++;

    /* handle press indicator for touch */
    if(led_cfg->press_indicator) {
        static int times = 0;
        times++;
        if (times%2)
            proximity_light(led_cfg, 1);
        else
            proximity_dim(led_cfg, 1);
    }

    /* handle proximity timer */
    if (led_cfg->proximity_timer) {
        led_cfg->proximity_timer--;
        if (led_cfg->leds_state == LEDS_PROXIMITY
            && !led_cfg->proximity_timer)
            proximity_dim(led_cfg, 1);
    }

    /*handle proximity all timer */
    if (led_cfg->proximity_all_timer) {
        led_cfg->proximity_all_timer--;
        if (led_cfg->leds_state == LEDS_PROXIMITY
            && !led_cfg->proximity_all_timer)
            proximity_dim(led_cfg, !led_cfg->proximity_timer);
    }

    /* normal operation. */
    if (led_cfg->leds_state == LEDS_TEST) {
        if (!(cnt%3))
            leds_test(led_cfg);
    } else if (led_cfg->leds_state == LEDS_PROD) {
        if (!(cnt%16))
            leds_production(led_cfg);
    } else if (led_cfg->leds_state == LEDS_RESET) {
        leds_reset(led_cfg);
    } else if (led_cfg->leds_state != LEDS_INFO) {
        /* LEDS_NORMAL or LEDS_PROXIMITY */
        int dimmed = (led_cfg->leds_state == LEDS_PROXIMITY && !led_cfg->proximity_timer);

        if (!(cnt%4))
            blink_led(led_cfg, BLINK_FAST, dimmed);

        if (!(cnt%8))
            blink_led(led_cfg, BLINK_SLOW, dimmed);
    }

    /* check buttons every fourth run */
    if (!(cnt%4))
        check_buttons(led_cfg,butt_cfg, 0);

    uloop_timeout_set(&blink_inform_timer, 100);

    //printf("Timer\n");
}

static led_action_t index_from_action(const char* action) {
    int i;
    for (i=0 ; i<LED_ACTION_MAX ; i++) {
        if (!strcasecmp(action, fn_actions[i]))
            return i;
    }

    DEBUG_PRINT("action %s not found!\n", action);
    return -1;
};


static void set_function_led(struct leds_configuration* led_cfg, const char* fn_name, const char* action) {
    int i;
    const char* led_name = NULL;
    int led_fn_idx = -1;
    led_action_t action_idx;
    struct led_map *map;

    DEBUG_PRINT("(%s ->%s)\n",fn_name, action);
    for (i=0 ; i<LED_FUNCTIONS ; i++) {
        if (!strcmp(fn_name, led_functions[i])) {
            led_name = led_functions[i];
            led_fn_idx = i;
        }
    }
    DEBUG_PRINT("fun=%s idx=%d\n",led_name,led_fn_idx);
    if (!(led_name)) return;

// printf("Action\n");
    DEBUG_PRINT("set_function_led_mid2\n");


//    snprintf(led_name_color, 256, "%s_%s", led_name, color);
    action_idx = index_from_action(action);
    if (action_idx == -1) return;

    led_cfg->led_fn_action[led_fn_idx] = action_idx;

    map = &led_cfg->led_map_config[led_fn_idx][action_idx];
    for (i=0 ; i<map->led_actions_nr ; i++) {
        int led_idx = map->led_actions[i].led_index;
        DEBUG_PRINT("[%d] %d %d\n", map->led_actions_nr, led_idx, map->led_actions[i].led_state);

        /* In silent mode, we set lc->state to off. It might make more
           sense to maintain the desired state, and omit blinking it
           in the blink_handler, but then we would need some
           additional flag per led. In all cases,
           led_cfg->led_fn_action records the desired state, so it
           isn't lost when switching back to non-silent mode. */
        if (led_cfg->leds_state == LEDS_SILENT
            && led_cfg->leds[led_idx]->use_proximity
            && action_idx < LED_ALERT)
            led_set_state(led_cfg, led_idx, OFF);
        else
            led_set_state(led_cfg, led_idx,
                          map->led_actions[i].led_state);

        if (led_cfg->leds_state != LEDS_INFO) {
            led_set(led_cfg, map->led_actions[i].led_index, -1);

            if (led_cfg->leds_state == LEDS_PROXIMITY &&
                led_cfg->leds[led_idx]->use_proximity) {
                /* Changing led status of any dimmed led should also
                   light up the display. */
                if (!led_cfg->proximity_timer)
                    proximity_light(led_cfg, 0);
                if (led_cfg->proximity_timer < 5*10)
                    led_cfg->proximity_timer = 5*10;
            }
        }
    }
    DEBUG_PRINT("end\n");
}

/* Leds marked with use_proximity are lit up when led_cfg->state ==
   LEDS_NORMAL or led_cfg->proximity_timer > 0. */
static void proximity_light(struct leds_configuration* led_cfg, int all)
{
    int i;
    DEBUG_PRINT("\n");
    /* if led stored state is not OFF and the leds is used to indicate proximity turn on */
    for (i=0 ; i<led_cfg->leds_nr ; i++) {
        struct led_config* lc = led_cfg->leds[i];
        if (lc->use_proximity && (lc->state || all))
            led_set(led_cfg, i, 1);
    }

#if 0
    /* if we have a feedback led turn it on if state is PROXIMITY else set to internal state.*/
    if (led_cfg->button_feedback_led >= 0)
        led_set(led_cfg, led_cfg->button_feedback_led,
                led_cfg->leds_state == LEDS_PROXIMITY ? 1 : -1);
#endif
}

void proximity_dim(struct leds_configuration* led_cfg, int all)
{
    int i;
    DEBUG_PRINT("\n");
    for (i=0 ; i<led_cfg->leds_nr ; i++) {
        struct led_config* lc = led_cfg->leds[i];
        if (lc->use_proximity && (!lc->state || all)
            && lc->blink_state)
            led_set(led_cfg, i, 0);
    }

    if (all && led_cfg->leds_state == LEDS_PROXIMITY
        && led_cfg->button_feedback_led >= 0) {
        led_set(led_cfg, led_cfg->button_feedback_led, -1);
    }
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
    DEBUG_PRINT("led_set_method (%s)\n",method);

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

static int led_status_method(struct ubus_context *ubus_ctx, struct ubus_object *obj,
                             struct ubus_request_data *req, const char *method,
                             struct blob_attr *msg)
{
    int action, i, led_fn_idx=0;
    char *fn_name = strchr(obj->name, '.') + 1;

    for (i=0 ; i<LED_FUNCTIONS ; i++) {
        if (!strcmp(fn_name, led_functions[i])) {
            led_fn_idx = i;
        }
    }

    action = led_cfg->led_fn_action[led_fn_idx];

    DEBUG_PRINT( "Led %s method: %s action %d\n", fn_name, method, action);

    blob_buf_init (&bblob, 0);
    blobmsg_add_string(&bblob, "state", fn_actions[action]);
    ubus_send_reply(ubus_ctx, req, bblob.head);

    return 0;
}

static int leds_proximity_method(struct ubus_context *ubus_ctx, struct ubus_object *obj,
                                 struct ubus_request_data *req, const char *method,
                                 struct blob_attr *msg)
{
    const struct blobmsg_policy proximity_policy[] = {
        /* FIXME: Can we use an integer type here? */
        { .name = "timeout", .type = BLOBMSG_TYPE_STRING, },
        { .name = "light-all", .type = BLOBMSG_TYPE_STRING, },
    };
    unsigned long timeout = 0;
    unsigned long light_all = 0;
    struct blob_attr *tb[ARRAY_SIZE(proximity_policy)];
    blobmsg_parse(proximity_policy, ARRAY_SIZE(proximity_policy),
                  tb, blob_data(msg), blob_len(msg));
    if (tb[0]) {
        const char *digits = blobmsg_data(tb[0]);
        char *end;
        timeout = strtoul(digits, &end, 10);
        if (!*digits || *end) {
            syslog(LOG_INFO, "Leds proximity method: Invalid timeout %s\n", digits);
            return 1;
        }
    }
    if (tb[1]) {
        const char *digits = blobmsg_data(tb[1]);
        char *end;
        light_all = strtoul(digits, &end, 10);
    }
    DEBUG_PRINT ("proximity method: timeout %lu, light-all %lu\n", timeout, light_all);
    timeout *= 10;
    light_all *= 10;
    if (led_cfg->leds_state == LEDS_PROXIMITY) {
        if (light_all) {
            if (!led_cfg->proximity_all_timer && !led_cfg->proximity_timer) {
                proximity_light(led_cfg, 1);
                led_cfg->proximity_all_timer = light_all;
            }
        }
        else if (timeout && !led_cfg->proximity_timer)
            proximity_light(led_cfg, 0);
    }
    else
        /* Skip setup of this timer when not in proximity mode. */
        led_cfg->proximity_all_timer = 0;

    if (timeout > led_cfg->proximity_all_timer)
        led_cfg->proximity_timer = timeout;

    return 0;
}

static int leds_set_method(struct ubus_context *ubus_ctx, struct ubus_object *obj,
                           struct ubus_request_data *req, const char *method,
                           struct blob_attr *msg)
{
    struct blob_attr *tb[__LED_MAX];
    char* state;
    int i,j;
    DEBUG_PRINT("\n");

    blobmsg_parse(led_policy, ARRAY_SIZE(led_policy), tb, blob_data(msg), blob_len(msg));

    if (tb[LED_STATE]) {
        leds_state_t old;
        state = blobmsg_data(tb[LED_STATE]);

        for (i=0 ; i<LEDS_MAX ; i++) {
            if (!strcasecmp(state, leds_states[i]))
                break;
        }

        if (i == LEDS_MAX) {
            syslog(LOG_INFO, "leds_set_method: Unknown state %s.\n", state);
            return 0;
        }

        old = led_cfg->leds_state;
        led_cfg->leds_state = i;

        if (i == LEDS_INFO) {
            all_leds_off(led_cfg);
            set_function_led(led_cfg, "eco", "off");
            set_function_led(led_cfg, "eco", "ok");
        }

        if (i == LEDS_TEST) {
            all_leds_off(led_cfg);
        }
        if (i == LEDS_ALLON) {
            all_leds_on(led_cfg);
        }
        if (i == LEDS_ALLOFF) {
            all_leds_off(led_cfg);
        }

        if (i <= LEDS_SILENT) {
            if (i == LEDS_SILENT || old >= LEDS_SILENT) {
                all_leds_off(led_cfg);
                set_function_led(led_cfg, "eco", "off");
                for (j=0 ; j<LED_FUNCTIONS ; j++) {
                    set_function_led(led_cfg, led_functions[j], fn_actions[led_cfg->led_fn_action[j]]);
                }
            }
            if (i == LEDS_NORMAL)
                proximity_light(led_cfg, 0);
            else if (i == LEDS_PROXIMITY) {
                if (led_cfg->proximity_timer)
                    proximity_light(led_cfg, 0);
                else
                    proximity_dim(led_cfg, 1);
            }
        }
    }
    else
        syslog(LOG_INFO, "leds_set_method: Unknown attribute.\n");

    return 0;
}


static int leds_status_method(struct ubus_context *ubus_ctx, struct ubus_object *obj,
                              struct ubus_request_data *req, const char *method,
                              struct blob_attr *msg)
{
	DEBUG_PRINT("\n");

	blob_buf_init (&bblob, 0);
	blobmsg_add_string(&bblob, "state", leds_states[led_cfg->leds_state]);
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
    { .name = "proximity", .handler = leds_proximity_method },
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

    if (sfp_h)
        sfp_ubus_populate(sfp_h, ubus_ctx);

    if (catv_h){
        catv_ubus_populate(catv_h, ubus_ctx);
    }


    uloop_timeout_set(&blink_inform_timer, 100);

    if (i2c_touch)
        uloop_timeout_set(&i2c_touch_reset_timer, I2C_RESET_TIME);

    uloop_run();
}




static int load_cfg_file()
{
    /* Initialize */
    uci_ctx = ucix_init_path("/lib/db/config/", "hw");
    if(!uci_ctx) {
        return 0;
    }
    return 1;
}

int ledmngr(void) {
    const char *ubus_socket = NULL;
    int i;

    open_ioctl();

    if (! load_cfg_file() ){
        DEBUG_PRINT("Failed to load config file \"hw\"\n");
        exit(1);
    }
//    if (led_need_type (led_cfg, I2C) || button_need_type (butt_cfg, I2C))

	i2c_touch = sx9512_init(uci_ctx);
    spi_init(uci_ctx);

    led_cfg  = get_led_config();
    butt_cfg = get_button_config(uci_ctx, i2c_touch);
    /* Initialize the buttons, sometimes the button gpios are left in a pressed state, reading them 10 times should fix that */
    /* BUG: move this hack into button.c */
    for (i=0 ; i<10 ; i++)
        check_buttons(led_cfg,butt_cfg,1);

    sfp_h = sfp_init(uci_ctx);

    catv_h = catv_init(uci_ctx, "/dev/i2c-0", 0x50, 0x51);

    if(catv_h == 0)
        DEBUG_PRINT("no CATV device found \n");

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

