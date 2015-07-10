#include "gpio_shift_register.h"
#include <stdlib.h>
#include "log.h"

int gpio_shift_register_init(gpio_shift_register_t *p, gpio_t gpio_clk, gpio_t gpio_dat, gpio_t gpio_mask, int size)
{
	p->clk=gpio_clk;
	p->dat=gpio_dat;
	p->mask=gpio_mask;
	p->size=size;
	p->state_cache=0;
	gpio_set_state(p->clk, 0);
	gpio_set_state(p->dat, 0);
	gpio_set_state(p->mask, 0);
	return 0;
}

void gpio_shift_register_set(gpio_shift_register_t *p, int state)
{
	int i;
	if(!p->size)
		return;
	gpio_set_state(p->mask, 0); //mask low
	for(i=p->size; i; i--) {
		gpio_set_state(p->clk, 0); //clk low
		gpio_set_state(p->dat, (state>>(i-1)) & 1); //place bit
		gpio_set_state(p->clk, 1); //clk high
	}
	gpio_set_state(p->mask, 1); //mask high / sreg load
	p->state_cache=state; //update internal register copy
}


void gpio_shift_register_cached_set(gpio_shift_register_t *p, shift_register_index_t index, gpio_state_t state)
{
	if(!p->size)
		return;
	if(!(index < p->size)) {
		syslog(LOG_ERR, "index %d out of bounds", index);
		return;
	}
	//update internal register copy
	if(state)
		p->state_cache |= (1<<index);
	else
		p->state_cache &= ~(1<<index);
	gpio_shift_register_set(p, p->state_cache);
}
