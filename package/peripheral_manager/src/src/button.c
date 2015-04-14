/*-*- Mode: C; c-basic-offset: 8; indent-tabs-mode: nil -*-*/
#include "config.h"
#include <syslog.h>
#include <time.h>
#include "log.h"
#include "button.h"
#include "led.h"
#include "touch_sx9512.h"
#include "prox_px3220.h"

/* used to map in the driver buttons to a function button */
struct button_drv_list {
        struct list_head list;
        struct timespec pressed_time;
        struct button_drv *drv;
};

/**/
struct function_button {
	struct list_head list;
        char *name;
        char *hotplug;
        int minpress;
        int longpress;                          /* negative value means valid if  mintime < time < abs(longpress ) */
                                                /* positive value means valid if time > longpreass */
                                                /* zero value means valid if time > mintime */
	struct list_head drv_list;              /* list of all driver button that is needed to activate this button function */
};

/* PUT every button from drivers into a list */
struct drv_button_list{
	struct list_head list;
	struct button_drv *drv;
};

/* list of all driver buttons added by drivers. */
static LIST_HEAD(drv_buttons_list);

/* list containing all function buttons read from config file */
static LIST_HEAD(buttons);

static struct button_drv *get_drv_button(char *name);
//static struct function_button *get_button(char *name);

void button_add( struct button_drv *drv)
{
	struct drv_button_list *drv_node = malloc(sizeof(struct drv_button_list));

	DBG(1,"called with led name [%s]", drv->name);
	drv_node->drv = drv;

	list_add(&drv_node->list, &drv_buttons_list);
}

static struct button_drv *get_drv_button(char *name)
{
	struct list_head *i;
	list_for_each(i, &drv_buttons_list) {
		struct drv_button_list *node = list_entry(i, struct drv_button_list, list);
                if (! strcmp(node->drv->name, name))
                        return node->drv;
	}
        return NULL;
}

#if 0
static struct function_button *get_button(char *name)
{
	struct list_head *i;
	list_for_each(i, &buttons) {
		struct function_button *node = list_entry(i, struct function_button, list);
                if (! strcmp(node->name, name))
                        return node;
	}
        return NULL;
}
#endif

static void dump_drv_list(void)
{
	struct list_head *i;
	list_for_each(i, &drv_buttons_list) {
		struct drv_button_list *node = list_entry(i, struct drv_button_list, list);
		DBG(1,"button name = [%s]",node->drv->name);
	}
}

static void dump_buttons_list(void)
{
	struct list_head *i;
	list_for_each(i, &buttons) {
		struct function_button *node = list_entry(i, struct function_button, list);
		DBG(1,"button name = [%s]",node->name);
                {
                        struct list_head *j;
                        list_for_each(j, &node->drv_list) {
                                struct button_drv_list *drv_node = list_entry(j, struct button_drv_list, list);
                                if(drv_node->drv != NULL)
                                        DBG(1,"%13s drv button name = [%s]","",drv_node->drv->name);
                        }
                        DBG(1,"%13s minpress = %d","",node->minpress);
                        DBG(1,"%13s longpress = %d","",node->longpress);
                }
	}
}

static int timer_started(struct button_drv_list *button_drv)
{
        if (button_drv->pressed_time.tv_sec == 0 )
                if (button_drv->pressed_time.tv_nsec == 0 )
                        return 0;
        return 1;
}

static void timer_start(struct button_drv_list *button_drv)
{
        clock_gettime(CLOCK_MONOTONIC, &button_drv->pressed_time);
}

static void timer_stop(struct button_drv_list *button_drv)
{
        button_drv->pressed_time.tv_sec = 0;
        button_drv->pressed_time.tv_nsec = 0;
}

static int timer_valid(struct button_drv_list *button_drv, int mtimeout, int longpress)
{
        struct timespec now;
        int sec;
        int nsec;

        if (timer_started(button_drv)) {
                clock_gettime(CLOCK_MONOTONIC, &now);
                sec =  now.tv_sec  - button_drv->pressed_time.tv_sec;
                nsec = now.tv_nsec - button_drv->pressed_time.tv_nsec;
                if ( mtimeout < (sec*1000 + nsec/1000000)) {
                        if (longpress == 0)
                                return 1;

                        if (longpress < 0) {
                                longpress = -1 * longpress;
                                if ( longpress > (sec*1000 + nsec/1000000)) {
                                        return 1;

                                } else {
                                        return 0;
                                }
                        }

                        if ( longpress < (sec*1000 + nsec/1000000)) {
                                return 1;
                        }
                }
        }
        return 0;
}

#define BUTTON_TIMEOUT 100
static void button_handler(struct uloop_timeout *timeout);
static struct uloop_timeout button_inform_timer = { .cb = button_handler };

static void button_handler(struct uloop_timeout *timeout)
{
 	struct list_head *i;
//        DBG(1, "");

#ifdef HAVE_BOARD_H

        /* sx9512 driver needs to read out all buttons at once */
        /* so call it once at beginning of scanning inputs  */
        sx9512_check();
        /* same for px3220 */
        px3220_check();
#endif

        /* clean out indicator status, set by any valid press again if we find it */
        led_pressindicator_clear();

	list_for_each(i, &buttons) {
                struct list_head *j;
		struct function_button *node = list_entry(i, struct function_button, list);

                list_for_each(j, &node->drv_list) {
                        struct button_drv_list *drv_node = list_entry(j, struct button_drv_list, list);
                        if (drv_node->drv) {
                                button_state_t st = drv_node->drv->func->get_state(drv_node->drv);

                                if (st == PRESSED ) {
                                        if (! timer_started(drv_node)) {
                                                timer_start(drv_node);
                                                DBG(1, " %s pressed", drv_node->drv->name);
                                        }

                                        if ( timer_valid(drv_node, node->minpress, 0)) {
                                                        led_pressindicator_set();
                                        }
                                }

                                if (st == RELEASED ) {
                                        if (timer_started(drv_node)) {
                                                DBG(1, " %s released", drv_node->drv->name);

                                                if ( timer_valid(drv_node, node->minpress, node->longpress) ) {
                                                        char str[512];
                                                        DBG(1, "send key %s [%s]to system", node->name, node->hotplug);
                                                        snprintf(str,
                                                                 512,
                                                                 "ACTION=register INTERFACE=%s /sbin/hotplug-call button &",
                                                                 node->hotplug);
                                                        system(str);
                                                        syslog(LOG_INFO, "%s",str);
                                                } else {
//                                                DBG(1, " %s not valid", drv_node->drv->name);
                                                }
                                        }
                                        timer_stop(drv_node);
                                }
//                                DBG(1, " %s state = %d", drv_node->drv->name,st);
                        }
                }
        }
	uloop_timeout_set(&button_inform_timer, BUTTON_TIMEOUT);
}

/* in order to support long press there is a need to go over every function button
   and find any driver button that is part of a longpress function.
   if found then the longpress time for that button needs to be filled in (but negative)
   on any other function button that has the same driver button. This to prevent two
   function buttons to trigger on one driver button release.
*/

/* Find functions that use driver (drv) that has a zero longpress time and set it to time */
static void longpress_set(int max_time,struct button_drv *drv) {
	struct list_head *i;
	list_for_each(i, &buttons) {
		struct function_button *node = list_entry(i, struct function_button, list);
                struct list_head *j;
                list_for_each(j, &node->drv_list) {
                        struct button_drv_list *drv_node = list_entry(j, struct button_drv_list, list);
                        if(drv_node->drv == drv){
                                if (node->longpress == 0) {
                                        node->longpress = max_time;
                                }
                        }
                }
        }
}


/* find any use of longpress and set all other to negative longpress time to indicate min max time
   for a valid press
*/
static void longpress_find(void) {
	struct list_head *i;
	list_for_each(i, &buttons) {
		struct function_button *node = list_entry(i, struct function_button, list);
                struct list_head *j;
                list_for_each(j, &node->drv_list) {
                        struct button_drv_list *drv_node = list_entry(j, struct button_drv_list, list);
                        if(drv_node->drv != NULL){
                                if (node->longpress > 0) {
                                        DBG(1,"%13s drv button name = [%s]","",drv_node->drv->name);
                                        DBG(1,"%13s longpress = %d","",node->longpress);
                                        longpress_set(node->longpress * -1, drv_node->drv);
                                }
                        }
                }
	}
}

void button_init( struct server_ctx *s_ctx)
{
	struct ucilist *node;
	LIST_HEAD(buttonnames);
        int default_minpress = 0;
        char *s;

        /* read out default global options */
        s = ucix_get_option(s_ctx->uci_ctx, "hw" , "button_map", "minpress");
        DBG(1, "default minpress = [%s]", s);
        if (s){
                default_minpress =  strtol(s,0,0);
        }

        /* read function buttons from section button_map */
	ucix_get_option_list(s_ctx->uci_ctx, "hw" ,"button_map", "buttonnames", &buttonnames);
	list_for_each_entry(node, &buttonnames, list) {
                struct function_button *function;

//                DBG(1, "value = [%s]",node->val);

		function = malloc(sizeof(struct function_button));
		memset(function,0,sizeof(struct function_button));
                function->name = node->val;

                /* read out minpress */
                s = ucix_get_option(s_ctx->uci_ctx, "hw" , function->name, "minpress");
                DBG(1, "minpress = [%s]", s);
                if (s){
                        function->minpress =  strtol(s,0,0);
                }else
                        function->minpress =  default_minpress;

                /* read out long_press */
                s = ucix_get_option(s_ctx->uci_ctx, "hw" , function->name, "longpress");
                DBG(1, "longpress = [%s]", s);
                if (s){
                        function->longpress =  strtol(s,0,0);
                }

                /* read out hotplug option */
                s = ucix_get_option(s_ctx->uci_ctx, "hw" , function->name, "hotplug");
                DBG(1, "hotplug = [%s]", s);
                if (s){
                        function->hotplug = s;
                }

                INIT_LIST_HEAD(&function->drv_list);

                {
                        struct ucilist *drv_node;
                        LIST_HEAD(head);
                        int num = 0;

                        /* read out all driver buttons that needs to be pressed for this button function. */
                        ucix_get_option_list(s_ctx->uci_ctx, "hw" ,function->name, "button", &head);

                        list_for_each_entry(drv_node, &head, list) {
                                struct button_drv_list *new_button;

                                num++;
                                DBG(1,"function %s -> drv button %s", function->name, drv_node->val);

                                new_button = malloc(sizeof(struct button_drv_list));
                                memset(new_button,0,sizeof(struct button_drv_list));

                                new_button->drv = get_drv_button(drv_node->val);

                                if(new_button->drv == NULL){
                                        syslog(LOG_WARNING, "%s wanted drv button [%s] but it can't be found. check spelling.",
                                               function->name,
                                               drv_node->val);
                                }

                                list_add( &new_button->list, &function->drv_list);
                        }
                        if (num == 0 )
                                syslog(LOG_WARNING, "Function  %s did not have any mapping to a driver button", function->name);
                }

                list_add(&function->list, &buttons);
        }

	uloop_timeout_set(&button_inform_timer, BUTTON_TIMEOUT);

        longpress_find();
	dump_drv_list();
        dump_buttons_list();
}

