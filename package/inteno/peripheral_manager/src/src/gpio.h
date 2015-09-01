#ifndef GPIO_H
#define GPIO_H

#include <fcntl.h>
#include <sys/ioctl.h>
#include <board.h>

typedef int gpio_t;

typedef enum {
	GPIO_STATE_LOW,
	GPIO_STATE_HIGH,
} gpio_state_t;

int board_ioctl_init(void);
int board_ioctl(int ioctl_id, int action, int hex, char* string_buf, int string_buf_len, int offset);
#define gpio_init() board_ioctl_init()
gpio_state_t gpio_get_state(gpio_t gpio);
void gpio_set_state(gpio_t gpio, gpio_state_t state);

#endif /* GPIO_H */
