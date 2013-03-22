// uci test program Copyright Benjamin Larsson 2012 <benjamin@southpole.se>


#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <uci.h>
#include "ucix.h"


int main(int argc, char **argv)
{
	struct uci_context *ctx;
	const char *ucitmp;
	
	ctx = ucix_init("broadcom");
	if(!ctx)
		printf("Failed to load config file");
	
	ucitmp = ucix_get_option(ctx, "broadcom", "nvram", "test");
	printf("test = %s\n",ucitmp);
	ucix_add_section(ctx, "broadcom", "nvram", "broadcom");
	ucix_add_option(ctx, "broadcom", "nvram", "test", "tomte");
	ucix_add_option(ctx, "broadcom", "nvram", "test2", "tomte2");
	printf("Hello world\n");
	ucix_commit(ctx, "broadcom");
	ucitmp = ucix_get_option(ctx, "broadcom", "nvram", "test");
	printf("test = %s\n",ucitmp);
	ucix_cleanup(ctx);
}