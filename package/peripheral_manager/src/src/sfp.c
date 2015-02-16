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
#include "sfp.h"
#include "log.h"

//static struct blob_buf b;

struct i2c_sfp {
        const char *bus;
        int rom_addr;
        int ddm_addr;
        const char *name;
};

const struct i2c_sfp i2c_sfp_list[] = {
        { .bus = "/dev/i2c-0",
          .rom_addr = 0x50,
          .ddm_addr = 0x51,
          .name = "EG200"
        },
        { .bus = "/dev/i2c-1",
          .rom_addr = 0x50,
          .ddm_addr = 0x51,
          .name = "EG300"
        },
};

const struct i2c_sfp *i2c_sfp;

static int sfp_rom_fd = -1;
static int sfp_ddm_fd = -1;

static int sfp_rom_byte(unsigned addr)
{
        int ret;
        if (!i2c_sfp)
                return -1;

        if (sfp_rom_fd >= 0) {
                ret = i2c_smbus_read_byte_data(sfp_rom_fd, addr);
                if (ret >= 0)
                        return ret;
                /* Close and retry */
                close (sfp_rom_fd);
                goto open;
        }
        if (sfp_rom_fd < 0) {
        open:
                sfp_rom_fd = i2c_open_dev(i2c_sfp->bus, i2c_sfp->rom_addr, I2C_FUNC_SMBUS_READ_BYTE);
                if (sfp_rom_fd < 0)
                        return -1;
        }
        ret = i2c_smbus_read_byte_data(sfp_rom_fd, addr);
        if (ret < 0) {
                DBG(1,"%s: i2c_smbus_read_byte_data failed: addr %d", __func__, addr);
        }
        return ret;
};

static int sfp_rom_bytes(unsigned addr, char *p, size_t length)
{
        int byte = sfp_rom_byte(addr);
        unsigned int i;
        if (byte < 0)
                return 0;
        p[0] = byte;

        for (i = 1; i < length; i++) {
                byte = i2c_smbus_read_byte_data(sfp_rom_fd, addr + i);
                if (byte < 0)
                        return 0;
                p[i] = byte;
        }
        return 1;
}

static int sfp_rom_get_type(struct blob_buf *b)
{
        int byte = sfp_rom_byte (0);
        char buf[20];
        const char *value;

        if (byte < 0)
                return 0;

        switch (byte) {
        case 0:
                value = "unspecified";
                break;
        case 1:
                value = "GBIC";
                break;
        case 2:
                value = "soldered module/connector";
                break;
        case 3:
                value = "SFP";
                break;
        default:
                snprintf(buf, sizeof(buf), "%s %d",
                         byte < 0x80 ? "reserved" : "vendor specific",
                         byte);
                value = buf;
                break;
        }

        blobmsg_add_string(b, "type", value);
        return 1;
}

static int sfp_rom_get_type_method(struct ubus_context *ubus_ctx, struct ubus_object *obj,
                                   struct ubus_request_data *req, const char *method,
                                   struct blob_attr *msg)
{
	struct blob_buf b;
        memset(&b, 0, sizeof(struct blob_buf ));
	blob_buf_init (&b, 0);
	sfp_rom_get_type(&b);
	ubus_send_reply(ubus_ctx, req, b.head);
	return 0;
}
static int sfp_rom_get_connector(struct blob_buf *b)
{
        int byte = sfp_rom_byte (2);
        char buf[20];

        const char *value;

        if (byte < 0)
                return 0;

        switch (byte) {
        case 0:
                value = "Unspecified";
                break;
        case 1:
                value = "SC";
                break;
        case 2:
                value = "Fiber Channel style 1";
                break;
        case 3:
                value = "Fiber Channel style 2";
                break;
        case 4:
                value = "TNC/BNC";
                break;
        case 5:
                value = "Fiber Channel coaxial";
                break;
        case 6:
                value = "FiberJack";
                break;
        case 7:
                value = "LC";
                break;
        case 8:
                value = "MT-RJ";
                break;
        case 9:
                value = "MU";
                break;
        case 10:
                value = "SG";
                break;
        case 11:
                value = "Optical pigtail";
                break;
        case 32:
                value = "HSSDC II";
                break;
        case 33:
                value = "Copper pigtail";
                break;
        default:
                snprintf(buf, sizeof(buf), "%s %d",
                         byte < 0x80 ? "reserved" : "vendor specific",
                         byte);
                value = buf;
                break;
        }

        blobmsg_add_string(b, "connector", value);
        return 1;
}

static int sfp_rom_get_connector_method(struct ubus_context *ubus_ctx, struct ubus_object *obj,
                                        struct ubus_request_data *req, const char *method,
                                        struct blob_attr *msg)
{
	struct blob_buf b;
        memset(&b, 0, sizeof(struct blob_buf));
	blob_buf_init (&b, 0);
	sfp_rom_get_connector (&b);
	ubus_send_reply(ubus_ctx, req, b.head);
	return 0;
}

static int sfp_rom_get_ethernet(struct blob_buf *b)
{
        int byte = sfp_rom_byte (6);
        int i;
        char value[11];
        if (byte < 0)
                return 0;

        i = 0;
        if (byte & 8)
                value[i++] = 'T';
        if (byte & 4) {
                if (i)
                        value[i++] = ',';
                strcpy(value+i, "CX");
                i += 2;
        }
        if (byte & 2) {
                if (i)
                        value[i++] = ',';
                strcpy(value+i, "LX");
                i += 2;
        }
        if (byte & 1) {
                if (i)
                        value[i++] = ',';
                strcpy(value+i, "SX");
                i += 2;
        }
        if (!i)
                return 0;

        value[i] = '\0';
        blobmsg_add_string(b, "ethernet", value);
        return 1;
}

static int sfp_rom_get_ethernet_method(struct ubus_context *ubus_ctx, struct ubus_object *obj,
                                       struct ubus_request_data *req, const char *method,
                                       struct blob_attr *msg)
{
	struct blob_buf b;
        memset(&b, 0, sizeof(struct blob_buf));
	blob_buf_init (&b, 0);
	sfp_rom_get_ethernet(&b);
	ubus_send_reply(ubus_ctx, req, b.head);
	return 0;
}

static int sfp_rom_get_encoding(struct blob_buf *b)
{
        int byte = sfp_rom_byte (11);
        char buf[20];
        const char *value;
        if (byte < 0)
                return 0;

        switch (byte) {
        case 0:
                value = "Unspecified";
                break;
        case 1:
                value = "8B10B";
                break;
        case 2:
                value = "4B5B";
                break;
        case 3:
                value = "NRZ";
                break;
        case 4:
                value = "Manchester";
                break;
        default:
                snprintf(buf, sizeof(buf), "%s %d",
                         "reserved", byte);
                value = buf;
                break;
        }

        blobmsg_add_string(b, "encoding", value);
        return 1;
}

static int sfp_rom_get_encoding_method(struct ubus_context *ubus_ctx, struct ubus_object *obj,
                                       struct ubus_request_data *req, const char *method,
                                       struct blob_attr *msg)
{
	struct blob_buf b;
        memset(&b, 0, sizeof(struct blob_buf));
	blob_buf_init (&b, 0);
	sfp_rom_get_encoding(&b);
	ubus_send_reply(ubus_ctx, req, b.head);
	return 0;
}

static int sfp_rom_get_rate(struct blob_buf *b)
{
        int byte = sfp_rom_byte (12);
        int tol;
        if (byte < 0)
                return 0;

        /* Read byte is in units of 100 Mbit/s, scale to Mbit/s. */
        blobmsg_add_u32(b, "rate", 100*byte);
        tol = sfp_rom_byte (66);
        if (tol > 0)
                blobmsg_add_u32(b, "rate-max", (100 + tol)*byte);

        tol = sfp_rom_byte (67);
        if (tol > 0 && tol <= 100)
                blobmsg_add_u32(b, "rate-min", (100 - tol)*byte);

        return 1;
}

static int sfp_rom_get_rate_method(struct ubus_context *ubus_ctx, struct ubus_object *obj,
                                   struct ubus_request_data *req, const char *method,
                                   struct blob_attr *msg)
{
	struct blob_buf b;
        memset(&b, 0, sizeof(struct blob_buf));
	blob_buf_init (&b, 0);
	sfp_rom_get_rate(&b);
	ubus_send_reply(ubus_ctx, req, b.head);
	return 0;
}

static int sfp_rom_get_length(struct blob_buf *b)
{
        int sm_1000;
        int sm_100;
        int mm50;
        int mm62;
        int cu;

        if ( (sm_1000 = sfp_rom_byte (14)) < 0
             || (sm_100 = sfp_rom_byte (15)) < 0
             || (mm50 = sfp_rom_byte (16)) < 0
             || (mm62 = sfp_rom_byte (17)) < 0
             || (cu = sfp_rom_byte (18)) < 0)
                return 0;

        if (sm_1000 > 0)
                blobmsg_add_u32(b, "single-mode", sm_1000 * 1000);
        else if (sm_100 > 0)
                blobmsg_add_u32(b, "single-mode", sm_100 * 100);
        if (mm50 > 0)
                blobmsg_add_u32(b, "multi-mode-50", mm50 * 10);
        if (mm62 > 0)
                blobmsg_add_u32(b, "multi-mode-62.5", mm62 * 10);
        if (cu > 0)
                blobmsg_add_u32(b, "copper", cu);
        return 1;
}

static int sfp_rom_get_length_method(struct ubus_context *ubus_ctx, struct ubus_object *obj,
                                     struct ubus_request_data *req, const char *method,
                                     struct blob_attr *msg)
{
	struct blob_buf b;
        memset(&b, 0, sizeof(struct blob_buf));
	blob_buf_init (&b, 0);
	sfp_rom_get_length(&b);
	ubus_send_reply(ubus_ctx, req, b.head);
	return 0;
}

static int sfp_rom_get_vendor(struct blob_buf *b)
{
        char buf[17];
        int i;
        if (!sfp_rom_bytes(20, buf, 16))
                return 0;

        for (i = 16; i > 0 && buf[i-1] == ' '; i--)
                ;
        buf[i] = '\0';

        blobmsg_add_string(b, "vendor", buf);
        return 1;
}

static int sfp_rom_get_vendor_method(struct ubus_context *ubus_ctx, struct ubus_object *obj,
                                     struct ubus_request_data *req, const char *method,
                                     struct blob_attr *msg)
{
	struct blob_buf b;
        memset(&b, 0, sizeof(struct blob_buf));
	blob_buf_init (&b, 0);
	sfp_rom_get_vendor(&b);
	ubus_send_reply(ubus_ctx, req, b.head);
	return 0;
}

static int sfp_rom_get_oui(struct blob_buf *b)
{
        char buf[3];
        char value[9];
        if (!sfp_rom_bytes(37, buf, 3))
                return 0;

        snprintf(value, sizeof(value), "%02x:%02x:%02x",
                 (unsigned char) buf[0], (unsigned char) buf[1], (unsigned char) buf[2]);
        blobmsg_add_string(b, "oui", value);
        return 1;
}

static int sfp_rom_get_oui_method(struct ubus_context *ubus_ctx, struct ubus_object *obj,
                                  struct ubus_request_data *req, const char *method,
                                  struct blob_attr *msg)
{
	struct blob_buf b;
        memset(&b, 0, sizeof(struct blob_buf));
	blob_buf_init (&b, 0);
	sfp_rom_get_oui(&b);
	ubus_send_reply(ubus_ctx, req, b.head);
	return 0;
}

static int sfp_rom_get_pn(struct blob_buf *b)
{
        char buf[17];
        int i;
        if (!sfp_rom_bytes(40, buf, 16))
                return 0;

        for (i = 16; i > 0 && buf[i-1] == ' '; i--)
                ;
        buf[i] = '\0';

        blobmsg_add_string(b, "pn", buf);
        return 1;
}

static int sfp_rom_get_pn_method(struct ubus_context *ubus_ctx, struct ubus_object *obj,
                                 struct ubus_request_data *req, const char *method,
                                 struct blob_attr *msg)
{
	struct blob_buf b;
        memset(&b, 0, sizeof(struct blob_buf));
	blob_buf_init (&b, 0);
	sfp_rom_get_pn(&b);
	ubus_send_reply(ubus_ctx, req, b.head);
	return 0;
}

static int sfp_rom_get_rev(struct blob_buf *b)
{
        char buf[5];
        int i;
        if (!sfp_rom_bytes(56, buf, 4))
                return 0;

        for (i = 4; i > 0 && buf[i-1] == ' '; i--)
                ;
        buf[i] = '\0';

        blobmsg_add_string(b, "rev", buf);
        return 1;
}

static int sfp_rom_get_rev_method(struct ubus_context *ubus_ctx, struct ubus_object *obj,
                                  struct ubus_request_data *req, const char *method,
                                  struct blob_attr *msg)
{
	struct blob_buf b;
        memset(&b, 0, sizeof(struct blob_buf));
	blob_buf_init (&b, 0);
	sfp_rom_get_rev(&b);
	ubus_send_reply(ubus_ctx, req, b.head);
	return 0;
}

static int sfp_rom_get_sn(struct blob_buf *b)
{
        char buf[17];
        int i;
        if (!sfp_rom_bytes(68, buf, 16))
                return 0;

        for (i = 16; i > 0 && buf[i-1] == ' '; i--)
                ;
        buf[i] = '\0';

        blobmsg_add_string(b, "sn", buf);
        return 1;
}
static int sfp_rom_get_sn_method(struct ubus_context *ubus_ctx, struct ubus_object *obj,
                                 struct ubus_request_data *req, const char *method,
                                 struct blob_attr *msg)
{
	struct blob_buf b;
        memset(&b, 0, sizeof(struct blob_buf));
	blob_buf_init (&b, 0);
	sfp_rom_get_sn(&b);
	ubus_send_reply(ubus_ctx, req, b.head);
	return 0;
}

static int sfp_rom_get_date(struct blob_buf *b)
{
        char buf[8];
        char value[14];
        int i;
        if (!sfp_rom_bytes(84, buf, 8))
                return 0;

        value[0] = '2';
        value[1] = '0';
        value[2] = buf[0];
        value[3] = buf[1];
        value[4] = '-';
        value[5] = buf[2];
        value[6] = buf[3];
        value[7] = '-';
        value[8] = buf[4];
        value[9] = buf[5];
        for (i = 8; i > 6 && buf[i-1] == ' '; i--)
                ;
        memset(value+10, 0, 4);
        if (i > 6) {
                value[10] = ' ';
                value[11] = buf[6];
                if (i > 7)
                        value[12] = buf[7];
        }

        blobmsg_add_string(b, "date", value);
        return 1;
}

static int sfp_rom_get_date_method(struct ubus_context *ubus_ctx, struct ubus_object *obj,
                                   struct ubus_request_data *req, const char *method,
                                   struct blob_attr *msg)
{
	struct blob_buf b;
        memset(&b, 0, sizeof(struct blob_buf));
	blob_buf_init (&b, 0);
	sfp_rom_get_date(&b);
	ubus_send_reply(ubus_ctx, req, b.head);
	return 0;
}

static int sfp_rom_get_ddm(struct blob_buf *b)
{
        int byte = sfp_rom_byte(94);
        char buf[20];
        const char *value;

        if (byte < 0)
                return 0;

        switch (byte) {
        case 0:
                value = "none";
                break;
        case 1:
                value = "9.3";
                break;
        case 2:
                value = "9.5";
                break;
        case 3:
                value = "10.2";
                break;
        case 4:
                value = "10.4";
                break;
        case 5:
                value = "11.0";
                break;
        case 6:
                value = "11.3";
                break;
        default:
                snprintf(buf, sizeof(buf), "%s %d",
                         "reserved", byte);
                value = buf;
        }
        blobmsg_add_string(b, "ddm", value);
        return 1;
};

static int sfp_rom_get_ddm_method(struct ubus_context *ubus_ctx, struct ubus_object *obj,
                                  struct ubus_request_data *req, const char *method,
                                  struct blob_attr *msg)
{
	struct blob_buf b;
        memset(&b, 0, sizeof(struct blob_buf));
	blob_buf_init (&b, 0);
	sfp_rom_get_ddm(&b);
	ubus_send_reply(ubus_ctx, req, b.head);
	return 0;
}

static int sfp_rom_get_all_method(struct ubus_context *ubus_ctx, struct ubus_object *obj,
                                  struct ubus_request_data *req, const char *method,
                                  struct blob_attr *msg)
{
	struct blob_buf b;
        memset(&b, 0, sizeof(struct blob_buf));
	blob_buf_init (&b, 0);
	sfp_rom_get_connector(&b);
	sfp_rom_get_ethernet(&b);
	sfp_rom_get_encoding(&b);
	sfp_rom_get_rate(&b);
	sfp_rom_get_length(&b);
	sfp_rom_get_vendor(&b);
	sfp_rom_get_oui(&b);
	sfp_rom_get_pn(&b);
	sfp_rom_get_rev(&b);
	sfp_rom_get_sn(&b);
	sfp_rom_get_date(&b);
	sfp_rom_get_ddm(&b);
	ubus_send_reply(ubus_ctx, req, b.head);
	return 0;
}

struct sfp_ddm {
        /* For checking when to reread calibration data. */
        time_t timestamp;
        unsigned type; /* From address 92 */
        unsigned version; /* From address 94 */
        /* Calibration constants, read from addresses 56-91 if bit 4 is
           set in ddm_type */
        float rx_pwr[5];
        float tx_i_slope;
        float tx_i_offset;
        float tx_pwr_slope;
        float tx_pwr_offset;
        float t_slope;
        float t_offset;
        float v_slope;
        float v_offset;
};
static struct sfp_ddm sfp_ddm;

static int sfp_ddm_read_float(float *x, unsigned addr)
{
        uint8_t buf[4];
        int32_t m;
        int e;
        unsigned i;

        if (!i2c_sfp || sfp_ddm_fd < 0)
                return 0;
        /* Used only for constant data, so byte accesses should be ok. And
           there should only be regular normalized numbers (or zero).
        */
        for (i = 0; i < 4; i++) {
                int byte = i2c_smbus_read_byte_data(sfp_ddm_fd, addr + i);
                if (byte < 0)
                        return 0;
                buf[i] = byte;
        }
        e = (buf[0] & 0x7f) << 1 | buf[1] >> 7;
        m = (((int32_t) buf[1] & 0x7f) << 16) | ((int32_t) buf[2] << 8) | buf[3];
        if (e == 0 && m == 0) {
                *x = 0.0;
                return 1;
        }
        if (e == 0 || e == 0xff)
                /* NaN or infinity */
                return 0;
        m |= (1U << 23);
        if (buf[0] & 0x80)
                m = -m;
        *x = ldexpf ((float) m, e - (127 + 23));

        return 1;
}

/* Plain i2c_smbus_read_word_data use little-endian byteorder. We have
   msb first, so swap it. The swap should be a nop for the failure
   case w == -1. */
static int i2c_smbus_read_word_swapped(int fd, unsigned addr)
{
        int w = i2c_smbus_read_word_data(fd, addr);
        return (w >> 8) | ((w & 0xff) << 8);

}
static int sfp_ddm_read_fp(float *x, unsigned addr)
{
        int w;
        if (!i2c_sfp || sfp_ddm_fd < 0)
                return 0;
        w = i2c_smbus_read_word_swapped(sfp_ddm_fd, addr);
        if (w < 0)
                return 0;
        *x = (float) w / 0x100;
        return 1;
}

static int sfp_ddm_read_si(float *x, unsigned addr)
{
        int w;

        if (!i2c_sfp || sfp_ddm_fd < 0)
                return 0;
        w = i2c_smbus_read_word_swapped(sfp_ddm_fd, addr);
        if (w < 0)
                return 0;
        *x = (float) (int16_t) w;
        return 1;
}

static int sfp_ddm_read_ui(float *x, unsigned addr)
{
        int w;

        if (!i2c_sfp || sfp_ddm_fd < 0)
                return 0;
        w = i2c_smbus_read_word_swapped(sfp_ddm_fd, addr);
        if (w < 0)
                return 0;
        *x = (float) w;
        return 1;
}

static int ddm_prepare(void)
{
        int byte;
        int reread;
        time_t now = time(NULL);


        byte = sfp_rom_byte(92);
        if (byte < 0) {
        fail:
                if (sfp_ddm_fd >= 0)
                        close(sfp_ddm_fd);
                sfp_ddm_fd = -1;
                return 0;
        }
        if ( (byte & 0xc0) != 0x40)
                goto fail;

        if (byte & 4) {
                syslog(LOG_INFO, "sfp: ddm requires address change, not implemented.");
                goto fail;
        }
        sfp_ddm.type = byte;
        byte = sfp_rom_byte(94);
        if (byte <= 0)
                goto fail;
        sfp_ddm.version = byte;
        if (sfp_ddm_fd < 0) {
                sfp_ddm_fd = i2c_open_dev(i2c_sfp->bus, i2c_sfp->ddm_addr,
                                          I2C_FUNC_SMBUS_READ_BYTE | I2C_FUNC_SMBUS_READ_WORD_DATA);
                if (sfp_ddm_fd < 0)
                        return 0;

                reread = 1;
        }
        else if (sfp_ddm.type & 0x10)
                /* External calibration */

                /* We could check vendor, sn, etc, to try to figure out if the
                   SFP has been replaced, but it's less work to just reread
                   the calibration data. */
                reread = (now > sfp_ddm.timestamp + 120 || sfp_ddm.timestamp > now);
        else
                reread = 0;

        if (reread) {
                if (sfp_ddm.type & 0x10) {
                        unsigned i;
                        for (i = 0; i < 5; i++)
                                if (!sfp_ddm_read_float(&sfp_ddm.rx_pwr[4-i], 56+4*i))
                                        goto fail;
                        if (! (sfp_ddm_read_fp(&sfp_ddm.tx_i_slope, 76)
                               && sfp_ddm_read_si(&sfp_ddm.tx_i_offset, 78)
                               && sfp_ddm_read_fp(&sfp_ddm.tx_pwr_slope, 80)
                               && sfp_ddm_read_si(&sfp_ddm.tx_pwr_offset, 82)
                               && sfp_ddm_read_fp(&sfp_ddm.t_slope, 84)
                               && sfp_ddm_read_si(&sfp_ddm.t_offset, 86)
                               && sfp_ddm_read_fp(&sfp_ddm.v_slope, 88)
                               && sfp_ddm_read_si(&sfp_ddm.v_offset, 89))) {
                                syslog(LOG_INFO, "sfp: Reading ddm calibration data failed.");
                                goto fail;
                        }
                        DBG(1,"Read ddm calibration data:\n"
                            "rx_pwr: %g %g %g %g %g\n"
                            "tx_i: %g %g\n"
                            "tx_pwr: %g %g\n"
                            "T: %g %g\n"
                            "V: %g %g\n",
                            sfp_ddm.rx_pwr[0], sfp_ddm.rx_pwr[1],
                            sfp_ddm.rx_pwr[2], sfp_ddm.rx_pwr[3],
                            sfp_ddm.rx_pwr[4],
                            sfp_ddm.tx_i_slope, sfp_ddm.tx_i_offset,
                            sfp_ddm.tx_pwr_slope, sfp_ddm.tx_pwr_offset,
                            sfp_ddm.t_slope, sfp_ddm.t_offset,
                            sfp_ddm.v_slope, sfp_ddm.v_offset);

                }
                else {
                        sfp_ddm.rx_pwr[0] = sfp_ddm.rx_pwr[2]
                                = sfp_ddm.rx_pwr[3] = sfp_ddm.rx_pwr[4] = 0.0;
                        sfp_ddm.rx_pwr[1] = 1.0;

                        sfp_ddm.tx_i_slope = sfp_ddm.tx_pwr_slope
                                = sfp_ddm.t_slope = sfp_ddm.v_slope = 1.0;
                        sfp_ddm.tx_i_offset = sfp_ddm.tx_pwr_offset
                                = sfp_ddm.t_offset = sfp_ddm.v_offset = 0.0;
                }
                sfp_ddm.timestamp = now;
        }
        return 1;
};

static int sfp_ddm_get_temperature(struct blob_buf *b, int raw)
{
        float x;
        char buf[15];

        if (!ddm_prepare())
                return 0;

        if (!sfp_ddm_read_si(&x, 96))
                return 0;

        x = sfp_ddm.t_slope * x + sfp_ddm.t_offset;
        if (raw) {
                blobmsg_add_u32(b, "raw", (uint32_t) (x+0.5));
                blobmsg_add_string(b, "unit", "1/256 °C");
        }

        snprintf(buf, sizeof(buf), "%.2f °C", x * (1.0/256));
        blobmsg_add_string(b, "temperature", buf);
        return 1;
}

static int sfp_ddm_get_temperature_method(struct ubus_context *ubus_ctx, struct ubus_object *obj,
                                          struct ubus_request_data *req, const char *method,
                                          struct blob_attr *msg)
{
	struct blob_buf b;
        memset(&b, 0, sizeof(struct blob_buf));
	blob_buf_init (&b, 0);
	sfp_ddm_get_temperature(&b, 1);
	ubus_send_reply(ubus_ctx, req, b.head);
	return 0;
}

static int sfp_ddm_get_voltage(struct blob_buf *b, int raw)
{
        float x;
        char buf[10];
        if (!ddm_prepare())
                return 0;

        if (!sfp_ddm_read_ui(&x, 98))
                return 0;

        x = sfp_ddm.v_slope * x + sfp_ddm.v_offset;
        if (raw) {
                blobmsg_add_u32(b, "raw", (uint32_t) (x+0.5));
                blobmsg_add_string(b, "unit", "100uV");
        }
        snprintf(buf, sizeof(buf), "%.4f V", x * (1.0/10000));
        blobmsg_add_string(b, "voltage", buf);
        return 1;
}

static int sfp_ddm_get_voltage_method(struct ubus_context *ubus_ctx, struct ubus_object *obj,
                                      struct ubus_request_data *req, const char *method,
                                      struct blob_attr *msg)
{
	struct blob_buf b;
        memset(&b, 0, sizeof(struct blob_buf));
	blob_buf_init (&b, 0);
	sfp_ddm_get_voltage(&b, 1);
	ubus_send_reply(ubus_ctx, req, b.head);
	return 0;
}

static int sfp_ddm_get_current(struct blob_buf *b, int raw)
{
        float x;
        char buf[10];
        if (!ddm_prepare())
                return 0;

        if (!sfp_ddm_read_ui(&x, 100))
                return 0;

        x = sfp_ddm.tx_i_slope * x + sfp_ddm.tx_i_offset;
        if (raw) {
                blobmsg_add_u32(b, "raw", (uint32_t) (x+0.5));
                blobmsg_add_string(b, "unit", "2 uA");
        }
        snprintf(buf, sizeof(buf), "%.3f mA", x * (1.0/500));
        blobmsg_add_string(b, "current", buf);
        return 1;
}

static int sfp_ddm_get_current_method(struct ubus_context *ubus_ctx, struct ubus_object *obj,
                                      struct ubus_request_data *req, const char *method,
                                      struct blob_attr *msg)
{
	struct blob_buf b;
        memset(&b, 0, sizeof(struct blob_buf));
	blob_buf_init (&b, 0);
	sfp_ddm_get_current(&b, 1);
	ubus_send_reply(ubus_ctx, req, b.head);
	return 0;
}

static int sfp_ddm_get_tx_pwr(struct blob_buf *b, int raw)
{
        float x;
        char buf[10];
        if (!ddm_prepare())
                return 0;

        if (!sfp_ddm_read_ui(&x, 102))
                return 0;

        x = sfp_ddm.tx_pwr_slope * x + sfp_ddm.tx_pwr_offset;
        if (raw) {
                blobmsg_add_u32(b, "raw", (uint32_t) (x+0.5));
                blobmsg_add_string(b, "unit", "0.1uW");
        }
        snprintf(buf, sizeof(buf), "%.4f mW", x * (1.0/10000));
        blobmsg_add_string(b, "tx-pwr", buf);

        return 1;
}

static int sfp_ddm_get_tx_pwr_method(struct ubus_context *ubus_ctx, struct ubus_object *obj,
                                     struct ubus_request_data *req, const char *method,
                                     struct blob_attr *msg)
{
	struct blob_buf b;
        memset(&b, 0, sizeof(struct blob_buf));
	blob_buf_init (&b, 0);
	sfp_ddm_get_tx_pwr(&b, 1);
	ubus_send_reply(ubus_ctx, req, b.head);
	return 0;
}


static int sfp_ddm_get_rx_pwr(struct blob_buf *b, int raw)
{
        unsigned i;
        float x;
        char buf[10];

        if (!ddm_prepare())
                return 0;

        for (x = sfp_ddm.rx_pwr[0], i = 1; i < 5; i++) {
                float v;
                /* NOTE: There's only a single word to read. It's unclear how
                   to get several values. However, typically, rx_pwr[2,3,4]
                   are zero. */
                if (sfp_ddm.rx_pwr[i] != 0.0) {
                        if (!sfp_ddm_read_ui(&v, 104))
                                return 0;
                        x += v*sfp_ddm.rx_pwr[i];
                }
        }
        if (raw) {
                blobmsg_add_u32(b, "raw", (uint32_t) (x+0.5));
                blobmsg_add_string(b, "unit", "0.1uW");
        }

        snprintf(buf, sizeof(buf), "%.4f mW", x * (1.0/10000));
        blobmsg_add_string(b, "rx-pwr", buf);
        blobmsg_add_string(b, "rx-pwr-type",
                           (sfp_ddm.type & 8) ? "average" : "OMA");
        return 1;
}

static int sfp_ddm_get_rx_pwr_method(struct ubus_context *ubus_ctx, struct ubus_object *obj,
                                     struct ubus_request_data *req, const char *method,
                                     struct blob_attr *msg)
{
	struct blob_buf b;
        memset(&b, 0, sizeof(struct blob_buf));
	blob_buf_init (&b, 0);
	sfp_ddm_get_rx_pwr(&b, 1);
	ubus_send_reply(ubus_ctx, req, b.head);
	return 0;
}
static int sfp_ddm_get_all_method(struct ubus_context *ubus_ctx, struct ubus_object *obj,
                                  struct ubus_request_data *req, const char *method,
                                  struct blob_attr *msg)
{
	struct blob_buf b;

        memset(&b, 0, sizeof(struct blob_buf));
	blob_buf_init (&b, 0);

	sfp_ddm_get_temperature(&b, 0);
	sfp_ddm_get_voltage(&b, 0);
	sfp_ddm_get_current(&b, 0);
	sfp_ddm_get_tx_pwr(&b, 0);
	sfp_ddm_get_rx_pwr(&b, 0);

	ubus_send_reply(ubus_ctx, req, b.head);
	return 0;
}

static const struct ubus_method sfp_rom_methods[] = {
        { .name = "get-type", .handler = sfp_rom_get_type_method },
        { .name = "get-connector", .handler = sfp_rom_get_connector_method },
        { .name = "get-ethernet", .handler = sfp_rom_get_ethernet_method },
        { .name = "get-encoding", .handler = sfp_rom_get_encoding_method },
        { .name = "get-rate", .handler = sfp_rom_get_rate_method },
        { .name = "get-length", .handler = sfp_rom_get_length_method },
        { .name = "get-vendor", .handler = sfp_rom_get_vendor_method },
        { .name = "get-oui", .handler = sfp_rom_get_oui_method },
        { .name = "get-pn", .handler = sfp_rom_get_pn_method },
        { .name = "get-rev", .handler = sfp_rom_get_rev_method },
        { .name = "get-sn", .handler = sfp_rom_get_sn_method },
        { .name = "get-date", .handler = sfp_rom_get_date_method },
        { .name = "get-ddm", .handler = sfp_rom_get_ddm_method },
        { .name = "get-all", .handler = sfp_rom_get_all_method },
};

static struct ubus_object_type sfp_rom_type =
        UBUS_OBJECT_TYPE("sfp.rom", sfp_rom_methods);

static const struct ubus_method sfp_ddm_methods[] = {
        { .name = "get-rx-pwr", .handler = sfp_ddm_get_rx_pwr_method },
        { .name = "get-tx-pwr", .handler = sfp_ddm_get_tx_pwr_method },
        { .name = "get-temperature", .handler = sfp_ddm_get_temperature_method },
        { .name = "get-current", .handler = sfp_ddm_get_current_method },
        { .name = "get-voltage", .handler = sfp_ddm_get_voltage_method },
        { .name = "get-all", .handler = sfp_ddm_get_all_method },
};

static struct ubus_object_type sfp_ddm_type =
        UBUS_OBJECT_TYPE("sfp.ddm", sfp_ddm_methods);

static struct ubus_object sfp_objects[] = {
        { .name = "sfp.rom", .type = &sfp_rom_type,
          .methods = sfp_rom_methods, ARRAY_SIZE(sfp_rom_methods) },
        { .name = "sfp.ddm", .type = &sfp_ddm_type,
          .methods = sfp_ddm_methods, ARRAY_SIZE(sfp_ddm_methods) },
};

struct sfp_handler * sfp_init( struct uci_context *uci_ctx)
{
        const char *p;
        unsigned int i;

        p = ucix_get_option(uci_ctx, "hw", "board", "hardware");
        if (p == 0){
                syslog(LOG_INFO, "%s: Missing Hardware identifier in configuration. I2C is not started",__func__);
                return 0;
        }

        /* Here we match the hardware name to a init table, and get the
           i2c chip address */
        i2c_sfp = NULL;
        for (i = 0; i < sizeof(i2c_sfp_list) / sizeof(i2c_sfp_list[0]); i++) {
                DBG(1,"I2C hardware platform %s tested.", i2c_sfp_list[i].name);
                if (!strcmp(i2c_sfp_list[i].name, p)) {
                        DBG(1,"I2C hardware platform %s found.", p);
                        i2c_sfp = &i2c_sfp_list[i];
                        break;
                }
        }
        if (!i2c_sfp) {
                DBG(1,"No sfp I2C hardware found: %s.", p);
                return 0;
        }

        /* just return something not NULL */
        return (struct sfp_handler *)4;
}

int sfp_ubus_populate( struct sfp_handler *h, struct ubus_context *ubus_ctx)
{
        unsigned int i;
        int ret;

        for (i = 0; i < ARRAY_SIZE(sfp_objects); i++) {
                ret = ubus_add_object (ubus_ctx, &sfp_objects[i]);
                if (ret)
                        DBG(1,"Failed to add sfp object: %s", ubus_strerror(ret));
        }
        return 0;
}
