/**  Broadcom libnvram.so compatible wrapper
 *
 * Copyright 2012 Benjamin Larsson <benjamin@southpole.se>
 *
 */

/*

Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
provided that the above copyright notice and this permission notice appear in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN
AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
OF THIS SOFTWARE.

*/


#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "uci.h"


struct uci_context *ctx = NULL;
struct uci_ptr ptr;
int nvram_inited = 0;
int nvram_debug = 0;

/** Function prototypes are taken from bcmnvram.h Copyright Broadcom Corporation.
 *  Only some of the nvram_* functions exposed from libnvram.so are implemented.
 *  get, getall, unset, set, commit
 */


/* uci does not support . in the key part, replace it with _ */

static const char * filter_dots_in_string(char *key) {
	int length = 0;
	int i;
	length = strlen(key);
	for (i=0 ; i<length ; i++) {
		if (key[i] == '.')
			key[i] = '_';
	}
	return key;
}

static void nvram_display_section(struct uci_section *s)
{
	struct uci_element *e;
	struct uci_option *o;

	printf("%s.%s=%s\n", s->package->e.name, s->e.name, s->type);
	uci_foreach_element(&s->options, e) {
		o = uci_to_option(e);
		printf("%s.%s.%s=%s\n", o->section->package->e.name, o->section->e.name, o->e.name, o->v.string);
	}
}

void nvram_init() {
	if (!nvram_inited) {
		ctx = ucix_init("broadcom");
		if(!ctx) {
			printf("Failed to load config file \"broadcom\"\n");
			return;
		}
		ucix_add_section(ctx, "broadcom", "nvram", "broadcom");
		ucix_add_option(ctx, "broadcom", "nvram", "init", "1");
		ucix_commit(ctx, "broadcom");
		nvram_debug = ucix_get_option_int(ctx, "broadcom", "nvram", "debug");
		nvram_inited = 1;
		if (nvram_debug)
			printf("nvram_init()\n");
	}
}


/*
 * Get the value of an NVRAM variable. The pointer returned may be
 * invalid after a set.
 * @param	name	name of variable to get
 * @return	value of variable or NULL if undefined
 */
const char * nvram_get(const char *name) {
	const char *ucitmp;
	nvram_init();
	ucitmp = ucix_get_option(ctx, "broadcom", "nvram", filter_dots_in_string(name));
	if (nvram_debug)
		printf("%s=nvram_get(%s)\n", ucitmp, name);

	return ucitmp;
}


/*
 * Set the value of an NVRAM variable. The name and value strings are
 * copied into private storage. Pointers to previously set values
 * may become invalid. The new value may be immediately
 * retrieved but will not be permanently stored until a commit.
 * @param	name	name of variable to set
 * @param	value	value of variable
 * @return	0 on success and errno on failure
 */
int nvram_set(const char *name, const char *value) {
	nvram_init();
	ucix_add_option(ctx, "broadcom", "nvram", filter_dots_in_string(name), value);
	ucix_commit(ctx, "broadcom");
	if (nvram_debug)
		printf("nvram_set(%s, %s)\n", filter_dots_in_string(name), value);
	return 0;
}


/*
 * Unset an NVRAM variable. Pointers to previously set values
 * remain valid until a set.
 * @param	name	name of variable to unset
 * @return	0 on success and errno on failure
 * NOTE: use nvram_commit to commit this change to flash.
 */
int nvram_unset(const char *name){
	nvram_init();
	ucix_del(ctx, "broadcom", "nvram", filter_dots_in_string(name));
	ucix_commit(ctx, "broadcom");
	if (nvram_debug)
		printf("nvram_unset(%s)\n", filter_dots_in_string(name));
	return 0;
}


/*
 * Commit NVRAM variables to permanent storage. All pointers to values
 * may be invalid after a commit.
 * NVRAM values are undefined after a commit.
 * @return	0 on success and errno on failure
 */
int nvram_commit(void){
	nvram_init();
	ucix_commit(ctx, "broadcom");
	if (nvram_debug)
		printf("nvram_commit()\n");

	return 0;
}


/*
 * Get all NVRAM variables (format name=value\0 ... \0\0).
 * @param	buf	buffer to store variables
 * @param	count	size of buffer in bytes
 * @return	0 on success and errno on failure
 */
int nvram_getall(char *nvram_buf, int count) {
	nvram_init();

	ptr.package = "broadcom";
	ptr.section = "nvram";

	if (uci_lookup_ptr(ctx, &ptr, NULL, true) != UCI_OK)
		return 1;

	if (!(ptr.flags & UCI_LOOKUP_COMPLETE))
		return 1;

	nvram_display_section(ptr.s);

	return 0;
}

