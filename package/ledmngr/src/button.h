#ifndef BUTTON_H
#define BUTTON_H

#include <time.h>

#include "ucix.h"

typedef enum {
    ACTIVE_HIGH,
    ACTIVE_LOW,
} button_active_t;

#include "led.h"

struct button_config {
    char*   name;
    int     address;
    int     active;
    char*   command;
    int     pressed_state;
    struct timespec pressed_time;       /* Store the time we get a press detection */
    int     press_min_time;             /* time in miliseconds that button needs to be pressed to be valid */
    led_type_t type;

    /* feedback_led feature is currently not activated. All leds used in proximity detect is flashed instead. */
    /* should that instead be encoded as a special led, ??? */
    char*   feedback_led;       /* led to flash when pressed */
};

struct button_configuration {
    struct i2c_touch *i2c_touch;
    int         button_nr;
    struct button_config**  buttons;
};

void check_buttons(struct leds_configuration *led_cfg, struct button_configuration *butt_cfg, int initialize);
struct button_configuration* get_button_config(struct uci_context *uci_ctx, struct i2c_touch *i2c_touch);

#endif /* BUTTON_H */
