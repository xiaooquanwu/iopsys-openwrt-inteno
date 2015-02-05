#include <syslog.h>
#include "log.h"
#include "button.h"


/* PUT every led from drivers into a list */
struct drv_button_list{
	struct list_head list;
	struct button_drv *drv;
};
LIST_HEAD(drv_buttons_list);

void button_add( struct button_drv *drv)
{
	struct drv_button_list *drv_node = malloc(sizeof(struct drv_button_list));

	DBG(1,"called with led name [%s]\n", drv->name);
	drv_node->drv = drv;

	list_add(&drv_node->list, &drv_buttons_list);
}

static void dump_drv_list(void)
{
	struct list_head *i;
	list_for_each(i, &drv_buttons_list) {
		struct drv_button_list *node = list_entry(i, struct drv_button_list, list);
		DBG(1,"button name = [%s]\n",node->drv->name);
	}
}

void button_init( struct server_ctx *s_ctx)
{
	dump_drv_list();
}

