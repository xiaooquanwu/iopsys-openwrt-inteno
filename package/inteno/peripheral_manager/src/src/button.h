#ifndef BUTTON_H
#define BUTTON_H
#include "server.h"

#define BUTTON_MAX 32
#define BUTTON_MAX_NAME_LEN 16
#define UBUS_BUTTON_NAME_PREPEND "button."
#define UBUS_BUTTON_NAME_PREPEND_LEN sizeof(UBUS_BUTTON_NAME_PREPEND)

typedef enum {
	BUTTON_RELEASED,
	BUTTON_PRESSED,
	BUTTON_ERROR,
} button_state_t;

typedef enum {
	BUTTON_PRESS_NONE,
	BUTTON_PRESS_SHORT,
	BUTTON_PRESS_LONG,
} button_press_type_t;

struct button_drv;

struct button_drv_func {
	button_state_t (*get_state)(struct button_drv *);		/* Get button state, on,off ...	*/
};

struct button_drv {
	const char *name;		/* name, set in the confg file,has to be uniq	*/
	void *priv;			/* for use by the driver			*/
	struct button_drv_func *func;	/* function pointers for reading the button	*/
};

void button_add( struct button_drv *drv);
void button_init( struct server_ctx *s_ctx);

#endif /* BUTTON_H */
