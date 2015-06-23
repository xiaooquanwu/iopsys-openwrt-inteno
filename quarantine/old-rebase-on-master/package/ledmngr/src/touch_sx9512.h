#ifndef TOUCH_SX9512_H
#define TOUCH_SX9512_H

#include "libubus.h"
#include "ucix.h"

#include "led.h"
#include "button.h"

struct i2c_touch;

#define I2C_RESET_TIME (1000 * 60 * 30) /* 30 min in ms */

struct i2c_touch * sx9512_init( struct uci_context *uci_ctx );

void sx9512_led_set( struct led_config* lc, int state);
int  sx9512_check(struct i2c_touch *i2c_touch);
int sx9512_check_button(struct button_config *bc, struct i2c_touch *i2c_touch);

void sx9512_reset(struct i2c_touch *i2c_touch);

#endif /* TOUCH_SX9512_H */
