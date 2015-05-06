#ifndef LED_H
#define LED_H

#include "server.h"

typedef enum {
	OFF,
	ON,
	FLASH_SLOW,
	FLASH_FAST,
	PULSING,
	FADEON,
	FADEOFF,
	NEED_INIT,		/* set on loading config */
	LED_STATES_MAX,
} led_state_t;

typedef enum {
	NONE,
	RED,
	GREEN,
	BLUE,
	YELLOW,
	WHITE,
} led_color_t;

struct led_drv;

struct led_drv_func{
	int         (*set_state)(struct led_drv *, led_state_t);	/* Set led state, on,off,flash ...      */
	led_state_t (*get_state)(struct led_drv *);			/* Get led state, on,off,flash ...	*/
	int         (*set_color)(struct led_drv *, led_color_t);	/* Set led color			*/
	led_color_t (*get_color)(struct led_drv *);			/* Get led color			*/
	int         (*support)  (struct led_drv *, led_state_t);	/* do driver has hardware support for state */
};

struct led_drv {
	const char *name;		/* name, set in the confg file,has to be uniq	*/
	void *priv;			/* for use by the driver			*/
	struct led_drv_func *func;	/* function pointers for controlling the led	*/
};

void led_add( struct led_drv *);
void led_init(struct server_ctx *);

void led_pressindicator_set(void);
void led_pressindicator_clear(void);

#endif /* LED_H */
