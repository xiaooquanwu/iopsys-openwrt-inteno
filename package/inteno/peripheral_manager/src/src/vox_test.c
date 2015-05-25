#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/select.h>
#include <termios.h>

#include <board.h>
#include "gpio.h"


int debug_level;
struct termios orig_termios;

#define SPI_SLAVE_SELECT 1

void reset_terminal_mode(void);
void set_conio_terminal_mode( void );
int kbhit( void );
int getch( void );
void display(void);
void inc(void);
void dec(void);

void reset_terminal_mode( void)
{
        tcsetattr(0, TCSANOW, &orig_termios);
}

void set_conio_terminal_mode( void )
{
        struct termios new_termios;

        /* take two copies - one for now, one for later */
        tcgetattr(0, &orig_termios);
        memcpy(&new_termios, &orig_termios, sizeof(new_termios));

        /* register cleanup handler, and set the new terminal mode */
        atexit(reset_terminal_mode);
        cfmakeraw(&new_termios);
        tcsetattr(0, TCSANOW, &new_termios);
}

int kbhit( void )
{
        struct timeval tv = { 0L, 0L };
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(0, &fds);
        return select(1, &fds, NULL, NULL, &tv);
}

int getch( void )
{
        int r = 0;
        int c = 0;
        if ((r = read(0, &c, sizeof(c))) < 0) {
                return r;
        } else {
#if __BYTE_ORDER == __BIG_ENDIAN
                int ret = 0;
                ret |= (c >> 24) & 0x000000ff;
                ret |= (c >>  8) & 0x0000ff00;
                ret |= (c <<  8) & 0x00ff0000;
                ret |= (c << 24) & 0xff000000;
                return ret;
#else
                return c;
#endif
        }
}

unsigned char spi_data [6] = {8,3,0,0,0,0};
int pos;

void display(void){

        printf("\r");
        printf("%02x %02x %02x %02x %02x %02x \r",
               spi_data[0],
               spi_data[1],
               spi_data[2],
               spi_data[3],
               spi_data[4],
               spi_data[5]
                );


        if (pos){
                int jump = pos/2;
                printf("\e[%dC",pos+jump);
        }
        fflush(stdout);
}

void inc(void){

        int byte = pos/2;
        int nibble = pos%2;

        int val_hi = (spi_data[byte] >> 4 ) & 0xF;
        int val_lo =  spi_data[byte]        & 0xF;

        if(!nibble) {
                val_hi++;
                if(val_hi > 0xF )
                        val_hi = 0xf;
        }else{
                val_lo++;
                if(val_lo > 0xF )
                        val_lo = 0xf;
        }

        spi_data[byte] = val_hi << 4 | val_lo;
}

void dec(void){
        int byte = pos/2;
        int nibble = pos%2;

        int val_hi = (spi_data[byte] >> 4 ) & 0xF;
        int val_lo =  spi_data[byte]        & 0xF;

        if(!nibble) {
                val_hi--;
                if(val_hi < 0 )
                        val_hi = 0;
        }else{
                val_lo--;
                if(val_lo < 0 )
                        val_lo = 0;
        }

        spi_data[byte] = val_hi << 4 | val_lo;
}

int main(int argc, char *argv[])
{
        int ch;

        gpio_open_ioctl();
        /* arg 4 is the spi mode encoded in a string pointer */
        /* mode is decribed i/bcm963xx/shared/opensource/include/bcm963xx/bcmSpiRes.h */
        board_ioctl(BOARD_IOCTL_SPI_INIT, SPI_SLAVE_SELECT, 0, (char *)0, 0, 391000);
        set_conio_terminal_mode();
        fflush(stdout);

        display();

        while ( 'q' != (char)(ch = getch())) {
                /* right */
                if (ch == 4414235) {
                        pos++;
                        if (pos > 11)
                                pos = 11;
                }
                /* left */
                if (ch == 4479771) {
                        pos--;
                        if (pos < 0)
                                pos = 0;
                }
                /* up */
                if (ch == 4283163) {
                        inc();
                        board_ioctl(BOARD_IOCTL_SPI_WRITE, SPI_SLAVE_SELECT, 0, (char*)spi_data, 6, 0);
                }
                /* down */
                if (ch == 4348699) {
                        dec();
                        board_ioctl(BOARD_IOCTL_SPI_WRITE, SPI_SLAVE_SELECT, 0, (char*)spi_data, 6, 0);
                }
                display();
        }
        return 0;
}
