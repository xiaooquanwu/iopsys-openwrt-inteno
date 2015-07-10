#include <stdio.h>
#include <string.h>
#include <unistd.h>
//#include <libgen.h>
#include <stdint.h>
#include <stdlib.h>
#include <fcntl.h>
#include <error.h>
#include <errno.h>
//#include <limits.h>
//#include <sys/types.h>
//#include <sys/stat.h>
//#include <syslog.h>
//#include <config.h>
#include <getopt.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include "smbus.h"
#include "i2c.h"
#include "log.h"
#include "gpio.h"
#include "gpio_shift_register.h"
#include "sx9512.h"

#define DEV_I2C "/dev/i2c-0"

int verbose, debug_level;


#define CMDS \
X(NONE,             "none",             0, 0, "", "") \
X(GPIO_GET,         "gpio_get",         1, 1, "Get pin state", "<pin>") \
X(GPIO_SET,         "gpio_set",         2, 2, "Set pin state", "<pin> <state>") \
X(SH_SET,           "sh_set",           5, 5, "Set shift register state", "<clk> <dat> <mask> <size> <state>") \
X(SMBUS_READ,       "smbus_read",       3, 3, "Read from I2C/SMBUS device", "<addr> <reg> <len>") \
X(SMBUS_WRITE,      "smbus_write",      3, 3, "Write to I2C/SMBUS device", "<addr> <reg> <hex_data>") \
X(SX9512_BUTTON,    "sx9512_button",    0, 0, "Read SX9512 buttons (endless loop)", "") \
X(SX9512_READ,      "sx9512_read",      0, 0, "Look at configuration data (compare to default)", "") \
X(SX9512_INIT,      "sx9512_init",      0, 1, "Init SX9512 config to device defaults", "[device]") \
X(SX9512_NVM_LOAD,  "sx9512_nvm_load",  0, 0, "SX9512 load values from NVM", "") \
X(SX9512_NVM_STORE, "sx9512_nvm_store", 0, 0, "SX9512 store config to NVM", "") \
X(SX9512_RESET,     "sx9512_reset",     0, 0, "Send reset command to SX9512", "")


#define X(id, str, min_arg, max_arg, desc, arg_desc) CMD_##id,
enum { CMDS CMDS_AMOUNT } cmd;
#undef X

struct cmd {
	const char *str;
	int min_arg, max_arg;
	const char *desc, *arg_desc;
};

#define X(id, str, min_arg, max_arg, desc, arg_desc) { str, min_arg, max_arg, desc, arg_desc },
const struct cmd cmd_data[] = { CMDS };
#undef X


#define SX9512_DEVCFGS \
X(NONE,    "none"   ) \
X(DEFAULT, "default") \
X(CLEAR,   "clear"  ) \
X(CG300,   "cg300"  ) \
X(CG301,   "cg301"  ) \
X(EG300,   "eg300"  ) \
X(DG400,   "dg400"  )

#define X(a, b) SX9512_DEVCFG_##a,
enum sx9512_devcfg { SX9512_DEVCFGS SX9512_DEVCFG_AMOUNT };
#undef X

#define X(a, b) b,
const char *sx9512_devcfg_str[] = { SX9512_DEVCFGS };
#undef X


static enum sx9512_devcfg sx9512_devcfg_str_to_id(const char *s)
{
	int i;
	for(i=0; i<SX9512_DEVCFG_AMOUNT; i++) {
		if(!strcmp(s, sx9512_devcfg_str[i]))
			return i;
	}
	return 0;
}


static void print_usage(char *prg_name)
{
	int i;
	char tmp[64];
	printf("Usage: %s [options...] <cmd> <arg(s)>\n", prg_name);
	printf("Options:\n");
	printf("  -v, --verbose  Verbose output\n");
	printf("  -h, --help     Show this help screen.\n");
	printf("Commands:\n");
	for(i=0;i<CMDS_AMOUNT;i++) {
		sprintf(tmp, "%s %s", cmd_data[i].str, cmd_data[i].arg_desc);
		printf("  %-40.40s %s\n", tmp, cmd_data[i].desc);
	}
	printf("\n");
}


int main(int argc, char **argv)
{
	int i, j, ch, r, fd=0;
	int pin, state;
	int pin_clk, pin_dat, pin_mask;
	gpio_shift_register_t p;
	int addr=0, s, n, l;
	enum sx9512_devcfg devcfg=0;
	uint8_t tmp[32];
	char *str_value=0, *eptr, str_hex[3];
	struct sx9512_reg_nvm nvm, nvm_def;
	while(1) {
		int option_index = 0;
		static struct option long_options[] = {
			{"verbose", no_argument,       0, 'v'},
			{"help",    no_argument,       0, 'h'},
			{0, 0, 0, 0}
		};
		ch = getopt_long(argc, argv, "vh", long_options, &option_index);
		if(ch == -1)
			break;
		switch (ch) {
			case 'v':
				verbose=1;
				break;
			case 'h':
			default:
				print_usage(argv[0]);
				exit(-1);
		}
	}
	//i=argc-optind;
	if((argc-optind)<1) {
		print_usage(argv[0]);
		error(-1, errno, "Error: need cmd");
	}
	for(i=0;i<CMDS_AMOUNT;i++) {
		if(!strcmp(argv[optind], cmd_data[i].str))
			cmd=i;
	}
	if(!cmd) {
		print_usage(argv[0]);
		error(-1, errno, "Error: bad cmd %s", argv[optind]);
	}
	optind++;
	if((argc-optind)<cmd_data[cmd].min_arg) {
		print_usage(argv[0]);
		error(-1, errno, "Error: too few arguments");
	}
	if((argc-optind)>cmd_data[cmd].max_arg) {
		print_usage(argv[0]);
		error(-1, errno, "Error: too many arguments");
	}
	switch(cmd) {
	case CMD_GPIO_GET:
	case CMD_GPIO_SET:
	case CMD_SH_SET:
		gpio_init();
		break;
	case CMD_SMBUS_READ:
	case CMD_SMBUS_WRITE:
		addr=strtol(argv[optind],NULL,16);
		optind++;
		if(verbose)
			printf("Open I2C device %s\n", DEV_I2C);
		fd = open(DEV_I2C, O_RDWR);
		if(fd < 0)
			error(-1, errno, "could not open %s", DEV_I2C);
		if(verbose)
			printf("Set I2C addr=%02x\n", addr);
		if(ioctl(fd, I2C_SLAVE, addr) < 0) {
			error(0, errno, "could not set address %x for i2c chip", addr);
			close(fd);
			return -1;
		}
		break;
	case CMD_SX9512_BUTTON:
	case CMD_SX9512_READ:
	case CMD_SX9512_INIT:
	case CMD_SX9512_NVM_LOAD:
	case CMD_SX9512_NVM_STORE:
	case CMD_SX9512_RESET:
		if((fd=sx9512_init(DEV_I2C, SX9512_I2C_ADDRESS, NULL))<0)
			error(-1, errno, "could not init SX9512");
		break;
	default:
		break;
	}

	switch(cmd) {
	case CMD_GPIO_GET:
		pin=strtol(argv[optind],0,0);
		if(verbose)
			printf("Get gpio %d state\n", pin);
		r=gpio_get_state(pin);
		if(verbose)
			printf("state=%d\n", r);
		return(r);
	case CMD_GPIO_SET:
		pin=strtol(argv[optind],0,0);
		optind++;
		state=strtol(argv[optind],0,0);
		if(state!=0 && state!=1) {
			print_usage(argv[0]);
			error(-1, errno, "Error: bad state %d", state);
		}
		if(verbose)
			printf("Set gpio %d state to %d\n", pin, state);
		gpio_set_state(pin, state);
		break;
	case CMD_SH_SET:
		pin_clk=strtol(argv[optind],NULL,0);
		optind++;
		pin_dat=strtol(argv[optind],NULL,0);
		optind++;
		pin_mask=strtol(argv[optind],NULL,0);
		optind++;
		s=strtol(argv[optind],NULL,0);
		optind++;
		state=strtol(argv[optind],NULL,16);
		if(verbose)
			printf("Set shift register (clk=%d, dat=%d, mask=%d, size=%d) state to %X\n", pin_clk, pin_dat, pin_mask, s, state);
		gpio_shift_register_init(&p, pin_clk, pin_dat, pin_mask, s);
		gpio_shift_register_set(&p, state);
		break;
	case CMD_SMBUS_READ:
		s=strtol(argv[optind],NULL,16);
		optind++;
		n=strtol(argv[optind],NULL,0);
		if(s+n>256)
			n=256-s;
		if(verbose)
			printf("smbus read start (addr=%02x, reg=%02x, len=%d)\n", addr, s, n);
		for(i=s; i<(s+n); i+=32) {
			l=n-(i-s);
			if(l>32)
				l=32;
			if(verbose)
				printf("smbus read (reg=%02x, len=%d)\n", i, l);
			r=i2c_smbus_read_i2c_block_data(fd, i, l, (__u8 *)&tmp);
			if(r<0) {
				error(0, errno, "I2C read error (%d)", r);
				close(fd);
				return(-1);
			}
			printf("%02X:", i/32);
			for(j=0; j<l; j++)
				printf("%02x", tmp[j]);
			printf("\n");
		}
		close(fd);
		if(n==1)
			return(tmp[0]);
		break;
	case CMD_SMBUS_WRITE:
		s=strtol(argv[optind],NULL,16);
		optind++;
		str_value = argv[optind];
		n=strlen(str_value);
		if(n%2)
			error(-1, errno, "Error: odd length hex value %s", str_value);
		n>>=1;
		if(s+n>256)
			n=256-s;
		if(verbose)
			printf("smbus write start (addr=%02x, reg=%02x, len=%d, val=%s)\n", addr, s, n, str_value);
		for(i=0; i<n; i+=32) {
			l=n-i;
			if(l>32)
				l=32;
			str_hex[2]=0;
			for(j=0; j<l; j++) {
				str_hex[0]=str_value[(i+j)<<1];
				str_hex[1]=str_value[((i+j)<<1)+1];
				tmp[j]=strtol(str_hex, &eptr,16);
				if((errno != 0 && tmp[j] == 0) || eptr==str_hex)
					error(-1, errno, "hex conversion error at %d (%s)", j, str_hex);
			}
			if(verbose)
				printf("smbus write (reg=%02x, len=%d, val=%.*s)\n", s+i, l, l*2, str_value+(i*2));
			r=i2c_smbus_write_i2c_block_data(fd, s+i, l, tmp);
			if(r<0) {
				error(0, errno, "I2C write error (%d)", r);
				close(fd);
				return(-1);
			}
			printf("%02X:", i/32);
			for(j=0; j<l; j++)
				printf("%02x ", tmp[j]);
			printf("\n");
		}
		close(fd);
		break;
	case CMD_SX9512_BUTTON:
		while(1) {
			if(verbose)
				printf("Start reading buttons from SX9512\n");
			struct sx9512_touch_state touch_state;
			if(sx9512_read_status_cached(fd, &touch_state))
				error(-1, errno, "I2C read error");
			//printf("[state %02X]\n", touch_state.state);
			if(touch_state.touched)
				printf("[touch %02X]\n", touch_state.touched);
			if(touch_state.released)
				printf("[release %02X]\n", touch_state.released);
			fflush(stdout);
			sleep(1);
		}
		break;
	case CMD_SX9512_READ:
		if(verbose)
			printf("Read SX9512 registers (and compare to defaults)\n");
		sx9512_reg_nvm_init_defaults(&nvm_def, 0xff, 0xff);
		if(sx9512_reg_nvm_read(fd, &nvm))
			error(-1, errno, "while reading nvm registers");
		s=sizeof(nvm);
		for(i=0; i<s; i++)
			printf("%02x: %02x (%02x)0 %s\n", SX9512_REG_NVM_AREA_START+i, ((uint8_t *)&nvm)[i], ((uint8_t *)&nvm_def)[i], sx9512_reg_name(SX9512_REG_NVM_AREA_START+i));
		break;
	case CMD_SX9512_INIT:
		if((argc-optind)==1)
			devcfg = sx9512_devcfg_str_to_id(argv[optind]);
		switch(devcfg) {
		case SX9512_DEVCFG_DEFAULT:
			sx9512_reg_nvm_init_defaults(&nvm, 0xff, 0xff);
			break;
		case SX9512_DEVCFG_CLEAR:
			memset(&nvm, 0, sizeof(nvm));
			break;
		case SX9512_DEVCFG_CG300:
			sx9512_reg_nvm_init_cg300(&nvm);
			break;
		case SX9512_DEVCFG_CG301:
			sx9512_reg_nvm_init_cg301(&nvm);
			break;
		case SX9512_DEVCFG_EG300:
			sx9512_reg_nvm_init_eg300(&nvm);
			break;
		case SX9512_DEVCFG_DG400:
			sx9512_reg_nvm_init_dg400(&nvm);
			break;
		default:
			fprintf(stderr, "Error: bad device arg, valid options are:\n");
			for(i=0;i<SX9512_DEVCFG_AMOUNT;i++)
				fprintf(stderr, "%s ", sx9512_devcfg_str[i]);
			fprintf(stderr, "\n");
			return -1;
		}
		if(verbose)
			printf("Init SX9512 registers to %s\n", sx9512_devcfg_str[devcfg]);
		if(sx9512_reg_nvm_write(fd, &nvm))
			error(-1, errno, "while writing nvm registers");
		break;
	case CMD_SX9512_NVM_LOAD:
		if(sx9512_reg_nvm_load(fd))
			error(-1, errno, "while loading nvm registers");
		break;
	case CMD_SX9512_NVM_STORE:
		if(sx9512_reg_nvm_store(fd))
			error(-1, errno, "while storing nvm registers");
		break;
	case CMD_SX9512_RESET:
		if(sx9512_reset(fd))
			error(-1, errno, "while trying to reset");
		break;
	default:
		break;
	}
	fflush(stdout);
	return 0;
}
