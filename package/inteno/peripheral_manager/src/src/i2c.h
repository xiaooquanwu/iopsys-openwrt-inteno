#ifndef I2C_H
#define I2C_H
/*
struct i2c_reg_tab {
    char addr;
    char value;
    char range;  
};*/

/*
struct i2c_touch{
    int dev;
    int shadow_irq;
    int shadow_touch;
    int shadow_proximity;
    int addr;
    int irq_button;
    const struct i2c_reg_tab *init_tab;
    int init_tab_len;
    const char *name;
} *i2c_touch;*/


//void do_init_tab( struct i2c_touch *i2c_touch);
//struct i2c_touch * i2c_init(struct uci_context *uci_ctx, char* i2c_dev_name, struct i2c_touch* i2c_touch_list, int len);

void dump_i2c(int fd,int start,int stop);
int i2c_open_dev (const char *bus, int addr, unsigned long needed);

#endif
