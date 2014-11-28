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
    struct timespec pressed_time;
    led_type_t type;
    char*   feedback_led;
};

struct button_configuration {
    int         button_nr;
    struct button_config**  buttons;
};

struct button_configuration* get_button_config(struct uci_context *uci_ctx);

#endif /* BUTTON_H */
