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

#include "libubus.h"
#include <uci_config.h>
#include <uci.h>
#include "ucix.h"


#include "i2c.h"
#include "catv.h"
#include "log.h"


struct catv_handler
{
    int i2c_a0;
    int i2c_a2;
    struct uci_context *ctx;
};

static struct catv_handler *pcatv;

static int catv_get_type(struct blob_buf *b)
{
    int type;
    char *s;

    type = i2c_smbus_read_byte_data(pcatv->i2c_a0,32);

    if (type < 0)
        return UBUS_STATUS_NO_DATA;

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

    return UBUS_STATUS_OK;
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

    return UBUS_STATUS_OK;
}

static int catv_get_partnum(struct blob_buf *b)
{
    char buf[12+1];
    int ret;
    memset(buf, 0, sizeof(buf));

    ret = i2c_smbus_read_i2c_block_data(pcatv->i2c_a0, 0, 12, (__u8*)buf);
    if(ret < 0)
        return UBUS_STATUS_NO_DATA;

    blobmsg_add_string(b, "Part number",buf );

    return UBUS_STATUS_OK;
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

    return UBUS_STATUS_OK;
}

static int catv_get_vendor(struct blob_buf *b)
{
    char buf[20+1];
    int ret;
    memset(buf, 0, sizeof(buf));

    ret = i2c_smbus_read_i2c_block_data(pcatv->i2c_a0, 12, 20, (__u8*)buf);
    if(ret < 0)
        return UBUS_STATUS_NO_DATA;

    blobmsg_add_string(b, "Vendor",buf );

    return UBUS_STATUS_OK;
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

    return UBUS_STATUS_OK;
}

static int catv_get_vendor_partnum(struct blob_buf *b)
{
    char buf[20+1];
    int ret;
    memset(buf, 0, sizeof(buf));

    ret = i2c_smbus_read_i2c_block_data(pcatv->i2c_a0, 33, 20, (__u8*)buf);
    if(ret < 0)
        return UBUS_STATUS_NO_DATA;

    blobmsg_add_string(b, "Vendor part number",buf );

    return UBUS_STATUS_OK;
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

    return UBUS_STATUS_OK;
}

static int catv_get_date(struct blob_buf *b)
{
    char buf[8+1];
    int ret;
    memset(buf, 0, sizeof(buf));

    ret = i2c_smbus_read_i2c_block_data(pcatv->i2c_a0, 73, 8, (__u8*)buf);
    if(ret < 0)
        return UBUS_STATUS_NO_DATA;

    blobmsg_add_string(b, "Date",buf );

    return UBUS_STATUS_OK;
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

    return UBUS_STATUS_OK;
}



static int catv_get_revision(struct blob_buf *b)
{
    char buf[4+1];
    int ret;
    memset(buf, 0, sizeof(buf));

    ret = i2c_smbus_read_i2c_block_data(pcatv->i2c_a0, 53, 4, (__u8*)buf);
    if(ret < 0)
        return UBUS_STATUS_NO_DATA;

    blobmsg_add_string(b, "Revision",buf );

    return UBUS_STATUS_OK;
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

    return UBUS_STATUS_OK;
}

static int catv_get_serial(struct blob_buf *b)
{
    char buf[16+1];
    int ret;
    memset(buf, 0, sizeof(buf));

    ret = i2c_smbus_read_i2c_block_data(pcatv->i2c_a0, 57, 16, (__u8*)buf);
    if(ret < 0)
        return UBUS_STATUS_NO_DATA;

    blobmsg_add_string(b, "Serial",buf );

    return UBUS_STATUS_OK;
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

    return UBUS_STATUS_OK;
}

static int catv_get_interface(struct blob_buf *b)
{
    int type;
    char *s;

    type = i2c_smbus_read_byte_data(pcatv->i2c_a0,81);

    if(type < 0)
        return UBUS_STATUS_NO_DATA;

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

    return UBUS_STATUS_OK;
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

    return UBUS_STATUS_OK;
}

static int catv_get_bandwidth(struct blob_buf *b)
{
    int type;
    char *s;

    type = i2c_smbus_read_byte_data(pcatv->i2c_a0,82);

    if(type < 0)
        return UBUS_STATUS_NO_DATA;

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

    return UBUS_STATUS_OK;
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

    return UBUS_STATUS_OK;
}

static int catv_get_wavelength(struct blob_buf *b)
{
    int num;
    char buf[10];
    num = i2c_smbus_read_byte_data(pcatv->i2c_a0,83);

    if(num < 0)
        return UBUS_STATUS_NO_DATA;

    snprintf(buf, 10, "%d nm",num*10 );
    blobmsg_add_string(b, "Receiver wavelength",buf );

    return UBUS_STATUS_OK;
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

    return UBUS_STATUS_OK;
}

static int catv_get_responsivity(struct blob_buf *b)
{
    int num;
    char buf[15];
    num = i2c_smbus_read_byte_data(pcatv->i2c_a0,84);
    if(num < 0)
        return UBUS_STATUS_NO_DATA;

    snprintf(buf, 15, "%1.2f",num*0.01 );
    blobmsg_add_string(b, "Responsivity",buf );

    return UBUS_STATUS_OK;
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

    return UBUS_STATUS_OK;
}

static int catv_get_minoutput(struct blob_buf *b)
{
    int num;
    char buf[15];
    num = i2c_smbus_read_byte_data(pcatv->i2c_a0,85);
    if(num < 0)
        return UBUS_STATUS_NO_DATA;

    snprintf(buf, 15, "%d dBmV",num );
    blobmsg_add_string(b, "Minimum RF output",buf );

    return UBUS_STATUS_OK;
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

    return UBUS_STATUS_OK;
}

static int catv_get_maxoptical(struct blob_buf *b)
{
    int num;
    char buf[15];
    signed char value;
    num = i2c_smbus_read_byte_data(pcatv->i2c_a0,86);
    if(num < 0)
        return UBUS_STATUS_NO_DATA;

    value = (signed char)(num & 0xff);

    snprintf(buf, 15, "%2.1f dBmV",value * 0.1 );
    blobmsg_add_string(b, "Maximum Optical Input Power", buf);

    return UBUS_STATUS_OK;
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

    return UBUS_STATUS_OK;
}

static int catv_get_minoptical(struct blob_buf *b)
{
    int num;
    char buf[15];
    signed char value;
    num = i2c_smbus_read_byte_data(pcatv->i2c_a0,87);

    if(num < 0)
        return UBUS_STATUS_NO_DATA;

    value = (signed char)(num & 0xff);

    snprintf(buf, 15, "%2.1f dBmV",value * 0.1 );
    blobmsg_add_string(b, "Minimum Optical Input Power", buf);

    return UBUS_STATUS_OK;
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

    return UBUS_STATUS_OK;
}

static int catv_get_templimit(struct blob_buf *b)
{
    char buf[15];
    float temp;
    int ret;

    ret = i2c_smbus_read_i2c_block_data(pcatv->i2c_a2, 0, 2, (__u8*)buf);
    if(ret < 0)
        return UBUS_STATUS_NO_DATA;

    temp = ((signed short)(buf[0]<<8 | (buf[1] &0xff) ))/256.0;
    snprintf(buf, 15, "%3.4f", temp );
    blobmsg_add_string(b, "Temp Hi Alarm", buf);

    ret = i2c_smbus_read_i2c_block_data(pcatv->i2c_a2, 2, 2, (__u8*)buf);
    temp = ((signed short)(buf[0]<<8 | (buf[1] &0xff) ))/256.0;
    snprintf(buf, 15, "%3.4f", temp );
    blobmsg_add_string(b, "Temp Lo Alarm", buf);

    ret = i2c_smbus_read_i2c_block_data(pcatv->i2c_a2, 4, 2, (__u8*)buf);
    temp = ((signed short)(buf[0]<<8 | (buf[1] &0xff) ))/256.0;
    snprintf(buf, 15, "%3.4f", temp );
    blobmsg_add_string(b, "Temp Hi Warning", buf);

    ret = i2c_smbus_read_i2c_block_data(pcatv->i2c_a2, 6, 2, (__u8*)buf);
    temp = ((signed short)(buf[0]<<8 | (buf[1] &0xff) ))/256.0;
    snprintf(buf, 15, "%3.4f", temp );
    blobmsg_add_string(b, "Temp Lo Warning", buf);

    return UBUS_STATUS_OK;
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

    return UBUS_STATUS_OK;
}


static int catv_get_vcclimit(struct blob_buf *b)
{
    char buf[15];
    float vcc;
    int ret;

    ret = i2c_smbus_read_i2c_block_data(pcatv->i2c_a2, 8, 2, (__u8*)buf);
    if(ret < 0)
        return UBUS_STATUS_NO_DATA;

    vcc = 2.44 * (1 + (((unsigned short)(buf[0]<<8 | (buf[1] & 0xff) ))/512.0) );

    snprintf(buf, 15, "%2.1f", vcc);
    blobmsg_add_string(b, "VCC Hi Alarm", buf);

    ret = i2c_smbus_read_i2c_block_data(pcatv->i2c_a2, 10, 2, (__u8*)buf);
    vcc = 2.44 * (1 + (((unsigned short)(buf[0]<<8 | (buf[1] & 0xff) ))/512.0) );
    snprintf(buf, 15, "%2.1f", vcc );
    blobmsg_add_string(b, "VCC Lo Alarm", buf);

    ret = i2c_smbus_read_i2c_block_data(pcatv->i2c_a2, 12, 2, (__u8*)buf);
    vcc = 2.44 * (1 + (((unsigned short)(buf[0]<<8 | (buf[1] & 0xff) ))/512.0) );
    snprintf(buf, 15, "%2.1f", vcc );
    blobmsg_add_string(b, "VCC Hi Warning", buf);

    ret = i2c_smbus_read_i2c_block_data(pcatv->i2c_a2, 14, 2, (__u8*)buf);
    vcc = 2.44 * (1 + (((unsigned short)(buf[0]<<8 | (buf[1] & 0xff) ))/512.0) );
    snprintf(buf, 15, "%2.1f", vcc );
    blobmsg_add_string(b, "VCC Lo Warning", buf);

    return UBUS_STATUS_OK;
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

    return UBUS_STATUS_OK;
}

static int catv_get_vpdlimit(struct blob_buf *b)
{
    char buf[15];
    float vpd;
    float resp;
    int ret;

    resp = i2c_smbus_read_byte_data(pcatv->i2c_a0,84);
    resp = resp * 0.01;

    ret = i2c_smbus_read_i2c_block_data(pcatv->i2c_a2, 16, 2, (__u8*)buf);
    if(ret < 0)
        return UBUS_STATUS_NO_DATA;

    vpd = 2.44/1024 * (short)(buf[0]<<8 | (buf[1] & 0xff)) ;
    vpd = 10*log(vpd/(0.47*4.9)/resp)/log(10);
    snprintf(buf, 15, "%2.1f", vpd);
    blobmsg_add_string(b, "VPD Hi Alarm", buf);

    ret = i2c_smbus_read_i2c_block_data(pcatv->i2c_a2, 18, 2, (__u8*)buf);
    vpd = 2.44/1024 * (short)(buf[0]<<8 | (buf[1] & 0xff)) ;
    vpd = 10*log(vpd/(0.47*4.9)/resp)/log(10);
    snprintf(buf, 15, "%2.1f", vpd );
    blobmsg_add_string(b, "VPD Lo Alarm", buf);

    ret = i2c_smbus_read_i2c_block_data(pcatv->i2c_a2, 20, 2, (__u8*)buf);
    vpd = 2.44/1024 * (short)(buf[0]<<8 | (buf[1] & 0xff)) ;
    vpd = 10*log(vpd/(0.47*4.9)/resp)/log(10);
    snprintf(buf, 15, "%2.1f", vpd );
    blobmsg_add_string(b, "VPD Hi Warning", buf);

    ret = i2c_smbus_read_i2c_block_data(pcatv->i2c_a2, 22, 2, (__u8*)buf);
    vpd = 2.44/1024 * (short)(buf[0]<<8 | (buf[1] & 0xff)) ;
    vpd = 10*log(vpd/(0.47*4.9)/resp)/log(10);
    snprintf(buf, 15, "%2.1f", vpd );
    blobmsg_add_string(b, "VPD Lo Warning", buf);

    return UBUS_STATUS_OK;
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

    return UBUS_STATUS_OK;
}


static int catv_get_rflimit(struct blob_buf *b)
{
    char buf[15];
    float vpd;
    int ret;

    ret = i2c_smbus_read_i2c_block_data(pcatv->i2c_a2, 24, 2, (__u8*)buf);
    if(ret < 0)
        return UBUS_STATUS_NO_DATA;

    vpd = 2.44/1024.0 * (unsigned short)(buf[0]<<8 | (buf[1] & 0xff)) ;
    vpd = (vpd + 0.9148)/ 0.0582;
    snprintf(buf, 15, "%2.1f", vpd);
    blobmsg_add_string(b, "RF Hi Alarm", buf);

    ret = i2c_smbus_read_i2c_block_data(pcatv->i2c_a2, 26, 2, (__u8*)buf);
    vpd = 2.44/1024.0 * (unsigned short)(buf[0]<<8 | (buf[1] & 0xff)) ;
    vpd = (vpd + 0.9148)/ 0.0582;
    snprintf(buf, 15, "%2.1f", vpd );
    blobmsg_add_string(b, "RF Lo Alarm", buf);

    ret = i2c_smbus_read_i2c_block_data(pcatv->i2c_a2, 28, 2, (__u8*)buf);
    vpd = 2.44/1024.0 * (unsigned short)(buf[0]<<8 | (buf[1] & 0xff)) ;
    vpd = (vpd + 0.9148)/ 0.0582;
    snprintf(buf, 15, "%2.1f", vpd );
    blobmsg_add_string(b, "RF Hi Warning", buf);

    ret = i2c_smbus_read_i2c_block_data(pcatv->i2c_a2, 30, 2, (__u8*)buf);
    vpd = 2.44/1024.0 * (unsigned short)(buf[0]<<8 | (buf[1] & 0xff)) ;
    vpd = (vpd + 0.9148)/ 0.0582;
    snprintf(buf, 15, "%2.1f", vpd );
    blobmsg_add_string(b, "RF Lo Warning", buf);

    return UBUS_STATUS_OK;
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

    return UBUS_STATUS_OK;
}

static int catv_get_firmware(struct blob_buf *b)
{
    char buf[15];
    unsigned short version;
    int ret;

    ret = i2c_smbus_read_i2c_block_data(pcatv->i2c_a2, 56, 2, (__u8*)buf);
    if(ret < 0)
        return UBUS_STATUS_NO_DATA;

    version = (unsigned short)(buf[0]<<8 | (buf[1] & 0xff)) ;
    snprintf(buf, 15, "0x%04x", version);
    blobmsg_add_string(b, "Firmware version", buf);

    return UBUS_STATUS_OK;
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

    return UBUS_STATUS_OK;
}

static int catv_get_temp(struct blob_buf *b)
{
    char buf[15];
    float temp;
    int ret;

    ret = i2c_smbus_read_i2c_block_data(pcatv->i2c_a2, 58, 2, (__u8*)buf);
    if(ret < 0)
        return UBUS_STATUS_NO_DATA;

    temp = ((signed short)(buf[0]<<8 | (buf[1] &0xff) ))/256.0;

    snprintf(buf, 15, "%3.4f", temp);
    blobmsg_add_string(b, "Temperature", buf);

    return UBUS_STATUS_OK;
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

    return UBUS_STATUS_OK;
}

static int catv_get_vcc(struct blob_buf *b)
{
    char buf[15];
    float vcc;
    int ret;

    ret = i2c_smbus_read_i2c_block_data(pcatv->i2c_a2, 60, 2, (__u8*)buf);
    if(ret < 0)
        return UBUS_STATUS_NO_DATA;

    vcc = 2.44 * (1 + (((unsigned short)(buf[0]<<8 | (buf[1] & 0xff) ))/512.0) );

    snprintf(buf, 15, "%2.2f", vcc);
    blobmsg_add_string(b, "VCC", buf);

    return UBUS_STATUS_OK;
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

    return UBUS_STATUS_OK;
}

static int catv_get_vpd(struct blob_buf *b)
{
    char buf[15];
    float vpd;
    float resp;
    int ret;

    resp = i2c_smbus_read_byte_data(pcatv->i2c_a0,84);
    resp = resp * 0.01;

    ret = i2c_smbus_read_i2c_block_data(pcatv->i2c_a2, 62, 2, (__u8*)buf);
    if(ret < 0)
        return UBUS_STATUS_NO_DATA;

    vpd = 2.44/1024 * (short)(buf[0]<<8 | (buf[1] & 0xff)) ;
    vpd = 10*log(vpd/(0.47*4.9)/resp)/log(10);
    snprintf(buf, 15, "%2.1f", vpd);
    blobmsg_add_string(b, "VPD", buf);

    return UBUS_STATUS_OK;
}

static int catv_get_vpd_method(struct ubus_context *ubus_ctx, struct ubus_object *obj,
                                     struct ubus_request_data *req, const char *method,
                                     struct blob_attr *msg)
{
    struct blob_buf b;

    memset(&b, 0, sizeof(b));
    blob_buf_init(&b, 0);

    catv_get_vpd(&b);

    ubus_send_reply(ubus_ctx, req, b.head);

    return UBUS_STATUS_OK;
}

static int catv_get_rf(struct blob_buf *b)
{
    char buf[15];
    float rf;
    int ret;

    ret = i2c_smbus_read_i2c_block_data(pcatv->i2c_a2, 64, 2, (__u8*)buf);
    if(ret < 0)
        return UBUS_STATUS_NO_DATA;

    rf = 2.44/1024.0 * (unsigned short)(buf[0]<<8 | (buf[1] & 0xff)) ;
    rf = (rf + 0.9148)/ 0.0582;
    snprintf(buf, 15, "%2.1f", rf);
    blobmsg_add_string(b, "RF", buf);

    return UBUS_STATUS_OK;
}

static int catv_get_rf_method(struct ubus_context *ubus_ctx, struct ubus_object *obj,
                                     struct ubus_request_data *req, const char *method,
                                     struct blob_attr *msg)
{
    struct blob_buf b;

    memset(&b, 0, sizeof(b));
    blob_buf_init(&b, 0);

    catv_get_rf(&b);

    ubus_send_reply(ubus_ctx, req, b.head);

    return UBUS_STATUS_OK;
}

static int catv_get_status(struct blob_buf *b)
{
    int status;

    status = i2c_smbus_read_byte_data(pcatv->i2c_a2,73);

    if(status < 0)
        return UBUS_STATUS_NO_DATA;

    if (status & 0x01)
        blobmsg_add_string(b, "Signal detect","ON" );
    else
        blobmsg_add_string(b, "Signal detect","OFF" );

    if (status & 0x02)
        blobmsg_add_string(b, "Interrupt","ON" );
    else
        blobmsg_add_string(b, "Interrupt","OFF" );

    if (status & 0x04)
        blobmsg_add_string(b, "RF enable","ON" );
    else
        blobmsg_add_string(b, "RF enable","OFF" );

    if (status & 0x08)
        blobmsg_add_string(b, "hold AGC","ON" );
    else
        blobmsg_add_string(b, "hold AGC","OFF" );

    if (status & 0x10)
        blobmsg_add_string(b, "47MHz ~ 1000MHz","ON" );
    else
        blobmsg_add_string(b, "47MHz ~ 1000MHz","OFF" );

    if (status & 0x20)
        blobmsg_add_string(b, "47MHz ~ 591MHz","ON" );
    else
        blobmsg_add_string(b, "47MHz ~ 591MHz","OFF" );

    if (status & 0x40)
        blobmsg_add_string(b, "47MHz ~ 431MHz","ON" );
    else
        blobmsg_add_string(b, "47MHz ~ 431MHz","OFF" );

    return UBUS_STATUS_OK;
}


#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#endif

enum {
    FILTER_FILTER
};

static const struct blobmsg_policy filter_policy[] = {
    [FILTER_FILTER] = { .name = "filter", .type = BLOBMSG_TYPE_STRING },
};

static void catv_filter(struct catv_handler *h,int num)
{
    int status;
    status = i2c_smbus_read_byte_data(h->i2c_a2,73);
    status = status &  ~(0x10 | 0x20 | 0x40);
    status = status | (1 <<(3 + num));
    i2c_smbus_write_byte_data(h->i2c_a2, 73, status);
}

static int catv_set_filter_method(struct ubus_context *ubus_ctx, struct ubus_object *obj,
                                     struct ubus_request_data *req, const char *method,
                                     struct blob_attr *msg)
{
    struct blob_buf b;
    struct blob_attr *tb[ARRAY_SIZE(filter_policy)];

    blobmsg_parse(filter_policy, ARRAY_SIZE(filter_policy) , tb, blob_data(msg), blob_len(msg));

    if (tb[FILTER_FILTER]) {
        int num;
        num = strtol(blobmsg_data(tb[FILTER_FILTER]),0,0);
        catv_filter(pcatv,num);
    }

    memset(&b, 0, sizeof(b));
    blob_buf_init(&b, 0);

    if(catv_get_status(&b))
        return UBUS_STATUS_NO_DATA;

    ubus_send_reply(ubus_ctx, req, b.head);

    return UBUS_STATUS_OK;
}

enum {
    STATUS_RF_ENABLE
};
static const struct blobmsg_policy enable_policy[] = {
    [STATUS_RF_ENABLE] = { .name = "enable", .type = BLOBMSG_TYPE_STRING },
};

static void catv_enable(struct catv_handler *h)
{
    int status;
    status = i2c_smbus_read_byte_data(h->i2c_a2,73);
    status = status | 0x4;
    i2c_smbus_write_byte_data(h->i2c_a2, 73, status);
}
static void catv_disable(struct catv_handler *h)
{
    int status;
    status = i2c_smbus_read_byte_data(h->i2c_a2,73);
    status = status & ~0x4;
    i2c_smbus_write_byte_data(h->i2c_a2, 73, status);
}

static int catv_set_enable_method(struct ubus_context *ubus_ctx, struct ubus_object *obj,
                                  struct ubus_request_data *req, const char *method,
                                  struct blob_attr *msg)
{
    struct blob_buf b;
    struct blob_attr *tb[ARRAY_SIZE(enable_policy)];

    blobmsg_parse(enable_policy, ARRAY_SIZE(enable_policy) , tb, blob_data(msg), blob_len(msg));

    if (tb[STATUS_RF_ENABLE]) {

        if (0 == strncasecmp("off", blobmsg_data(tb[STATUS_RF_ENABLE]), 3) ){
            catv_disable(pcatv);
        }
        if (0 == strncasecmp("on", blobmsg_data(tb[STATUS_RF_ENABLE]), 2) ){
            catv_enable(pcatv);
        }
    }

    memset(&b, 0, sizeof(b));
    blob_buf_init(&b, 0);

    catv_get_status(&b);

    ubus_send_reply(ubus_ctx, req, b.head);

    return UBUS_STATUS_OK;
}


static int catv_get_status_method(struct ubus_context *ubus_ctx, struct ubus_object *obj,
                                     struct ubus_request_data *req, const char *method,
                                     struct blob_attr *msg)
{
    struct blob_buf b;
    memset(&b, 0, sizeof(b));
    blob_buf_init(&b, 0);

    catv_get_status(&b);

    ubus_send_reply(ubus_ctx, req, b.head);

    return UBUS_STATUS_OK;
}

static int catv_get_alarm(struct blob_buf *b)
{
    int status;

    status = i2c_smbus_read_byte_data(pcatv->i2c_a2,74);

    if(status < 0)
        return UBUS_STATUS_NO_DATA;

    if (status & 0x01)
        blobmsg_add_string(b, "Alarm temp HI","ON" );
    else
        blobmsg_add_string(b, "Alarm temp HI","OFF" );

    if (status & 0x02)
        blobmsg_add_string(b, "Alarm temp LO","ON" );
    else
        blobmsg_add_string(b, "Alarm temp LO","OFF" );

    if (status & 0x04)
        blobmsg_add_string(b, "Warning temp HI","ON" );
    else
        blobmsg_add_string(b, "Warning temp HI","OFF" );

    if (status & 0x08)
        blobmsg_add_string(b, "Warning temp LO","ON" );
    else
        blobmsg_add_string(b, "Warning temp LO","OFF" );

    if (status & 0x10)
        blobmsg_add_string(b, "Alarm VCC HI","ON" );
    else
        blobmsg_add_string(b, "Alarm VCC HI","OFF" );

    if (status & 0x20)
        blobmsg_add_string(b, "Alarm VCC LO","ON" );
    else
        blobmsg_add_string(b, "Alarm VCC LO","OFF" );

    if (status & 0x40)
        blobmsg_add_string(b, "Warning VCC HI","ON" );
    else
        blobmsg_add_string(b, "Warning VCC HI","OFF" );

    if (status & 0x80)
        blobmsg_add_string(b, "Warning VCC LO","ON" );
    else
        blobmsg_add_string(b, "Warning VCC LO","OFF" );


    status = i2c_smbus_read_byte_data(pcatv->i2c_a2,75);

    if (status & 0x01)
        blobmsg_add_string(b, "Alarm VPD HI","ON" );
    else
        blobmsg_add_string(b, "Alarm VPD HI","OFF" );

    if (status & 0x02)
        blobmsg_add_string(b, "Alarm VPD LO","ON" );
    else
        blobmsg_add_string(b, "Alarm VPD LO","OFF" );

    if (status & 0x04)
        blobmsg_add_string(b, "Warning VPD HI","ON" );
    else
        blobmsg_add_string(b, "Warning VPD HI","OFF" );

    if (status & 0x08)
        blobmsg_add_string(b, "Warning VPD LO","ON" );
    else
        blobmsg_add_string(b, "Warning VPD LO","OFF" );

    if (status & 0x10)
        blobmsg_add_string(b, "Alarm RF HI","ON" );
    else
        blobmsg_add_string(b, "Alarm RF HI","OFF" );

    if (status & 0x20)
        blobmsg_add_string(b, "Alarm RF LO","ON" );
    else
        blobmsg_add_string(b, "Alarm RF LO","OFF" );

    if (status & 0x40)
        blobmsg_add_string(b, "Warning RF HI","ON" );
    else
        blobmsg_add_string(b, "Warning RF HI","OFF" );

    if (status & 0x80)
        blobmsg_add_string(b, "Warning RF LO","ON" );
    else
        blobmsg_add_string(b, "Warning RF LO","OFF" );

    return UBUS_STATUS_OK;

}

static int catv_get_alarm_method(struct ubus_context *ubus_ctx, struct ubus_object *obj,
                                     struct ubus_request_data *req, const char *method,
                                     struct blob_attr *msg)
{
    struct blob_buf b;

    memset(&b, 0, sizeof(b));
    blob_buf_init(&b, 0);

    catv_get_alarm(&b);

    ubus_send_reply(ubus_ctx, req, b.head);

    return UBUS_STATUS_OK;
}

static int catv_save_method(struct ubus_context *ubus_ctx, struct ubus_object *obj,
                            struct ubus_request_data *req, const char *method,
                            struct blob_attr *msg)
{
    struct blob_buf b;
    int status;

    memset(&b, 0, sizeof(b));
    blob_buf_init(&b, 0);

    status = i2c_smbus_read_byte_data(pcatv->i2c_a2,73);

    if(status < 0)
        return UBUS_STATUS_NO_DATA;

    if (status & 0x4)
        ucix_add_option(pcatv->ctx, "catv", "catv", "enable", "yes");
    else
        ucix_add_option(pcatv->ctx, "catv", "catv", "enable", "no");

    if (status & 0x10)
        ucix_add_option(pcatv->ctx, "catv", "catv", "filter", "1");
    else if (status & 0x20)
        ucix_add_option(pcatv->ctx, "catv", "catv", "filter", "2");
    else if (status & 0x40)
        ucix_add_option(pcatv->ctx, "catv", "catv", "filter", "3");


    ucix_save(pcatv->ctx);
    ucix_commit(pcatv->ctx, "catv");

    blobmsg_add_string(&b, "Saved", "/etc/config/catv");

    ubus_send_reply(ubus_ctx, req, b.head);

    return UBUS_STATUS_OK;
}


static int catv_get_all_method(struct ubus_context *ubus_ctx, struct ubus_object *obj,
                               struct ubus_request_data *req, const char *method,
                               struct blob_attr *msg)
{
    struct blob_buf b;
    int ret = 0;

    memset(&b, 0, sizeof(b));
    blob_buf_init (&b, 0);

    ret += catv_get_partnum(&b);
    ret += catv_get_vendor(&b);
    ret += catv_get_type(&b);
    ret += catv_get_vendor_partnum(&b);
    ret += catv_get_revision(&b);
    ret += catv_get_serial(&b);
    ret += catv_get_date(&b);
    ret += catv_get_interface(&b);
    ret += catv_get_bandwidth(&b);
    ret += catv_get_wavelength(&b);
    ret += catv_get_responsivity(&b);
    ret += catv_get_minoutput(&b);
    ret += catv_get_maxoptical(&b);
    ret += catv_get_minoptical(&b);
    ret += catv_get_templimit(&b);
    ret += catv_get_vcclimit(&b);
    ret += catv_get_vpdlimit(&b);
    ret += catv_get_rflimit(&b);
    ret += catv_get_firmware(&b);
    ret += catv_get_temp(&b);
    ret += catv_get_vcc(&b);
    ret += catv_get_vpd(&b);
    ret += catv_get_rf(&b);
    ret += catv_get_status(&b);
    ret += catv_get_alarm(&b);

    ubus_send_reply(ubus_ctx, req, b.head);

    return UBUS_STATUS_OK;
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
    { .name = "vpd", .handler = catv_get_vpd_method },
    { .name = "rf", .handler = catv_get_rf_method },
    { .name = "status", .handler = catv_get_status_method },
    { .name = "alarm", .handler = catv_get_alarm_method },
    { .name = "get-all",  .handler = catv_get_all_method },

    { .name = "set-filter",  .handler = catv_set_filter_method },
    { .name = "set-enable",  .handler = catv_set_enable_method },

    { .name = "save", .handler = catv_save_method },
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

static void catv_config_open(struct catv_handler *h)
{
    int loop = 0;
    /* open config file */
again:

    h->ctx = ucix_init_path("/etc/config", "catv");

    if (NULL == h->ctx) {
        int fd;

        syslog(LOG_INFO,"CATV config file not found /etc/config/catv\n");
        fd = open("/etc/config/catv",O_RDWR | O_CREAT | O_TRUNC);
        close(fd);
        if (loop++ < 10)
            goto again;
    }
}

static void catv_config_read(struct catv_handler *h)
{
    const char *s;

    /* set set filter */
again:
    s = ucix_get_option(h->ctx, "catv", "catv", "filter");
    if (s){
        int num = strtol(s,0,0);
        catv_filter(h,num);
    } else {
        /* no data recreate the file */
        ucix_add_section(h->ctx,"catv","catv", "service");
        ucix_add_option(h->ctx,"catv", "catv", "enable","no");
        ucix_add_option(h->ctx,"catv", "catv", "filter","3");
        ucix_save(h->ctx);
        ucix_commit(h->ctx,"catv");
        goto again;
    }

    /* set enable */
    s = ucix_get_option(h->ctx, "catv", "catv", "enable");
    if (s){
        if (strncasecmp("yes", s, 3) == 0)
            catv_enable(h);
        else  if (strncasecmp("no", s, 2) == 0)
            catv_disable(h);
    }

}

struct catv_handler * catv_init(struct uci_context *uci_ctx, char *i2c_bus,int a0_addr,int a2_addr)
{
    struct catv_handler *h;
    const char *p;

    h = malloc( sizeof(struct catv_handler) );

    if (!h)
        return NULL;

    p = ucix_get_option(uci_ctx, "hw", "board", "hardware");
    if (p == 0) {
        syslog(LOG_INFO, "%s: Missing Hardware identifier in configuration. I2C is not started\n",__func__);
        return NULL;
    }

    /* only run on EG300 hardware */
    if ( strcasecmp("EG300", p)){
        free(h);
        return NULL;
    }

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

    catv_config_open(h);
    catv_config_read(h);

//    dump_i2c(h->i2c_a0,0,255);
//    dump_i2c(h->i2c_a2,0,255);

    pcatv = h;
    return h;
}

void catv_destroy(struct catv_handler *h)
{
    close(h->i2c_a0);
    close(h->i2c_a2);
    free(h);
}

