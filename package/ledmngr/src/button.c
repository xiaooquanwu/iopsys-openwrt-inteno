#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "button.h"
#include "log.h"

struct button_configuration* get_button_config(struct uci_context *uci_ctx) {
    int i;
    const char *butt_names;
    const char *butt_config;
    char *ptr, *rest;
    struct button_configuration* butt_cfg;

    butt_cfg = malloc(sizeof(struct button_configuration));
    memset(butt_cfg,0,sizeof(butt_cfg));

    butt_cfg->buttons = malloc(MAX_BUTTON * sizeof(struct button_config*));
    memset(butt_cfg->buttons,0,sizeof(butt_cfg->buttons));

    /* Initialize */
    if(!uci_ctx) {
        DEBUG_PRINT("Failed to load uci config file \"hw\"\n");
        return NULL;
    }

    butt_names = ucix_get_option(uci_ctx, "hw", "board", "buttonnames");
    if (!butt_names) {
        DEBUG_PRINT("No hw.board.buttonnames entry found\n");
        return NULL;
    }

    /* Populate button configuration structure */
    DEBUG_PRINT("Populate button configuration structure\n");
    ptr = strtok_r((char *)butt_names, " ", &rest);
    while(ptr != NULL) {
        struct button_config* bc;
        char type[256];
        char active[256];
        char command[256];
        char feedback_led[256];
        int  address;

        butt_config = ucix_get_option(uci_ctx, "hw", "buttons", ptr);

        bc = malloc(sizeof(struct button_config));
        bc->name = strdup(ptr);
        sscanf(butt_config, "%s %d %s %s %s",type, &address, active, command, feedback_led);
        DEBUG_PRINT("%-15s: %4s %2d %s %s\n",command, type,address, active, feedback_led);

        if (!strcmp(active, "al"))   bc->active = ACTIVE_LOW;
        if (!strcmp(active, "ah"))   bc->active = ACTIVE_HIGH;

        if (!strcasecmp(type, "gpio")) bc->type = GPIO;
        if (!strcasecmp(type, "i2c"))  bc->type = I2C;

        bc->command = strdup(command);
        bc->address = address;
        bc->pressed_state = 0;
        bc->feedback_led = strdup(feedback_led);

        /* Get next */
        ptr = strtok_r(NULL, " ", &rest);

        if (butt_cfg->button_nr >= MAX_BUTTON) {
            DEBUG_PRINT("Too many buttons configured! Only adding the %d first\n", MAX_BUTTON);
            return NULL;
        }
        butt_cfg->buttons[butt_cfg->button_nr] = bc;
        butt_cfg->button_nr++;
    }

    for (i=0 ; i<butt_cfg->button_nr ; i++) {
        DEBUG_PRINT("%-15s %-15s %d:%2d:%d \n",
                    butt_cfg->buttons[i]->name,
                    butt_cfg->buttons[i]->command,
                    butt_cfg->buttons[i]->type,
                    butt_cfg->buttons[i]->address,
                    butt_cfg->buttons[i]->active
                    );
    }

    /* Initialize the buttons, sometimes the button gpios are left in a pressed state, reading them 10 times should fix that */
    for (i=0 ; i<10 ; i++)
        check_buttons(1);
    DEBUG_PRINT("Buttons initialized\n");

    return butt_cfg;
}
