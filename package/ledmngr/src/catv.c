#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <math.h>

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
#include "log.h"

#include "libubus.h"

struct catv_handler
{
    int i2c_a0;
    int i2c_a2;
};

static struct catv_handler *pcatv;

void catv_get_type(struct blob_buf *b)
{
    int type;
    char *s;

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
    char buf[12+1];
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
    char buf[20+1];
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
    char buf[20+1];
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
    char buf[8+1];
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
    char buf[4+1];
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
    char buf[16+1];
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

static void catv_get_interface(struct blob_buf *b)
{
    int type;
    char *s;

    type = i2c_smbus_read_byte_data(pcatv->i2c_a0,81);

    switch (type) {
    case 0:
        s = "GPIO";
        break;
    case 1:
        s = "I2C";
        break;
    case 2:
        s = "SPI";
        break;
    default:
        s="Error reading data";
    }
 
    blobmsg_add_string(b, "Interface",s );
}

static int catv_get_interface_method(struct ubus_context *ubus_ctx, struct ubus_object *obj,
                                     struct ubus_request_data *req, const char *method,
                                     struct blob_attr *msg)
{
    struct blob_buf b;

    memset(&b, 0, sizeof(b));
    blob_buf_init(&b, 0);
    catv_get_interface(&b);
    ubus_send_reply(ubus_ctx, req, b.head);
    return 0;
}

static void catv_get_bandwidth(struct blob_buf *b)
{
    int type;
    char *s;

    type = i2c_smbus_read_byte_data(pcatv->i2c_a0,82);

    switch (type) {
    case 0:
        s = "47MHz~870MHz";
        break;
    case 1:
        s = "47MHz~1000MHz";
        break;
    case 2:
        s = "54MHz~1000MHz";
        break;
    case 3:
        s = "85MHz~1000MHz";
        break;
    default:
        s="Error reading data";
    }

    blobmsg_add_string(b, "RF bandwidth",s );
}

static int catv_get_bandwidth_method(struct ubus_context *ubus_ctx, struct ubus_object *obj,
                                     struct ubus_request_data *req, const char *method,
                                     struct blob_attr *msg)
{
    struct blob_buf b;

    memset(&b, 0, sizeof(b));
    blob_buf_init(&b, 0);
    catv_get_bandwidth(&b);
    ubus_send_reply(ubus_ctx, req, b.head);
    return 0;
}

static void catv_get_wavelength(struct blob_buf *b)
{
    int num;
    char buf[10];
    num = i2c_smbus_read_byte_data(pcatv->i2c_a0,83);

    snprintf(buf, 10, "%d nm",num*10 );
    blobmsg_add_string(b, "Receiver wavelength",buf );
}

static int catv_get_wavelength_method(struct ubus_context *ubus_ctx, struct ubus_object *obj,
                                     struct ubus_request_data *req, const char *method,
                                     struct blob_attr *msg)
{
    struct blob_buf b;

    memset(&b, 0, sizeof(b));
    blob_buf_init(&b, 0);
    catv_get_wavelength(&b);
    ubus_send_reply(ubus_ctx, req, b.head);
    return 0;
}
static void catv_get_responsivity(struct blob_buf *b)
{
    int num;
    char buf[15];
    num = i2c_smbus_read_byte_data(pcatv->i2c_a0,84);

    snprintf(buf, 15, "%1.2f",num*0.01 );
    blobmsg_add_string(b, "Responsivity",buf );
}

static int catv_get_responsivity_method(struct ubus_context *ubus_ctx, struct ubus_object *obj,
                                     struct ubus_request_data *req, const char *method,
                                     struct blob_attr *msg)
{
    struct blob_buf b;

    memset(&b, 0, sizeof(b));
    blob_buf_init(&b, 0);
    catv_get_responsivity(&b);
    ubus_send_reply(ubus_ctx, req, b.head);
    return 0;
}

static void catv_get_minoutput(struct blob_buf *b)
{
    int num;
    char buf[15];
    num = i2c_smbus_read_byte_data(pcatv->i2c_a0,85);

    snprintf(buf, 15, "%d dBmV",num );
    blobmsg_add_string(b, "Minimum RF output",buf );
}

static int catv_get_minoutput_method(struct ubus_context *ubus_ctx, struct ubus_object *obj,
                                     struct ubus_request_data *req, const char *method,
                                     struct blob_attr *msg)
{
    struct blob_buf b;

    memset(&b, 0, sizeof(b));
    blob_buf_init(&b, 0);
    catv_get_minoutput(&b);
    ubus_send_reply(ubus_ctx, req, b.head);
    return 0;
}

static void catv_get_maxoptical(struct blob_buf *b)
{
    int num;
    char buf[15];
    signed char value;
    num = i2c_smbus_read_byte_data(pcatv->i2c_a0,86);

    value = (signed char)(num & 0xff);

    snprintf(buf, 15, "%2.1f dBmV",value * 0.1 );
    blobmsg_add_string(b, "Maximum Optical Input Power", buf);
}

static int catv_get_maxoptical_method(struct ubus_context *ubus_ctx, struct ubus_object *obj,
                                     struct ubus_request_data *req, const char *method,
                                     struct blob_attr *msg)
{
    struct blob_buf b;

    memset(&b, 0, sizeof(b));
    blob_buf_init(&b, 0);
    catv_get_maxoptical(&b);
    ubus_send_reply(ubus_ctx, req, b.head);
    return 0;
}

static void catv_get_minoptical(struct blob_buf *b)
{
    int num;
    char buf[15];
    signed char value;
    num = i2c_smbus_read_byte_data(pcatv->i2c_a0,87);

    value = (signed char)(num & 0xff);

    snprintf(buf, 15, "%2.1f dBmV",value * 0.1 );
    blobmsg_add_string(b, "Minimum Optical Input Power", buf);
}

static int catv_get_minoptical_method(struct ubus_context *ubus_ctx, struct ubus_object *obj,
                                     struct ubus_request_data *req, const char *method,
                                     struct blob_attr *msg)
{
    struct blob_buf b;

    memset(&b, 0, sizeof(b));
    blob_buf_init(&b, 0);
    catv_get_minoptical(&b);
    ubus_send_reply(ubus_ctx, req, b.head);
    return 0;
}

static void catv_get_templimit(struct blob_buf *b)
{
    char buf[15];
    float temp;

    i2c_smbus_read_i2c_block_data(pcatv->i2c_a2, 0, 2, (__u8*)buf);
    temp = ((signed short)(buf[0]<<8 | (buf[1] &0xff) ))/256.0;
    snprintf(buf, 15, "%3.4f", temp );
    blobmsg_add_string(b, "Temp Hi Alarm", buf);

    i2c_smbus_read_i2c_block_data(pcatv->i2c_a2, 2, 2, (__u8*)buf);
    temp = ((signed short)(buf[0]<<8 | (buf[1] &0xff) ))/256.0;
    snprintf(buf, 15, "%3.4f", temp );
    blobmsg_add_string(b, "Temp Lo Alarm", buf);

    i2c_smbus_read_i2c_block_data(pcatv->i2c_a2, 4, 2, (__u8*)buf);
    temp = ((signed short)(buf[0]<<8 | (buf[1] &0xff) ))/256.0;
    snprintf(buf, 15, "%3.4f", temp );
    blobmsg_add_string(b, "Temp Hi Warning", buf);

    i2c_smbus_read_i2c_block_data(pcatv->i2c_a2, 6, 2, (__u8*)buf);
    temp = ((signed short)(buf[0]<<8 | (buf[1] &0xff) ))/256.0;
    snprintf(buf, 15, "%3.4f", temp );
    blobmsg_add_string(b, "Temp Lo Warning", buf);
}

static int catv_get_templimit_method(struct ubus_context *ubus_ctx, struct ubus_object *obj,
                                     struct ubus_request_data *req, const char *method,
                                     struct blob_attr *msg)
{
    struct blob_buf b;

    memset(&b, 0, sizeof(b));
    blob_buf_init(&b, 0);
    catv_get_templimit(&b);
    ubus_send_reply(ubus_ctx, req, b.head);
    return 0;
}


static void catv_get_vcclimit(struct blob_buf *b)
{
    char buf[15];
    float vcc;

    i2c_smbus_read_i2c_block_data(pcatv->i2c_a2, 8, 2, (__u8*)buf);
    vcc = 2.44 * (1 + (((unsigned short)(buf[0]<<8 | (buf[1] & 0xff) ))/512.0) );

    snprintf(buf, 15, "%2.1f", vcc);
    blobmsg_add_string(b, "VCC Hi Alarm", buf);

    i2c_smbus_read_i2c_block_data(pcatv->i2c_a2, 10, 2, (__u8*)buf);
    vcc = 2.44 * (1 + (((unsigned short)(buf[0]<<8 | (buf[1] & 0xff) ))/512.0) );
    snprintf(buf, 15, "%2.1f", vcc );
    blobmsg_add_string(b, "VCC Lo Alarm", buf);

    i2c_smbus_read_i2c_block_data(pcatv->i2c_a2, 12, 2, (__u8*)buf);
    vcc = 2.44 * (1 + (((unsigned short)(buf[0]<<8 | (buf[1] & 0xff) ))/512.0) );
    snprintf(buf, 15, "%2.1f", vcc );
    blobmsg_add_string(b, "VCC Hi Warning", buf);

    i2c_smbus_read_i2c_block_data(pcatv->i2c_a2, 14, 2, (__u8*)buf);
    vcc = 2.44 * (1 + (((unsigned short)(buf[0]<<8 | (buf[1] & 0xff) ))/512.0) );
    snprintf(buf, 15, "%2.1f", vcc );
    blobmsg_add_string(b, "VCC Lo Warning", buf);
}

static int catv_get_vcclimit_method(struct ubus_context *ubus_ctx, struct ubus_object *obj,
                                     struct ubus_request_data *req, const char *method,
                                     struct blob_attr *msg)
{
    struct blob_buf b;

    memset(&b, 0, sizeof(b));
    blob_buf_init(&b, 0);
    catv_get_vcclimit(&b);
    ubus_send_reply(ubus_ctx, req, b.head);
    return 0;
}
static void catv_get_vpdlimit(struct blob_buf *b)
{
    char buf[15];
    float vpd;
    float resp;

    resp = i2c_smbus_read_byte_data(pcatv->i2c_a0,84);
    resp = resp * 0.01;

    i2c_smbus_read_i2c_block_data(pcatv->i2c_a2, 16, 2, (__u8*)buf);

    vpd = 2.44/1024 * (short)(buf[0]<<8 | (buf[1] & 0xff)) ;
    vpd = 10*log(vpd/(0.47*4.9)/resp);
    snprintf(buf, 15, "%2.1f", vpd);
    blobmsg_add_string(b, "VPD Hi Alarm", buf);

    i2c_smbus_read_i2c_block_data(pcatv->i2c_a2, 18, 2, (__u8*)buf);
    vpd = 2.44/1024 * (short)(buf[0]<<8 | (buf[1] & 0xff)) ;
    vpd = 10*log(vpd/(0.47*4.9)/resp);
    snprintf(buf, 15, "%2.1f", vpd );
    blobmsg_add_string(b, "VPD Lo Alarm", buf);

    i2c_smbus_read_i2c_block_data(pcatv->i2c_a2, 20, 2, (__u8*)buf);
    vpd = 2.44/1024 * (short)(buf[0]<<8 | (buf[1] & 0xff)) ;
    vpd = 10*log(vpd/(0.47*4.9)/resp);
    snprintf(buf, 15, "%2.1f", vpd );
    blobmsg_add_string(b, "VPD Hi Warning", buf);

    i2c_smbus_read_i2c_block_data(pcatv->i2c_a2, 22, 2, (__u8*)buf);
    vpd = 2.44/1024 * (short)(buf[0]<<8 | (buf[1] & 0xff)) ;
    vpd = 10*log(vpd/(0.47*4.9)/resp);
    snprintf(buf, 15, "%2.1f", vpd );
    blobmsg_add_string(b, "VPD Lo Warning", buf);

}

static int catv_get_vpdlimit_method(struct ubus_context *ubus_ctx, struct ubus_object *obj,
                                     struct ubus_request_data *req, const char *method,
                                     struct blob_attr *msg)
{
    struct blob_buf b;

    memset(&b, 0, sizeof(b));
    blob_buf_init(&b, 0);
    catv_get_vpdlimit(&b);
    ubus_send_reply(ubus_ctx, req, b.head);
    return 0;
}


static void catv_get_rflimit(struct blob_buf *b)
{
    char buf[15];
    float vpd;

    i2c_smbus_read_i2c_block_data(pcatv->i2c_a2, 24, 2, (__u8*)buf);

    vpd = 2.44/1024.0 * (unsigned short)(buf[0]<<8 | (buf[1] & 0xff)) ;
    vpd = (vpd + 0.9148)/ 0.0582;
    snprintf(buf, 15, "%2.1f", vpd);
    blobmsg_add_string(b, "RF Hi Alarm", buf);

    i2c_smbus_read_i2c_block_data(pcatv->i2c_a2, 26, 2, (__u8*)buf);
    vpd = 2.44/1024.0 * (unsigned short)(buf[0]<<8 | (buf[1] & 0xff)) ;
    vpd = (vpd + 0.9148)/ 0.0582;
    snprintf(buf, 15, "%2.1f", vpd );
    blobmsg_add_string(b, "RF Lo Alarm", buf);

    i2c_smbus_read_i2c_block_data(pcatv->i2c_a2, 28, 2, (__u8*)buf);
    vpd = 2.44/1024.0 * (unsigned short)(buf[0]<<8 | (buf[1] & 0xff)) ;
    vpd = (vpd + 0.9148)/ 0.0582;
    snprintf(buf, 15, "%2.1f", vpd );
    blobmsg_add_string(b, "RF Hi Warning", buf);

    i2c_smbus_read_i2c_block_data(pcatv->i2c_a2, 30, 2, (__u8*)buf);
    vpd = 2.44/1024.0 * (unsigned short)(buf[0]<<8 | (buf[1] & 0xff)) ;
    vpd = (vpd + 0.9148)/ 0.0582;
    snprintf(buf, 15, "%2.1f", vpd );
    blobmsg_add_string(b, "RF Lo Warning", buf);
}

static int catv_get_rflimit_method(struct ubus_context *ubus_ctx, struct ubus_object *obj,
                                     struct ubus_request_data *req, const char *method,
                                     struct blob_attr *msg)
{
    struct blob_buf b;

    memset(&b, 0, sizeof(b));
    blob_buf_init(&b, 0);
    catv_get_rflimit(&b);
    ubus_send_reply(ubus_ctx, req, b.head);
    return 0;
}

static void catv_get_firmware(struct blob_buf *b)
{
    char buf[15];
    unsigned short version;

    i2c_smbus_read_i2c_block_data(pcatv->i2c_a2, 56, 2, (__u8*)buf);

    version = (unsigned short)(buf[0]<<8 | (buf[1] & 0xff)) ;
    snprintf(buf, 15, "0x%04x", version);
    blobmsg_add_string(b, "Firmware version", buf);
}

static int catv_get_firmware_method(struct ubus_context *ubus_ctx, struct ubus_object *obj,
                                     struct ubus_request_data *req, const char *method,
                                     struct blob_attr *msg)
{
    struct blob_buf b;

    memset(&b, 0, sizeof(b));
    blob_buf_init(&b, 0);
    catv_get_firmware(&b);
    ubus_send_reply(ubus_ctx, req, b.head);
    return 0;
}

static void catv_get_temp(struct blob_buf *b)
{
    char buf[15];
    float temp;

    i2c_smbus_read_i2c_block_data(pcatv->i2c_a2, 58, 2, (__u8*)buf);
    temp = ((signed short)(buf[0]<<8 | (buf[1] &0xff) ))/256.0;

    snprintf(buf, 15, "%3.4f", temp);
    blobmsg_add_string(b, "Temperature", buf);
}

static int catv_get_temp_method(struct ubus_context *ubus_ctx, struct ubus_object *obj,
                                     struct ubus_request_data *req, const char *method,
                                     struct blob_attr *msg)
{
    struct blob_buf b;

    memset(&b, 0, sizeof(b));
    blob_buf_init(&b, 0);
    catv_get_temp(&b);
    ubus_send_reply(ubus_ctx, req, b.head);
    return 0;
}

static void catv_get_vcc(struct blob_buf *b)
{
    char buf[15];
    float vcc;

    i2c_smbus_read_i2c_block_data(pcatv->i2c_a2, 60, 2, (__u8*)buf);
    vcc = 2.44 * (1 + (((unsigned short)(buf[0]<<8 | (buf[1] & 0xff) ))/512.0) );

    snprintf(buf, 15, "%2.2f", vcc);
    blobmsg_add_string(b, "VCC", buf);
}

static int catv_get_vcc_method(struct ubus_context *ubus_ctx, struct ubus_object *obj,
                                     struct ubus_request_data *req, const char *method,
                                     struct blob_attr *msg)
{
    struct blob_buf b;

    memset(&b, 0, sizeof(b));
    blob_buf_init(&b, 0);
    catv_get_vcc(&b);
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
    catv_get_type(&b);
    catv_get_vendor_partnum(&b);
    catv_get_revision(&b);
    catv_get_serial(&b);
    catv_get_date(&b);
    catv_get_interface(&b);
    catv_get_bandwidth(&b);
    catv_get_wavelength(&b);
    catv_get_responsivity(&b);
    catv_get_minoutput(&b);
    catv_get_maxoptical(&b);
    catv_get_minoptical(&b);
    catv_get_templimit(&b);
    catv_get_vcclimit(&b);
    catv_get_vpdlimit(&b);
    catv_get_rflimit(&b);
    catv_get_firmware(&b);
    catv_get_temp(&b);
    catv_get_vcc(&b);

    ubus_send_reply(ubus_ctx, req, b.head);

    return 0;
}

static const struct ubus_method catv_methods[] = {
    { .name = "partnumber",   .handler = catv_get_partnum_method },
    { .name = "vendor",   .handler = catv_get_vendor_method },
    { .name = "type",   .handler = catv_get_type_method},
    { .name = "vendornumber",   .handler = catv_get_vendor_partnum_method },
    { .name = "revision", .handler = catv_get_revision_method },
    { .name = "serial",   .handler = catv_get_serial_method },
    { .name = "date", .handler = catv_get_date_method },
    { .name = "interface", .handler = catv_get_interface_method },
    { .name = "bandwidth", .handler = catv_get_bandwidth_method },
    { .name = "wavelength", .handler = catv_get_wavelength_method },
    { .name = "responsivity", .handler = catv_get_responsivity_method },
    { .name = "minoutput", .handler = catv_get_minoutput_method },
    { .name = "maxoptical", .handler = catv_get_maxoptical_method },
    { .name = "minoptical", .handler = catv_get_minoptical_method },
    { .name = "templimit", .handler = catv_get_templimit_method },
    { .name = "vcclimit", .handler = catv_get_vcclimit_method },
    { .name = "vpdlimit", .handler = catv_get_vpdlimit_method },
    { .name = "rflimit", .handler = catv_get_rflimit_method },
    { .name = "firmware", .handler = catv_get_firmware_method },
    { .name = "temp", .handler = catv_get_temp_method },
    { .name = "vcc", .handler = catv_get_vcc_method },

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

    return ret;
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

