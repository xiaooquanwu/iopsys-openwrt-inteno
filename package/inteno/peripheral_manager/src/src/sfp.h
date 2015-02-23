#ifndef SFP_H
#include "libubus.h"
#include "ucix.h"

struct sfp_handler;

struct sfp_handler * sfp_init(struct uci_context *uci_ctx);
int sfp_ubus_populate(struct sfp_handler *h, struct ubus_context *ubus_ctx);
void sfp_destroy(struct sfp_handler *h);

#endif /*SFP_H*/
