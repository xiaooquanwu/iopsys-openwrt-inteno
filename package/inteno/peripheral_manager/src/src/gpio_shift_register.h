#ifndef GPIO_SHIFT_REGISTER_H
#define GPIO_SHIFT_REGISTER_H

#include "gpio.h"

typedef int shift_register_index_t;

typedef struct {
	gpio_t clk;
	gpio_t dat;
	gpio_t mask;
	int size;
	int state_cache;
} gpio_shift_register_t;

int gpio_shift_register_init(gpio_shift_register_t *p, gpio_t gpio_clk, gpio_t gpio_dat, gpio_t gpio_mask, int size);
void gpio_shift_register_set(gpio_shift_register_t *p, int state);
void gpio_shift_register_cached_set(gpio_shift_register_t *p, shift_register_index_t address, gpio_state_t bit_val);

#endif