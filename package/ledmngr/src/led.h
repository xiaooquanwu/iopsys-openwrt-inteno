#ifndef LED_H
#define LED_H

typedef enum {
    OFF,
    ON,
    BLINK_SLOW,
    BLINK_FAST,
    LED_STATES_MAX,
} led_state_t;

typedef enum {
    RED,
    GREEN,
    BLUE,
    YELLOW,
} led_color_t;

typedef enum {
    GPIO,
    LEDCTL,
    SHIFTREG2,
    SHIFTREG3,
    I2C,
} led_type_t;

typedef enum {
    LED_OFF,
    LED_OK,
    LED_NOTICE,
    LED_ALERT,
    LED_ERROR,
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


#include "button.h"

struct led_config {
    /* Configuration */
    char*	name;
    char*	function;
    led_color_t	color;
    led_type_t	type;
    int		address;
    button_active_t active;
    int		use_proximity;
    /* State */
    led_state_t	state;
    int		blink_state;
};

#endif /* LED_H */
