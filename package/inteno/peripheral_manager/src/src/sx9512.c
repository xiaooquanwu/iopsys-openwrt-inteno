#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <error.h>
#include <errno.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include "sx9512.h"
#include "smbus.h"
#include "i2c.h"
#include "gpio.h"


#define X(name, reserved, default) { #name, reserved, default },
const struct sx9512_reg_data sx9512_reg_data[] = { SX9512_REGS };
#undef X


//! init the sx9512 and optionally program the registers

//! @param addr I2C address (0=0x2c)
//! @param nvm compare and if different program the registers and flash the NVM with these contents
//! @return file descriptor for SX9512 I2C device
//! @retval -1 error
int sx9512_init(const char *dev, int addr, struct sx9512_reg_nvm *nvm)
{
	int fd, i;
	struct sx9512_reg_nvm nvm_read;
	if(!addr)
		addr=SX9512_I2C_ADDRESS;
	if((fd=i2c_open_dev(dev, addr, I2C_FUNC_SMBUS_READ_BYTE | I2C_FUNC_SMBUS_WRITE_BYTE))<0)
		return -1;
	/*fd = open(dev, O_RDWR);
	if(fd < 0) {
		error(0, errno, "could not open %s", dev);
		return -1;
	}
	if(ioctl(fd, I2C_SLAVE, addr) < 0) {
		error(0, errno, "could not set address %x for i2c chip", SX9512_I2C_ADDRESS);
		close(fd);
		return -1;
	}*/
	if(nvm) {
		if(sx9512_reg_nvm_read(fd, &nvm_read)) {
			close(fd);
			return -1;
		}
		for(i=0;(unsigned int)i<sizeof(struct sx9512_reg_nvm);i++) {
			if(sx9512_reg_reserved(i+SX9512_REG_NVM_AREA_START))
				continue;
			if(((uint8_t *)nvm)[i] != ((uint8_t *)&nvm_read)[i]) {
				fprintf(stderr, "sx9512_init: register mismatch, setting default values and burning to NVM\n");
				if(sx9512_reg_nvm_write(fd, nvm)) {
					close(fd);
					return -1;
				}
				sx9512_reg_nvm_store(fd); //store to NVM
				break;
			}
		}
	}
	return fd;
}


const char *sx9512_reg_name(sx9512_reg_t reg)
{
	return sx9512_reg_data[reg].name;
}


//! SX9512 check if reg is reserved
int sx9512_reg_reserved(sx9512_reg_t reg)
{
	if(reg==SX9512_REG_I2C_SOFT_RESET)
		return 0;
	if(reg>=SX9512_REGS_AMOUNT)
		return 1;
	return sx9512_reg_data[reg].reserved;
}


//! send reset command

//! @retval 0 ok
int sx9512_reset(int fd)
{
	if(i2c_smbus_write_byte_data(fd, SX9512_REG_I2C_SOFT_RESET, 0xde)<0)
		return -1;
	if(i2c_smbus_write_byte_data(fd, SX9512_REG_I2C_SOFT_RESET, 0x00)<0)
		return -1;
	return 0;
}


//! send reset command but retain LED values

//! @retval 0 ok
int sx9512_reset_restore_led_state(int fd)
{
	int r;
	uint8_t p[4];
	if((r=i2c_smbus_read_i2c_block_data(fd, SX9512_REG_LED1_ON, 4, (__u8 *)&p))<0)
		return r;
	if(sx9512_reset(fd))
		return -1;
	if((r=i2c_smbus_write_i2c_block_data(fd, SX9512_REG_LED1_ON, 4, (__u8 *)&p))<0)
		return r;
	return 0;
}


//! read interrupt reg

//! @retval -1 error
int sx9512_read_interrupt(int fd)
{
	int r;
	if((r=i2c_smbus_read_byte_data(fd, SX9512_REG_IRQ_SRC))<0)
		return -1;
	return r;
}


//! read touch reg

//! @retval -1 error
int sx9512_read_buttons(int fd)
{
	int r;
	if((r=i2c_smbus_read_byte_data(fd, SX9512_REG_TOUCH_STATUS))<0)
		return -1;
	return r;
}


//! read prox reg

//! @retval -1 error
int sx9512_read_proximity(int fd)
{
	int r;
	if((r=i2c_smbus_read_byte_data(fd, SX9512_REG_PROX_STATUS))<0)
		return -1;
	return r;
}


//! read status regs

//! @retval 0 ok
int sx9512_read_status(int fd, struct sx9512_reg_status *status)
{
	//REG_IRQ_SRC not working if using block read for some reason.
	//if(i2c_smbus_read_i2c_block_data(fd, SX9512_REG_IRQ_SRC, sizeof(struct sx9512_reg_status), (uint8_t *)status)<0)
		//return -1;
	int r;
	if((r=i2c_smbus_read_byte_data(fd, SX9512_REG_IRQ_SRC))<0)
		return -1;
	((uint8_t *)status)[0]=r;
	if((r=i2c_smbus_read_byte_data(fd, SX9512_REG_TOUCH_STATUS))<0)
		return -1;
	((uint8_t *)status)[1]=r;
	if((r=i2c_smbus_read_byte_data(fd, SX9512_REG_PROX_STATUS))<0)
		return -1;
	((uint8_t *)status)[2]=r;
	return 0;
}


//! read status cached

//! @retval 0 ok
int sx9512_read_status_cached(int fd, struct sx9512_touch_state *touch_state)
{
	static uint8_t last_state=0;
	struct sx9512_reg_status status;
	touch_state->touched=0;
	touch_state->released=0;
	if(sx9512_read_status(fd, &status))
		error(-1, errno, "I2C read error");
	touch_state->state=status.touch_status | !!((*(uint8_t *)&status.prox_status) & 0xc0);
	if(*(uint8_t *)&status.irq_src) {
		if(status.irq_src.touch || status.irq_src.prox_near)
			touch_state->touched = (last_state ^ touch_state->state) & touch_state->state;
		if(status.irq_src.release || status.irq_src.prox_far)
			touch_state->released = (last_state ^ touch_state->state) & last_state;
	}
	last_state=touch_state->state;
	return 0;
}


//! send cmd to load values from NVM to RAM

//! @retval 0 ok
int sx9512_reg_nvm_load(int fd)
{
	if(i2c_smbus_write_byte_data(fd, SX9512_REG_NVM_CTRL, 0x08)<0)
		return -1;
	return 0;
}


//! send cmd to store RAM to NVM

//! @retval 0 ok
int sx9512_reg_nvm_store(int fd)
{
	if(i2c_smbus_write_byte_data(fd, SX9512_REG_NVM_CTRL, 0x50)<0)
		return -1;
	if(i2c_smbus_write_byte_data(fd, SX9512_REG_NVM_CTRL, 0xa0)<0)
		return -1;
	//Wait 500ms then power off/on
	sleep(1);
	return 0;
}


//! read whole NVM region

//! @retval 0 ok
int sx9512_reg_nvm_read(int fd, struct sx9512_reg_nvm *p)
{
	int r, s, i, rl;
	s=sizeof(struct sx9512_reg_nvm);
	for(i=0; i<s; i+=32) {
		rl=s-i;
		if(rl>32)
			rl=32;
		if((r=i2c_smbus_read_i2c_block_data(fd, SX9512_REG_NVM_AREA_START+i, rl, (uint8_t *)p+i))<0)
			return -1;
	}
	return 0;
}


//! write whole NVM region

//! @retval 0 ok
int sx9512_reg_nvm_write(int fd, struct sx9512_reg_nvm *p)
{
	int r, s, i, rl;
	s=sizeof(struct sx9512_reg_nvm);
	for(i=0; i<s; i+=32) {
		rl=s-i;
		if(rl>32)
			rl=32;
		if((r=i2c_smbus_write_i2c_block_data(fd, SX9512_REG_NVM_AREA_START+i, rl, (uint8_t *)p+i))<0)
			return -1;
	}
	return 0;
}


//! init NVM struct
void sx9512_reg_nvm_init(struct sx9512_reg_nvm *p)
{
	memset(p, 0, sizeof(struct sx9512_reg_nvm));
	p->cap_sense_op.set_to_0x14=0x14;
}


//! init NVM struct to defaults
void sx9512_reg_nvm_init_defaults(struct sx9512_reg_nvm *p, uint8_t capsense_channels, uint8_t led_channels)
{
	int i;
	sx9512_reg_nvm_init(p);
	p->irq_mask.touch=1;
	p->irq_mask.release=1;
	p->irq_mask.prox_near=1;
	p->irq_mask.prox_far=1;
	if(led_channels) {
		p->led_map[0]=0x00;
		//p->led_map[1]=led_channels;
		p->led_map[1]=0x00; //default all leds off
		p->led_pwm_freq=0x10;
		p->led_idle=0xff;
		p->led1_on=0xff;
		p->led2_on=0xff;
		p->led_pwr_idle=0xff;
		p->led_pwr_on=0xff;
	}
	p->cap_sense_enable=capsense_channels;
	for(i=0;i<SX9512_CHANNELS+1;i++) {
		p->cap_sense_range[i].ls_control=0x01;
		p->cap_sense_range[i].delta_cin_range=0x03;
		p->cap_sense_thresh[i]=0x04;
	}
	p->cap_sense_thresh[0]=0x02;
	p->cap_sense_op.auto_compensation=0;
	p->cap_sense_op.proximity_bl0=1;
	p->cap_sense_op.proximity_combined_channels=0;
	p->cap_sense_mode.raw_filter=0x03;
	p->cap_sense_mode.touch_reporting=1;
	p->cap_sense_mode.cap_sense_digital_gain=0;
	p->cap_sense_mode.cap_sense_report_mode=0;
	p->cap_sense_debounce.cap_sense_prox_near_debounce=0;
	p->cap_sense_debounce.cap_sense_prox_far_debounce=0;
	p->cap_sense_debounce.cap_sense_touch_debounce=0;
	p->cap_sense_debounce.cap_sense_release_debounce=1;
	p->cap_sense_neg_comp_thresh=0x80;
	p->cap_sense_pos_comp_thresh=0x80;
	p->cap_sense_pos_filt.cap_sense_prox_hyst=0;
	p->cap_sense_pos_filt.cap_sense_pos_comp_debounce=2;
	p->cap_sense_pos_filt.cap_sense_avg_pos_filt_coef=7;
	p->cap_sense_neg_filt.cap_sense_touch_hyst=0;
	p->cap_sense_neg_filt.cap_sense_neg_comp_debounce=2;
	p->cap_sense_neg_filt.cap_sense_avg_neg_filt_coef=5;
	p->spo_chan_map=0xff;
}


void sx9512_reg_nvm_init_cg300(struct sx9512_reg_nvm *p)
{
	int i;
	sx9512_reg_nvm_init_defaults(p, 0x0f, 0x3c);
	p->led_map[0]=0x01;
	p->led_map[1]=0x3c;
	for(i=0;i<SX9512_CHANNELS+1;i++) {
		p->cap_sense_range[i].delta_cin_range=0x01;
	}
}


void sx9512_reg_nvm_init_cg301(struct sx9512_reg_nvm *p)
{
	int i;
	sx9512_reg_nvm_init_defaults(p, 0x3b, 0x7f);
	p->led_map[1]=0x7f;
	for(i=0;i<SX9512_CHANNELS+1;i++) {
		p->cap_sense_range[i].delta_cin_range=0x01;
	}
}


void sx9512_reg_nvm_init_eg300(struct sx9512_reg_nvm *p)
{
	sx9512_reg_nvm_init_defaults(p, 0x0f, 0xff);
}


void sx9512_reg_nvm_init_dg400(struct sx9512_reg_nvm *p)
{
	sx9512_reg_nvm_init_defaults(p, 0x3f, 0x00);
}
