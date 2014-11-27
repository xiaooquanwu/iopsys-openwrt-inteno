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

#define LED_FUNCTIONS 14
#define MAX_LEDS 20
#define SR_MAX 16
#define MAX_BUTTON 10

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

struct led_action {
    int		led_index;
    led_state_t	led_state;
} led_action;

struct led_map {
    char*   led_function;
    char*   led_name;
    int     led_actions_nr;
    struct led_action led_actions[LED_ACTION_MAX];
};

struct leds_configuration {
    int             leds_nr;
    struct led_config**  leds;
    int fd;
    int shift_register_state[SR_MAX];
    led_action_t led_fn_action[LED_FUNCTIONS];
    struct led_map led_map_config[LED_FUNCTIONS][LED_ACTION_MAX];

    /* If >= 0, index for the led used for button and proximity
       feedback. */
    int button_feedback_led;
    leds_state_t leds_state;
    int test_state;
    /* Number of blink_handler ticks the buttons should stay lit up */
    unsigned long proximity_timer; /* For active leds */
    unsigned long proximity_all_timer; /* For all leds */
};

int add_led(struct leds_configuration* led_cfg, char* led_name, const char* led_config, led_color_t color);

#endif /* LED_H */
