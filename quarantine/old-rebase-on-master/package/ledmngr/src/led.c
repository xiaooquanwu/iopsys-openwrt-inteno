#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "led.h"
#include "log.h"

int add_led(struct leds_configuration* led_cfg, char* led_name, const char* led_config, led_color_t color)
{
    struct led_config* lc;
    char type[256];
    char active[256];
    char function[256];
    int  address;

    if (!led_config) {
        return -1;
    }

    lc = malloc(sizeof(struct led_config));

    DEBUG_PRINT("Led %s: %s\n",led_name, led_config);

    if (led_cfg->leds_nr >= MAX_LEDS) {
        DEBUG_PRINT("Too many leds configured! Only adding the %d first\n", MAX_LEDS);
        return -1;
    }

    lc->name = strdup(led_name);
    sscanf(led_config, "%s %d %s %s", type, &address, active, function);

    if (!strcmp(type, "gpio")) lc->type = GPIO;
    if (!strcmp(type, "sr"))   lc->type = SHIFTREG2;
    if (!strcmp(type, "csr"))  lc->type = SHIFTREG3;
    if (!strcmp(type, "i2c"))  lc->type = I2C;
    if (!strcmp(type, "spi"))  lc->type = SPI;

    lc->address = address;
    lc->color = color;

    if (!strcmp(active, "al"))   lc->active = ACTIVE_LOW;
    if (!strcmp(active, "ah"))   lc->active = ACTIVE_HIGH;

    /* FIXME: Add to configuration file? Maybe we also want to
       exclude WAN_ leds on CG300. */
    if (!strncmp(lc->name, "Status_", 7) || !strncmp(lc->name, "WAN_", 4))
        lc->use_proximity = 0;
    else
        lc->use_proximity = 1;

    if (!strcmp(lc->name, "Status_green"))
        led_cfg->button_feedback_led = led_cfg->leds_nr;

    led_cfg->leds[led_cfg->leds_nr] = lc;
    led_cfg->leds_nr++;
    return 0;
}
