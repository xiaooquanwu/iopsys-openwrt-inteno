#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include "smbus.h"
#include <linux/i2c.h>
#include <linux/i2c-dev.h>

#include "i2c.h"
#include "catv.h"

#include "libubus.h"

struct catv_handler
{
    int i2c_a0;
    int i2c_a2;
};

static struct catv_handler *pcatv;

void catv_get_type(struct blob_buf *b)
{
    char buf[12],*s;
    int type;
    memset(buf, 0, sizeof(buf));
    type = i2c_smbus_read_byte_data(pcatv->i2c_a0,32);

    switch (type) {
    case 0:
        s = "CATV Receiver";
        break;
    case 1:
        s = "RFoG";
        break;
    case 2:
        s = "Satelite Fiber";
        break;
    case 3:
        s = "Fiber Node";
        break;
    default:
        s="Error reading data";
    }

    blobmsg_add_string(b, "Type", s);

}
static int catv_get_type_method(struct ubus_context *ubus_ctx, struct ubus_object *obj,
                                   struct ubus_request_data *req, const char *method,
                                   struct blob_attr *msg)
{
    struct blob_buf b;

    memset(&b, 0, sizeof(b));
    blob_buf_init(&b, 0);
    catv_get_type(&b);
    ubus_send_reply(ubus_ctx, req, b.head);

    return 0;
}


void catv_get_partnum(struct blob_buf *b)
{
    char buf[12];
    memset(buf, 0, sizeof(buf));
    i2c_smbus_read_i2c_block_data(pcatv->i2c_a0, 0, 12, (__u8*)buf);

    blobmsg_add_string(b, "Part number",buf );

}
static int catv_get_partnum_method(struct ubus_context *ubus_ctx, struct ubus_object *obj,
                                   struct ubus_request_data *req, const char *method,
                                   struct blob_attr *msg)
{
    struct blob_buf b;

    memset(&b, 0, sizeof(b));
    blob_buf_init(&b, 0);
    catv_get_partnum(&b);
    ubus_send_reply(ubus_ctx, req, b.head);

    return 0;
}

void catv_get_vendor(struct blob_buf *b)
{
    char buf[20];
    memset(buf, 0, sizeof(buf));
    i2c_smbus_read_i2c_block_data(pcatv->i2c_a0, 12, 20, (__u8*)buf);

    blobmsg_add_string(b, "Vendor",buf );

}

static int catv_get_vendor_method(struct ubus_context *ubus_ctx, struct ubus_object *obj,
                                   struct ubus_request_data *req, const char *method,
                                   struct blob_attr *msg)
{
    struct blob_buf b;

    memset(&b, 0, sizeof(b));
    blob_buf_init(&b, 0);
    catv_get_vendor(&b);
    ubus_send_reply(ubus_ctx, req, b.head);

    return 0;
}

void catv_get_vendor_partnum(struct blob_buf *b)
{
    char buf[20];
    memset(buf, 0, sizeof(buf));
    i2c_smbus_read_i2c_block_data(pcatv->i2c_a0, 33, 20, (__u8*)buf);

    blobmsg_add_string(b, "Vendor part number",buf );

}
static int catv_get_vendor_partnum_method(struct ubus_context *ubus_ctx, struct ubus_object *obj,
                                   struct ubus_request_data *req, const char *method,
                                   struct blob_attr *msg)
{
    struct blob_buf b;

    memset(&b, 0, sizeof(b));
    blob_buf_init(&b, 0);
    catv_get_vendor_partnum(&b);
    ubus_send_reply(ubus_ctx, req, b.head);

    return 0;
}

void catv_get_date(struct blob_buf *b)
{
    char buf[8];
    memset(buf, 0, sizeof(buf));
    i2c_smbus_read_i2c_block_data(pcatv->i2c_a0, 73, 8, (__u8*)buf);

    blobmsg_add_string(b, "Date",buf );

}

static int catv_get_date_method(struct ubus_context *ubus_ctx, struct ubus_object *obj,
                                   struct ubus_request_data *req, const char *method,
                                   struct blob_attr *msg)
{
    struct blob_buf b;

    memset(&b, 0, sizeof(b));
    blob_buf_init(&b, 0);
    catv_get_date(&b);
    ubus_send_reply(ubus_ctx, req, b.head);

    return 0;
}



void catv_get_revision(struct blob_buf *b)
{
    char buf[4];
    memset(buf, 0, sizeof(buf));
    i2c_smbus_read_i2c_block_data(pcatv->i2c_a0, 53, 4, (__u8*)buf);

    blobmsg_add_string(b, "Revision",buf );

}
static int catv_get_revision_method(struct ubus_context *ubus_ctx, struct ubus_object *obj,
                                  struct ubus_request_data *req, const char *method,
                                  struct blob_attr *msg)
{
    struct blob_buf b;

    memset(&b, 0, sizeof(b));
    blob_buf_init(&b, 0);
    catv_get_revision(&b);
    ubus_send_reply(ubus_ctx, req, b.head);

    return 0;
}

static void catv_get_serial(struct blob_buf *b)
{
    char buf[16];
    memset(buf, 0, sizeof(buf));

    i2c_smbus_read_i2c_block_data(pcatv->i2c_a0, 57, 16, (__u8*)buf);
    blobmsg_add_string(b, "Serial",buf );
}

static int catv_get_serial_method(struct ubus_context *ubus_ctx, struct ubus_object *obj,
                                  struct ubus_request_data *req, const char *method,
                                  struct blob_attr *msg)
{
    struct blob_buf b;

    memset(&b, 0, sizeof(b));
    blob_buf_init(&b, 0);
    catv_get_serial(&b);
    ubus_send_reply(ubus_ctx, req, b.head);
    return 0;
}


static int catv_get_all_method(struct ubus_context *ubus_ctx, struct ubus_object *obj,
                               struct ubus_request_data *req, const char *method,
                               struct blob_attr *msg)
{
    struct blob_buf b;

    memset(&b, 0, sizeof(b));
    blob_buf_init (&b, 0);

    catv_get_partnum(&b);
    catv_get_vendor(&b);
    catv_get_vendor_partnum(&b);
    catv_get_revision(&b);
    catv_get_serial(&b);
    catv_get_date(&b);
    catv_get_type(&b);

    ubus_send_reply(ubus_ctx, req, b.head);

    return 0;
}

static const struct ubus_method catv_methods[] = {
    { .name = "type",   .handler = catv_get_type_method},
    { .name = "partnumber",   .handler = catv_get_partnum_method },
    { .name = "vendor",   .handler = catv_get_vendor_method },
    { .name = "vendornumber",   .handler = catv_get_vendor_partnum_method },
    { .name = "serial",   .handler = catv_get_serial_method },
    { .name = "revision", .handler = catv_get_revision_method },
    { .name = "date", .handler = catv_get_date_method },
    { .name = "get-all",  .handler = catv_get_all_method },
};

static struct ubus_object_type catv_type =
    UBUS_OBJECT_TYPE("catv", catv_methods);


static struct ubus_object catv_object = {
    .name = "catv", .type = &catv_type,
    .methods = catv_methods, ARRAY_SIZE(catv_methods) };

int catv_ubus_populate(struct catv_handler *h, struct ubus_context *ubus_ctx)
{
    int ret;

    ret = ubus_add_object (ubus_ctx, &catv_object);

    return 0;
}

struct catv_handler * catv_init(char *i2c_bus,int a0_addr,int a2_addr)
{
    struct catv_handler *h;

    printf("%s:\n",__func__);

    h = malloc( sizeof(struct catv_handler) );

    if (!h)
        return NULL;

    h->i2c_a0 = i2c_open_dev(i2c_bus, a0_addr,
                             I2C_FUNC_SMBUS_READ_BYTE | I2C_FUNC_SMBUS_WRITE_BYTE);

    if (h->i2c_a0 == -1 ){
        syslog(LOG_INFO,"Did not find any CATV device at %s address %x \n", i2c_bus, a0_addr);
        free(h);
        return 0;
    }

    h->i2c_a2 = i2c_open_dev(i2c_bus, a2_addr,
                             I2C_FUNC_SMBUS_READ_BYTE | I2C_FUNC_SMBUS_WRITE_BYTE);

    if (h->i2c_a2 == -1 ){
        syslog(LOG_INFO,"Did not find any CATV device at %s address %x \n", i2c_bus, a2_addr);
        close(h->i2c_a0);
        free(h);
        return 0;
    }


    dump_i2c(h->i2c_a0,0,255);
    dump_i2c(h->i2c_a2,0,255);

    pcatv = h;
    return h;
}

void catv_destroy(struct catv_handler *h)
{
    close(h->i2c_a0);
    close(h->i2c_a2);
    free(h);
}

