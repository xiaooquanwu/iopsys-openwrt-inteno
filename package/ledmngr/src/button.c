#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <syslog.h>

#include <board.h>

#include "button.h"
#include "log.h"
#include "touch_sx9512.h"


#define MIN_MS_TIME_PRESSED 1000

extern int board_ioctl(int ioctl_id, int action, int hex, char* string_buf, int string_buf_len, int offset);
extern int led_set(struct leds_configuration* led_cfg, int led_idx, int state);
extern int get_led_index_by_name(struct leds_configuration* led_cfg, char* led_name);

/* For i2c buttons but not the near & far we save the time of press event
   so that we can see at release it was pressed long enough.
*/
static void touch_button_press_timer_start(struct button_config* bc)
{
    if (bc->type == I2C && bc->address < 8) {
        if ( bc->pressed_time.tv_sec == 0 )
            clock_gettime(CLOCK_MONOTONIC, &bc->pressed_time);
    }
}

/* For i2c buttons but not the near & far we check that is was
   pressed long enough.
*/
static int  button_press_time_valid(struct button_config* bc, int msec)
{
    struct timespec now;
    int sec;
    int nsec;

    if (bc->type == I2C && bc->address < 8) {
        if ( bc->pressed_time.tv_sec != 0 ) {
            clock_gettime(CLOCK_MONOTONIC, &now);
            sec = now.tv_sec - bc->pressed_time.tv_sec;
            nsec = now.tv_nsec - bc->pressed_time.tv_nsec;

            if ( msec < (sec*1000 + nsec/1000000)) {
                return 1;
            }
        }
    } else {
        return 1;
    }

    return 0;
}

static int button_use_feedback(const struct leds_configuration *led_cfg,
                               const struct button_config* bc)
{
    return (led_cfg->leds_state == LEDS_NORMAL || led_cfg->leds_state == LEDS_PROXIMITY)
        && led_cfg->button_feedback_led >= 0
        /* Touch buttons, excluding proximity. */
        && bc->type == I2C && bc->address < 8;
}

void proximity_dim(struct leds_configuration* led_cfg, int all);

void check_buttons(struct leds_configuration *led_cfg, struct button_configuration *butt_cfg, int initialize) {
    int button, i;
    struct button_config* bc;
    button = 0;
    sx9512_check(butt_cfg->i2c_touch);

    for (i=0 ; i<butt_cfg->button_nr ; i++) {
        bc = butt_cfg->buttons[i];

        if (bc->type == GPIO ){
            button = board_ioctl( BOARD_IOCTL_GET_GPIO, 0, 0, NULL, bc->address, 0);
        }else if (bc->type == I2C){
            button = sx9512_check_button(bc,butt_cfg->i2c_touch);
            if (button < 0)
                continue;
        }

        if (!initialize) {
            if (button^bc->active) {
                DEBUG_PRINT("Button %s pressed\n",bc->name);

                bc->pressed_state = 1;
                touch_button_press_timer_start(bc);

                if (button_use_feedback(led_cfg, bc)) {
                    if ( button_press_time_valid(bc, MIN_MS_TIME_PRESSED) ) {
                        led_cfg->press_indicator = 1;
                    }
                }
                //syslog(LOG_INFO, "Button %s pressed\n",bc->name);

                if(led_cfg->leds_state == LEDS_PROD) {
                    DEBUG_PRINT("Setting %s on\n", bc->feedback_led);
                    if (bc->feedback_led) led_set(led_cfg, get_led_index_by_name(led_cfg, bc->feedback_led), ON);
                }
            }

            if ((!(button^bc->active)) && (bc->pressed_state)) {
                char str[512] = {0};
#if 0
                if (button_use_feedback(led_cfg, bc)) {
                    if (led_cfg->leds_state == LEDS_PROXIMITY
                        && led_cfg->proximity_timer)
                        led_set(led_cfg, led_cfg->button_feedback_led, 1);
                    else
                        led_set(led_cfg, led_cfg->button_feedback_led, -1);
                }
#endif
                if (button_press_time_valid(bc, MIN_MS_TIME_PRESSED)){
                    if ((led_cfg->leds_state == LEDS_NORMAL)    ||
                        (led_cfg->leds_state == LEDS_PROXIMITY) ||
                        (led_cfg->leds_state == LEDS_SILENT)    ||
                        (led_cfg->leds_state == LEDS_INFO)) {

                        if (led_cfg->press_indicator == 1){
                            int i;
                            proximity_dim(led_cfg, 1);
                            for (i=0 ; i<led_cfg->leds_nr ; i++) {
                                led_set(led_cfg, i, -1);
                            }
                            led_cfg->press_indicator = 0;
                        }

                        DEBUG_PRINT("Button %s released, executing hotplug button command: %s\n",bc->name, bc->command);
                        snprintf(str, 512, "ACTION=register INTERFACE=%s /sbin/hotplug-call button &",bc->command);
                        system(str);
                        syslog(LOG_INFO, "ACTION=register INTERFACE=%s /sbin/hotplug-call button", bc->command);
                    } else {
                        DEBUG_PRINT("Button %s released, sending console log output: %s\n",bc->name, bc->command);
                        snprintf(str, 512, "echo %s %s >/dev/console &",bc->name, bc->command);
                        system(str);
                    }
                }
                bc->pressed_state = 0;
                bc->pressed_time.tv_sec = 0;
            }
        } else {
            bc->pressed_time.tv_sec = 0;
        }
    }

}

struct button_configuration* get_button_config(struct uci_context *uci_ctx,struct i2c_touch *i2c_touch) {
    int i;
    const char *butt_names;
    const char *butt_config;
    char *ptr, *rest;
    struct button_configuration* butt_cfg;

    butt_cfg = malloc(sizeof(struct button_configuration));
    memset(butt_cfg,0,sizeof(struct button_configuration));

    butt_cfg->buttons = malloc(MAX_BUTTON * sizeof(struct button_config*));
    memset(butt_cfg->buttons,0,sizeof(struct button_config*));

    /* Initialize */
    if(!uci_ctx) {
        DEBUG_PRINT("Failed to load uci config file \"hw\"\n");
        return NULL;
    }

    butt_cfg->i2c_touch = i2c_touch;

    butt_names = ucix_get_option(uci_ctx, "hw", "board", "buttonnames");
    if (!butt_names) {
        DEBUG_PRINT("No hw.board.buttonnames entry found\n");
        return NULL;
    }

    /* Populate button configuration structure */
    DEBUG_PRINT("Populate button configuration structure\n");
    ptr = strtok_r((char *)butt_names, " ", &rest);
    while(ptr != NULL) {
        struct button_config* bc;
        char type[256];
        char active[256];
        char command[256];
        char feedback_led[256];
        int  address;

        butt_config = ucix_get_option(uci_ctx, "hw", "buttons", ptr);

        bc = malloc(sizeof(struct button_config));
        bc->name = strdup(ptr);
        sscanf(butt_config, "%s %d %s %s %s",type, &address, active, command, feedback_led);
        DEBUG_PRINT("%-15s: %4s %2d %s feedback_led=[%s] \n",command, type,address, active, feedback_led);

        if (!strcmp(active, "al"))   bc->active = ACTIVE_LOW;
        if (!strcmp(active, "ah"))   bc->active = ACTIVE_HIGH;

        if (!strcasecmp(type, "gpio")) bc->type = GPIO;
        if (!strcasecmp(type, "i2c"))  bc->type = I2C;

        bc->command = strdup(command);
        bc->address = address;
        bc->pressed_state = 0;
        bc->feedback_led = strdup(feedback_led);

        /* Get next */
        ptr = strtok_r(NULL, " ", &rest);

        if (butt_cfg->button_nr >= MAX_BUTTON) {
            DEBUG_PRINT("Too many buttons configured!: %d Only adding the %d first\n", butt_cfg->button_nr, MAX_BUTTON);
            return NULL;
        }
        butt_cfg->buttons[butt_cfg->button_nr] = bc;
        butt_cfg->button_nr++;
    }

    for (i=0 ; i<butt_cfg->button_nr ; i++) {
        DEBUG_PRINT("%-15s %-15s %d:%2d:%d \n",
                    butt_cfg->buttons[i]->name,
                    butt_cfg->buttons[i]->command,
                    butt_cfg->buttons[i]->type,
                    butt_cfg->buttons[i]->address,
                    butt_cfg->buttons[i]->active
                    );
    }

    return butt_cfg;
}

#if 0
static int button_need_type(const struct button_configuration* butt_cfg, led_type_t type)
{
    int i;
    for (i=0 ; i<butt_cfg->button_nr ; i++)
        if (butt_cfg->buttons[i]->type == type)
            return 1;
    return 0;
}
#endif
