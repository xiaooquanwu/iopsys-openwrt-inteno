#ifndef BUTTON_H
#define BUTTON_H
#include "server.h"

typedef enum {
	RELEASED,
	PRESSED,
} button_state_t;

struct button_drv;

struct button_drv_func {
	button_state_t (*get_state)(struct button_drv *);		/* Get led state, on,off,flash ...	*/
};

struct button_drv {
	const char *name;		/* name, set in the confg file,has to be uniq	*/
	void *priv;			/* for use by the driver			*/
	struct button_drv_func *func;	/* function pointers for reading the button	*/
};

void button_add( struct button_drv *drv);
void button_init( struct server_ctx *s_ctx);

#endif /* BUTTON_H */
