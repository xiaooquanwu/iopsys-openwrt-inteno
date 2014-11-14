#ifndef CATV_H
struct catv_handler;

struct catv_handler * catv_init(char * i2c_bus, int i2c_addr);
void catv_destroy(struct catv_handler *h);

#endif /* CATV_H */
