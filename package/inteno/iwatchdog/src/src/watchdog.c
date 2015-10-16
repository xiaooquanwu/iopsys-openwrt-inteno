#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

const char *watchdog_file = "/proc/watchdog";
const char *init_string   = "1 5000000 1 4";
const char *kicker        = "OK";

int main(int argc, char **argv)
{
        int fd = open(watchdog_file, O_WRONLY);
        if (fd < 0) {
                perror("Open watchdog file");
                exit(1);
        }

        /* init */
        int res = write (fd, init_string, strlen(init_string) + 1);

        if (res < 0 ) {
                perror("Error init watchdog");
                exit(1);
        }

        while (1) {
                sleep(1);
                res = write (fd, kicker, strlen(init_string) + 1);

                if (res < 0 ){
                        perror("Error kicking watchdog");
                }
        }
        return 0;
}
