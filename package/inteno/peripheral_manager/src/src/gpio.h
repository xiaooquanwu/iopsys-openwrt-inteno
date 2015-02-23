#ifndef GPIO_H
#define GPIO_H

#include <fcntl.h>
#include <sys/ioctl.h>
#include <board.h>

void gpio_open_ioctl(void);
int board_ioctl(int ioctl_id, int action, int hex, char* string_buf, int string_buf_len, int offset);

#endif /* GPIO_H */
