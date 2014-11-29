/*
 * brcm_fw_tool Simple tool to handle Broadcom nand flash images
 *
 * Copyright (C) 2012 Benjamin Larsson <benjamin@southpole.se>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License v2
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 *
 * The code is based on the linux-mtd examples and the openwrt mtd package
 * Copyright (C) 2005      Waldemar Brodkorb <wbx@dass-it.de>,
 * Copyright (C) 2005-2009 Felix Fietkau <nbd@openwrt.org>  
 */

#include <limits.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <net/if.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <errno.h>
#include <error.h>
#include <time.h>
#include <string.h>
#include <sys/mman.h>
#include <err.h>
#include "brcm_fw_tool.h"
#include "crc32.h"
#include "jffs2.h"
#include <board.h>
#include "bcmnet.h"

#define MAP_SIZE (4096)
#define MAP_MASK 0xFFF
void          *mapped_base;

#if __BYTE_ORDER == __BIG_ENDIAN
#define STORE32_LE(X)           ((((X) & 0x000000FF) << 24) | (((X) & 0x0000FF00) << 8) | (((X) & 0x00FF0000) >> 8) | (((X) & 0xFF000000) >> 24))
#elif __BYTE_ORDER == __LITTLE_ENDIAN
#define STORE32_LE(X)           (X)
#else
#error unkown endianness!
#endif

#define IMAGETAG_CRC_START      0xFFFFFFFF

int verbose                  = 0;
long start_offset	     = 0;
long fsimg_size              = 0;
#define BLOCKSIZE 131072
#define VERSION_NAME "cferam.000"
#define READ_BUF 1024

#define WFI_VERSION             0x00005732

#define WFI_NOR_FLASH           1
#define WFI_NAND16_FLASH        2
#define WFI_NAND128_FLASH       3
#define WFI_NANDFS128_IMAGE     4
#define WFI_NANDFS16_IMAGE      5
#define WFI_NANDCFE_IMAGE       6
#define WFI_NORCFE_IMAGE        7

typedef struct _WFI_TAG
{
    uint32_t wfiCrc;
    uint32_t wfiVersion;
    uint32_t wfiChipId;
    uint32_t wfiFlashType;
    uint32_t wfiReserved;
} WFI_TAG;


#define IMAGE_LEN 10                   /* Length of Length Field */
#define ADDRESS_LEN 12                 /* Length of Address field */
#define TAGID_LEN  6                   /* Length of tag ID */
#define TAGINFO_LEN 20                 /* Length of vendor information field in tag */
#define TAGVER_LEN 4                   /* Length of Tag Version */
#define TAGLAYOUT_LEN 4                /* Length of FlashLayoutVer */

#define OFFSET_OEM_CUSTOMER (sizeof(WFI_TAG) + 126*1024)
#define OFFSET_MODEL_NAME (sizeof(WFI_TAG) + 127*1024)

typedef struct _bcm_tag_bccfe {
	unsigned char tagVersion[TAGVER_LEN];           // 0-3: Version of the image tag
	unsigned char sig_1[20];                        // 4-23: Company Line 1
	unsigned char sig_2[14];                        // 24-37: Company Line 2
	unsigned char chipid[6];                        // 38-43: Chip this image is for
	unsigned char boardid[16];                      // 44-59: Board name
	unsigned char big_endian[2];                    // 60-61: Map endianness -- 1 BE 0 LE
	char totalLength[IMAGE_LEN];           // 62-71: Total length of image
	unsigned char cfeAddress[ADDRESS_LEN];          // 72-83: Address in memory of CFE
	unsigned char cfeLength[IMAGE_LEN];             // 84-93: Size of CFE
	unsigned char rootAddress[ADDRESS_LEN];         // 94-105: Address in memory of rootfs
	unsigned char rootLength[IMAGE_LEN];            // 106-115: Size of rootfs
	unsigned char kernelAddress[ADDRESS_LEN];       // 116-127: Address in memory of kernel
	unsigned char kernelLength[IMAGE_LEN];          // 128-137: Size of kernel
	unsigned char dualImage[2];                     // 138-139: Unused at present
	unsigned char inactiveFlag[2];                  // 140-141: Unused at present
	unsigned char information1[TAGINFO_LEN];        // 142-161: Unused at present
	unsigned char tagId[TAGID_LEN];                 // 162-167: Identifies which type of tag this is, currently two-letter company code, and then three digits for version of broadcom code in which this tag was first introduced
	char tagIdCRC[4];                      // 168-171: CRC32 of tagId
	unsigned char reserved1[44];                    // 172-215: Reserved area not in use
	char imageCRC[4];                      // 216-219: CRC32 of images
	unsigned char reserved2[16];                    // 220-235: Unused at present
	char headerCRC[4];                     // 236-239: CRC32 of header excluding tagVersion
	unsigned char reserved3[16];                    // 240-255: Unused at present
} bcm_tag_bccfe;

/* IOCTL file descriptor */
int fd = 0;

static int board_ioctl(int ioctl_id, int action, int hex, char* string_buf, int string_buf_len, int offset) {
    BOARD_IOCTL_PARMS IoctlParms;
    IoctlParms.string = string_buf;
    IoctlParms.strLen = string_buf_len;
    IoctlParms.offset = offset;
    IoctlParms.action = action;
    IoctlParms.buf    = "";
    if ( ioctl(fd, ioctl_id, &IoctlParms) < 0 ) {
        fprintf(stderr, "ioctl: %d failed\n", ioctl_id);
        exit(1);
    }

    if (verbose) 
        fprintf(stderr, "IoctlParms.result = %d\n", IoctlParms.result);

    if (string_buf) {
        if (verbose) 
            printf("|%d%s|\n", IoctlParms.string[0],IoctlParms.string);
        return 0;
    }

    if (hex)
        printf("%x\n", IoctlParms.result);
    else
        printf("%d\n", IoctlParms.result);

    return 0;
}


static int write_flash_image(const char *in_file, int cfe, int fs) {

    //BOARD_IOCTL_PARMS IoctlParms;
    char* flash_buffer;
    int infile_fd;
    struct stat file_stat;
    int fw_size;

    fd = open("/dev/brcmboard", O_RDWR);
    if ( fd == -1 ) {
        fprintf(stderr, "failed to open: /dev/brcmboard\n");
        exit(1);
    }
    if(stat(in_file, &file_stat) < 0) {
        fprintf(stderr, "getting filesize of %s failed\n", in_file);
        exit(1);
    }
    fw_size = file_stat.st_size -20; //20=wfi header size

    flash_buffer = malloc(file_stat.st_size);
    if (!flash_buffer)  {
        fprintf(stderr, "malloc of size: %d failed\n", (int)file_stat.st_size);
        exit(0);
    }
    infile_fd = open(in_file, O_RDONLY);
    if ( infile_fd == -1 ) {
        fprintf(stderr, "failed to open: %s\n", in_file);
        exit(1);
    }

    if (read(infile_fd, flash_buffer, file_stat.st_size) < 0) {
        fprintf(stderr, "reading data from file %s failed\n", in_file);
        exit(1);
    }

    if (verbose)
        printf("Writing %s%s with size %d\n ", cfe?"cfe":"",fs?"fs":"",(int)file_stat.st_size);

    if (cfe)        board_ioctl(BOARD_IOCTL_FLASH_WRITE, BCM_IMAGE_WHOLE, 0, flash_buffer, fw_size, 0/*0xB8000000*/);
    if (fs)         board_ioctl(BOARD_IOCTL_FLASH_WRITE, BCM_IMAGE_WHOLE, 0, flash_buffer, fw_size, 0xB8000000+128*1024);

    printf("Flash written ok\n");

    close(fd);
    close(infile_fd);
    return 0;
}

void dump(int reg)
{
    printf("%x -> %08x\n",reg,*(int*)(mapped_base+reg));
}

void wr(int addr,int reg)
{
    *(int*)(mapped_base+addr) = reg;
}

int rr(int addr)
{
    return *(int*)(mapped_base+addr);
}


static int get_info(int memdump, int chip_id, int flash_size, int chip_rev, int cfe_version, int wan_interfaces, int status, int boot_mode, int boot_mode_id) {
    char ioctl_buf[64]={0};
    fd = open("/dev/brcmboard", O_RDWR);
    if ( fd == -1 ) {
        fprintf(stderr, "failed to open: /dev/brcmboard\n");
        exit(1);
    }

    if (chip_id)       board_ioctl(BOARD_IOCTL_GET_CHIP_ID ,          0, 1, NULL, 0, 0);
    if (flash_size)    board_ioctl(BOARD_IOCTL_FLASH_READ  , FLASH_SIZE, 0, NULL, 0, 0);
    if (chip_rev)      board_ioctl(BOARD_IOCTL_GET_CHIP_REV,          0, 1, NULL, 0, 0);
    //if (status)        board_ioctl(BOARD_IOCTL_GET_STATUS,            0, 1, NULL, 0, 0);
    //if (boot_mode)     board_ioctl(BOARD_IOCTL_GET_OTP_STATUS,        SEC_BT_ENABLE, 1, NULL, 0, 0);
    //if (boot_mode_id)  board_ioctl(BOARD_IOCTL_GET_OTP_STATUS,        MRKT_ID_MASK, 1, NULL, 0, 0);

//    if (set_gpio)      board_ioctl(BOARD_IOCTL_SET_GPIO,              0, 0, NULL, var1, var2);
    if (cfe_version) {
        board_ioctl(BOARD_IOCTL_GET_CFE_VER,           0, 0, ioctl_buf, 64, 0);
        printf("%d.%d.%d-%d.%d\n",ioctl_buf[0],ioctl_buf[1],ioctl_buf[2],ioctl_buf[3],ioctl_buf[4]);
    }
    /* Dump 4 bytes */
    if (memdump) {
//        if (verbose) printf("Dumping addr: 0x%x\n",memdump);
//        board_ioctl(BOARD_IOCTL_DUMP_ADDR ,          0, 1, (char*)memdump, 4, 0);
    off_t          dev_base = 0x10000000; /* physical address of internal registers */
    int            memfd;
    int tmp;

    memfd = open("/dev/mem", O_RDWR | O_SYNC);

    if (memfd < 0){
        errx(1, "Could not open /dev/mem\n");
    }

    mapped_base = mmap(0, MAP_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, memfd, dev_base & ~MAP_MASK);

    if (mapped_base == MAP_FAILED)
        errx(1, "mmap failure");

    dump(memdump);
    munmap(mapped_base, MAP_SIZE);
    close(memfd);
    }

    close(fd);


    if (wan_interfaces) {
	int  sfd, ret=0;
        char *dev_names[10];
        struct ifreq interfaces;

	if ( (sfd = socket(AF_INET, SOCK_DGRAM, 0)) > 0 ) {
	    strcpy(interfaces.ifr_name, "bcmsw");
	    if (ioctl(sfd, SIOCGIFINDEX, &interfaces) < 0)
	    {
		close(sfd);
		printf("bcmsw interface does not exist.\n");
		return 1;
	    }

	    *dev_names = malloc(1024);
            memset((void *) &interfaces, sizeof(interfaces), 0);
            interfaces.ifr_data = *dev_names;

            if ((ret = ioctl(sfd, SIOCGWANPORT, &interfaces)) < 0) {
                printf("ioctl SIOCGWANPORT error: %d\n", ret);
                return 1;
            }

            printf("WAN devices: %s\n", *dev_names);

            close(sfd);
        }
	else {
	    printf("Error opening socket\n");
        }
    }
    return 0;
}

static int set_info(int gpio, int led, int state, int bootline, char* wan_interface, int spi, int spi_dev, char* payload) {
    fd = open("/dev/brcmboard", O_RDWR);
    if ( fd == -1 ) {
        fprintf(stderr, "failed to open: /dev/brcmboard\n");
        exit(1);
    }

    if (gpio)       board_ioctl(BOARD_IOCTL_SET_GPIO ,         0, 0, NULL, gpio-1, state);
    if (led)        board_ioctl(BOARD_IOCTL_LED_CTRL ,         0, 0, NULL, led-1, state);

    if (bootline == 0 || bootline == 1) {
	FILE *file = fopen("/proc/nvram/Bootline", "r+");
	char boot_param[256];
	int ch;
	int change = 0;

	fgets(boot_param, sizeof(boot_param), file);

	for(ch=0; boot_param[ch]!=NULL; ch++)
	    if(boot_param[ch] == 'p' && boot_param[ch+1] == '=') {
		if (boot_param[ch+2] != bootline+'0')
		    change = 1;
		(bootline == 0) ? (boot_param[ch+2] = '0') : (boot_param[ch+2] = '1');
            }

	if (change) {
	    for(ch=0; boot_param[ch]!=NULL; ch++)
	        fprintf(file,"%c", boot_param[ch]);
	}

	fclose(file);
   }

    if (wan_interface) {
        int  sfd, ret=0;
        struct ifreq interface;

        if ( (sfd = socket(AF_INET, SOCK_DGRAM, 0)) > 0) {
            strncpy(interface.ifr_name, wan_interface, sizeof(interface.ifr_name));
            interface.ifr_data = (void *)state;
            if ((ret = ioctl(sfd, SIOCSWANPORT, &interface)) < 0) {
                printf("SIOCSWANPORT %s error: %d\n", interface.ifr_name, ret);
                return 1;
            }

            close(sfd);
        } else {
            printf("Error opening socket: %s\n", wan_interface);
        }
    }

    if (spi) {
		unsigned char buf[6] = {8,1,0,0,0,0};
        board_ioctl(BOARD_IOCTL_SPI_INIT, spi_dev, 0, 0, 0, 391000);
		board_ioctl(BOARD_IOCTL_SPI_WRITE, spi_dev, 0, buf, 6, NULL);
	}
    return 0;
}

static int
generate_image(const char *in_file, char *sequence_number)
{
	int in_fp = 0;
	int rb;
	uint8_t readbuf[BLOCKSIZE];
	int read_bytes = 0;
	struct jffs2_raw_dirent *pdir = NULL;
	unsigned long version = 0;
	uint8_t *p;
	int possibly_jffs2 = 0;
	int sequence;
	char fixed_sequence[16];
	int ver_name_len = strlen(VERSION_NAME);

	if ((in_fp = open(in_file, O_RDWR)) < 0){
		fprintf(stderr, "failed to open: %s\n", in_file);
		exit(1);
	}

	if (verbose)
		fprintf(stderr, "|%s|\n", sequence_number);
		
	if (!sequence_number) {
		fprintf(stderr, "No sequence number\n");
		exit(1);
	}

	sequence = atoi(sequence_number);
	sprintf(fixed_sequence, "%03d", sequence+1);

	if (verbose)
		fprintf(stderr, "New sequence: %s\n",fixed_sequence);

	if (start_offset) {
		lseek(in_fp, start_offset, SEEK_SET);
		if (verbose)
			fprintf(stderr, "Skipping %lu bytes\n", start_offset);
	}

	if (fsimg_size && verbose)
		fprintf(stderr, "Image size limited to %lu bytes\n", fsimg_size);

	for (;;) {
		rb = read(in_fp, &readbuf, sizeof(readbuf));
		if (rb <= 0)
			break;

		if (fsimg_size && (read_bytes + rb) > fsimg_size)
			rb = fsimg_size - read_bytes;
		read_bytes += rb;

		p = (uint8_t*)&readbuf;

		while (p < (&readbuf[rb-1])) {
			pdir = (struct jffs2_raw_dirent *)p;
			
			if( pdir->magic == JFFS2_MAGIC_BITMASK ) {
				/* Ignore current sequence number (-3) */
				if( pdir->nodetype == JFFS2_NODETYPE_DIRENT &&
				    ver_name_len == pdir->nsize &&
				    !memcmp(VERSION_NAME, pdir->name, ver_name_len-3) ) {
					if( pdir->version > version ) {
						if( pdir->ino != 0 ) {
							if (verbose) {
								printf("Found cferam structure at offset %x\n", (unsigned int)p);
								printf("name    : %s\n",pdir->name);
								printf("name_crc: %x\n",pdir->name_crc);
								printf("nsize   : %d\n",pdir->nsize);
								printf("totlen  : %d\n",pdir->totlen);
							}
							
							goto end;
						}
					}
				}
				p += (pdir->totlen + 3) & ~0x03;
			} else {
				/* Skip the rest of this block */
				p = &readbuf[BLOCKSIZE];
				if (possibly_jffs2++ > 100)
					goto error;
			}
		}

		if (fsimg_size && read_bytes >= fsimg_size)
			break;
	}

error:
	printf("JFFS2 image corrupt or not JFFS2 image\n");
	return 1;

end:
	if(sequence == -1) {
		memcpy(fixed_sequence, pdir->name + (ver_name_len-3), 3);
		fixed_sequence[3] = '\0';
		printf("%s\n", fixed_sequence);
		goto out;
	}

	memcpy(pdir->name+7, fixed_sequence, 3); 
	pdir->name_crc = crc32(0, pdir->name, strlen((char*)pdir->name));
	if (verbose) {
		printf("new name    : %s sn: %s\n",pdir->name, fixed_sequence);
		printf("new name_crc: %x\n",pdir->name_crc);
	}

	if (lseek(in_fp, -BLOCKSIZE, SEEK_CUR) < 0) {
		fprintf(stderr, "seeking of file %s failed\n", in_file);
		exit(1);
	}
	
	write(in_fp, &readbuf, BLOCKSIZE);
out:
	close(in_fp);
	
	return 0;
}

static int
signature_check(const char *in_file, const char *mtd_device, int return_block_size, int return_flash_type, int return_chip_id, int return_version_nr, int return_image_type, int return_size_check, int return_iboard_id, int return_model_name, int return_oem) {
	int in_fp = 0;
	WFI_TAG brcm_tag = {0};
	uint8_t readbuf[READ_BUF];
	int rb, calc_bytes, read_bytes;
	uint32_t crc = IMAGETAG_CRC_START;
	uint32_t header_crc;
	struct stat file_stat;
	uint16_t *pbuf;
	bcm_tag_bccfe nor_tag = {{0}};
	char iboardid[5] = {0};
	union _char2int {
		uint32_t intval;
		char charval[4];
	} char2int;
	uint32_t tag_crc_calc, tag_crc, tagid_crc, tagid_crc_calc, image_crc;

    if(stat(in_file, &file_stat) < 0) {
		fprintf(stderr, "getting filesize of %s failed\n", in_file);
		exit(1);
	}
	if ((in_fp = open(in_file, O_RDONLY)) < 0) {
		fprintf(stderr, "failed to open: %s\n", in_file);
		exit(1);
	}

	/* Check if we have a header first */
	
	if (read(in_fp, &nor_tag, sizeof(bcm_tag_bccfe)) < 0) {
		fprintf(stderr, "reading tag data from file %s failed\n", in_file);
		exit(1);
	}

	/* Check if header CRC matches, if so probably valid header */
	strncpy(char2int.charval, nor_tag.headerCRC, 4);
	header_crc = char2int.intval;
	
	tag_crc_calc = crc32(IMAGETAG_CRC_START, (uint8_t*)&nor_tag, sizeof(nor_tag) - 20);
	tagid_crc_calc = crc32(IMAGETAG_CRC_START, (uint8_t*)&(nor_tag.tagId[0]), TAGID_LEN);
	strncpy(char2int.charval, nor_tag.tagIdCRC, 4);
	tagid_crc = char2int.intval;
	strncpy(char2int.charval, nor_tag.headerCRC, 4);
	tag_crc = char2int.intval;

	if (verbose) {
		fprintf(stderr, "calc tagId CRC     : %x\n", tagid_crc_calc);
		fprintf(stderr, "calc header CRC    : %x\n", tag_crc_calc);
		fprintf(stderr, "tagVersion         : %s\n", nor_tag.tagVersion);
		fprintf(stderr, "sig_1              : %s\n", nor_tag.sig_1);
		fprintf(stderr, "sig_2              : %s\n", nor_tag.sig_2);
		fprintf(stderr, "chipid             : %s\n", nor_tag.chipid);
		fprintf(stderr, "boardid            : %s\n", nor_tag.boardid);
		fprintf(stderr, "big_endian         : %s\n", nor_tag.big_endian);
		fprintf(stderr, "totalLength        : %s\n", nor_tag.totalLength);
		fprintf(stderr, "cfeAddress         : %s\n", nor_tag.cfeAddress);
		fprintf(stderr, "cfeLength          : %s\n", nor_tag.cfeLength);
		fprintf(stderr, "rootAddress        : %s\n", nor_tag.rootAddress);
		fprintf(stderr, "rootLength         : %s\n", nor_tag.rootLength);
		fprintf(stderr, "kernelAddress      : %s\n", nor_tag.kernelAddress);
		fprintf(stderr, "kernelLength       : %s\n", nor_tag.kernelLength);
		fprintf(stderr, "dualImage          : %s\n", nor_tag.dualImage);
		fprintf(stderr, "inactiveFlag       : %s\n", nor_tag.inactiveFlag);
		fprintf(stderr, "information1       : %s\n", nor_tag.information1);
		fprintf(stderr, "tagId              : %s\n", nor_tag.tagId);
		strncpy(char2int.charval, nor_tag.tagIdCRC, 4);
		fprintf(stderr, "tagIdCRC           : %x\n", char2int.intval);
		fprintf(stderr, "reserved1          : %s\n", nor_tag.reserved1);
		strncpy(char2int.charval, nor_tag.imageCRC, 4);
		fprintf(stderr, "imageCRC           : %x\n", char2int.intval);
		fprintf(stderr, "reserved2          : %s\n", nor_tag.reserved2);
		strncpy(char2int.charval, nor_tag.headerCRC, 4);
		fprintf(stderr, "headerCRC          : %x\n", char2int.intval);
		fprintf(stderr, "reserved3          : %s\n", nor_tag.reserved3);
	}
	
	/* If this is true assume that we found a proper tag */
	if ((tag_crc_calc == tag_crc) && (tagid_crc == tagid_crc_calc)) {
		if (verbose) fprintf(stderr, "Header and tagid crc matched\n");

		if (return_chip_id) {
			fprintf(stdout, "%s\n", nor_tag.chipid);
			exit(0);
		}
		if (return_flash_type) {
			fprintf(stdout, "NOR\n");
			exit(0);
		}
		if (return_block_size) {
			fprintf(stdout, "0\n");
			exit(0);
		}
		if (return_iboard_id) {
			fprintf(stdout, "%s\n",nor_tag.boardid);
			exit(0);
		}
		if (return_version_nr) {
			fprintf(stdout, "%s\n",nor_tag.tagVersion);
			exit(0);
		}
		if (return_image_type) {
			fprintf(stdout, "FS\n");
			exit(0);
		}

		/* Calculate CRC of file */
		strncpy(char2int.charval, nor_tag.imageCRC, 4);
		image_crc = char2int.intval;
		calc_bytes = atoi(nor_tag.totalLength);
		lseek(in_fp, sizeof(bcm_tag_bccfe), SEEK_SET);

		read_bytes = 0;
		crc = IMAGETAG_CRC_START;
		do {
			rb = read(in_fp, &readbuf, sizeof(readbuf));
			read_bytes += rb;

			
			/* Check if we reached end of file, if so don't include the trailer in crc calculations */
			if (read_bytes > calc_bytes) {
				crc = crc32(crc, readbuf, rb - (read_bytes-calc_bytes));
				break;
			}
			
			crc = crc32(crc, readbuf, rb);
		} while (rb > 0);
		
		if (verbose) {
			fprintf(stdout, "Calcbytes : %d\n",calc_bytes);
			fprintf(stdout, "calc CRC  : %x\n",crc);
			fprintf(stdout, "image CRC : %x\n",image_crc);
		}
		
		if (image_crc == crc) {
			fprintf(stdout, "CRC_OK\n");
			exit(0);
		} else {
			fprintf(stdout, "CRC_NOK\n");
			exit(1);
		}
	}

	if (return_oem) {
		char buf[100];
		if (lseek(in_fp, -OFFSET_OEM_CUSTOMER, SEEK_END) < 0) {
			fprintf(stderr, "seeking to end of file %s failed\n", in_file);
			exit(1);
		}
		if (read(in_fp, buf, sizeof(buf)) != sizeof(buf)) {
			fprintf(stderr, "reading customer name from file %s failed\n", in_file);
			exit(1);
		}
		buf[sizeof(buf)-1] = 0;
		fprintf(stdout, "%s\n", buf);
		exit(0);
	}
	if (return_model_name) {
		char buf[100];
		if (lseek(in_fp, -OFFSET_MODEL_NAME, SEEK_END) < 0) {
			fprintf(stderr, "seeking to end of file %s failed\n", in_file);
			exit(1);
		}
		if (read(in_fp, buf, sizeof(buf)) != sizeof(buf)) {
			fprintf(stderr, "reading model name from file %s failed\n", in_file);
			exit(1);
		}
		buf[sizeof(buf)-1] = 0;
		fprintf(stdout, "%s\n", buf);
		exit(0);
	}
		
	/* No header found look for trailer */
	
	if (lseek(in_fp, -sizeof(WFI_TAG), SEEK_END) < 0) {
		fprintf(stderr, "seeking to end of file %s failed\n", in_file);
		exit(1);
	}
	if (read(in_fp, &brcm_tag, sizeof(WFI_TAG)) < 0) {
		fprintf(stderr, "reading tag data from file %s failed\n", in_file);
		exit(1);
	}
	if (lseek(in_fp, 0, SEEK_SET) < 0) {
		fprintf(stderr, "seeking to beginning of file %s failed\n", in_file);
		exit(1);
	}

	if (return_chip_id) {
		fprintf(stdout, "%X\n",brcm_tag.wfiChipId);
		exit(0);
	}
	if (return_iboard_id) {
		memcpy(iboardid, &brcm_tag.wfiReserved,4);
		fprintf(stdout, "%s\n",iboardid);
		exit(0);
	}

	if (return_block_size) {
		switch (brcm_tag.wfiFlashType) {
			case WFI_NAND128_FLASH:
			case WFI_NANDFS128_IMAGE:
				fprintf(stdout, "128\n"); exit(0); break;
			case WFI_NAND16_FLASH:
			case WFI_NANDFS16_IMAGE:
				fprintf(stdout, "16\n");  exit(0); break;
			default:
				fprintf(stdout, "0\n");   exit(0); break;
		}
	}

	if (return_flash_type) {
		switch (brcm_tag.wfiFlashType) {
			case WFI_NAND128_FLASH:
			case WFI_NANDFS128_IMAGE:
			case WFI_NAND16_FLASH:
			case WFI_NANDFS16_IMAGE:
				fprintf(stdout, "NAND\n");  exit(0); break;
			case WFI_NOR_FLASH:
				fprintf(stdout, "NOR\n"); exit(0); break;
			default:
				fprintf(stdout, "UNKNOWN\n");   exit(0); break;
		}

	}

	if (return_image_type) {
		switch (brcm_tag.wfiFlashType) {
			case WFI_NANDFS128_IMAGE:
			case WFI_NANDFS16_IMAGE:
				fprintf(stdout, "FS\n");  exit(0); break;
			case WFI_NANDCFE_IMAGE:
			case WFI_NORCFE_IMAGE:
				fprintf(stdout, "CFE\n");  exit(0); break;
			case WFI_NAND128_FLASH:
			case WFI_NAND16_FLASH:
			case WFI_NOR_FLASH:
			{
				rb = read(in_fp, &readbuf, sizeof(readbuf));
				pbuf = (uint16_t*) readbuf;
				if (pbuf[0] == JFFS2_MAGIC_BITMASK) {
					fprintf(stdout, "FS\n");  exit(0); break;
				} else if (file_stat.st_size > 131072) {
					fprintf(stdout, "CFE+FS\n");  exit(0); break;
				} else
					fprintf(stdout, "CFE\n");  exit(0); break;
			}
			default:
				fprintf(stdout, "UNKNOWN\n");  exit(0); break;
		}
	}

	if (return_size_check) {
		struct stat st;
		FILE *file;
		char line[128];
		unsigned long rootfs_size;
		unsigned long image_size;
		unsigned char *mtdsize;
		unsigned char *blocksize;
		char mtdname[8];
		char partname[16];

		/* Get rootfs size */
		if (file = fopen("/proc/mtd", "r")) {
			while(fgets(line, sizeof(line), file) != NULL)
			{
				if (sscanf(line, "%s %x %x %s\n", &mtdname, &mtdsize, &blocksize, &partname) && !strcmp(partname, "\"rootfs\""))
					break;
			}
			fclose(file);
		}
		rootfs_size = (unsigned long) mtdsize;

		/* Get image file size*/
		stat(in_file, &st);
		image_size = st.st_size;

		/* Compare */
		if (rootfs_size > image_size) {
			fprintf(stdout, "SIZE_OK\n");
			exit(0);
		}
		else {
			fprintf(stdout, "SIZE_FAIL\n");
			exit(0);
		}
	}

	calc_bytes = file_stat.st_size - sizeof(WFI_TAG);
	read_bytes = 0;
	crc = IMAGETAG_CRC_START;

	/* Calculate CRC of file */
	do {
		rb = read(in_fp, &readbuf, sizeof(readbuf));
		read_bytes += rb;

		
		/* Check if we reached end of file, if so don't include the trailer in crc calculations */
		if (read_bytes > calc_bytes) {
			crc = crc32(crc, readbuf, rb - (read_bytes-calc_bytes));
			break;
		}
		
		crc = crc32(crc, readbuf, rb);
	} while (rb > 0);

	if (verbose) {
		fprintf(stderr, "CRC of file %s is: %X\n", in_file, crc);
		fprintf(stderr, "CRC of trailer is      : %X\n", brcm_tag.wfiCrc);
		fprintf(stderr, "Chipid of trailer is   : %X\n", brcm_tag.wfiChipId);
		fprintf(stderr, "Reserved of trailer is : %X\n", brcm_tag.wfiReserved);
		fprintf(stderr, "Version of trailer is  : %X\n", brcm_tag.wfiVersion);
		fprintf(stderr, "Flashtype of trailer is: %X\n", brcm_tag.wfiFlashType);
	}

	if (brcm_tag.wfiCrc == crc) {
		fprintf(stdout, "CRC_OK\n");
		if (verbose)
			fprintf(stderr, "CRC of file %s is correct\n", in_file);
		return 0;
	} else {
		fprintf(stdout, "CRC_NOK\n");
		if (verbose)
			fprintf(stderr, "CRC of file %s is not correct\n", in_file);
		return 1;
	}
}


static void usage(void)
{
	fprintf(stderr, "Usage: brcm_fw_tool [<options> ...] <command> [<arguments> ...] \n\n"
	"brcm_fw_tool recognizes these commands:\n"
	"        check                       check the update firmware signature\n"
	"        update                      generate nand firmware image for update\n"
    "        write                       write firmware image to flash\n"
    "        info                        get information from the kernel board ioctl\n"
    "        set                         set information via the kernel board ioctl\n"
	"Following options are available:\n"
	"        -s <sequencenumber>         set new sequence number in output image\n"
	"                                    [000-999] or -1 to just read current value\n"
	"        -d <mtddevice>              mtd device to compare input file against\n"
	"        -b                          return block size from signature check\n"
	"        -t                          return flash type from signature check\n"
	"        -c                          return chipid from signature check\n"
	"        -v                          return version from signature check\n"
	"        -m                          return model name from signature check\n"
	"        -o                          return oem customer from signature check\n"
	"        -i                          return image type\n"
	"        -r                          return iboard id\n"
	"        -y                          return size check (determines if image size is appropriate)\n"
	"        -w                          use this option if the input is a .w (image + cfe) file\n"
	"        -W <start-offset>           set byte offset of fs image in file (use either -W or -w)\n"
	"        -Z <fsimg-size>             limit number of bytes scanned during update\n"
    "---- info options ----\n"
    "        -g <hex addr>               get 32 bit memdump of addr\n"
    "        -k                          get the SoC model from the kernel\n"
    "        -f                          get the flash size from the kernel\n"
    "        -e                          get the SoC model revision from the kernel\n"
    "        -l                          get the cfe version from the kernel\n"
    "        -w                          get the configured wan interfaces\n"
    "        -I                          get the boot mode\n"
    "        -M                          get the boot mode id\n"
    "---- set options ----\n"
    "        -h <gpio>                   set the led gpio to affect with state -p\n"
    "        -x <gpio>                   set the gpio io to affect with state -p\n"
    "        -p <state>                  state of led or gpio io 0/1\n"
    "        -u <0|1>                    set bootline\n"
    "        -d <interface> -p <0|1>     set the wan state of an interface\n"
    "        -k -s <dev> -p <payload>    write hex payload to spi device\n"
    "---- write options ----\n"
    "        -q                          write cfe to flash\n"
    "        -a                          write file system to flash\n"
    "---- generic options ----\n"
    "        -V                          verbose mode\n"
    "\n"
	"Example: To set a new sequence number on a file\n"
	"         brcm_fw_tool -s 002  update firmware_in firmware_out\n"
	"         To check the signature, returns 0 on succes, 1 on fail\n"
	"         brcm_fw_tool check firmware_in\n"
	"         To return the chip id from the firmware signature\n"
	"         brcm_fw_tool -c check firmware_in\n"
    "         To return the chip id from the kernel\n"
    "         brcm_fw_tool -k info\n"
	"\n");
	exit(1);
}


int main (int argc, char **argv)
{
	int ch;
	int  eof_marker        = 1;
	int  erasesize         = 128*1024;
	int  return_block_size = 0;
	int  return_flash_type = 0;
	int  return_chip_id    = 0;
	int  return_version_nr = 0;
	int  return_image_type = 0;
	int  return_size_check = 0;
	int  return_iboard_id  = 0;
	int  return_model_name = 0;
	int  return_oem        = 0;
    int  kernel_chip_id    = 0;
    int  kernel_flash_size = 0;
    int  kernel_chip_rev   = 0;
    int  kernel_cfe_ver    = 0;
    int  write_cfe         = 0;
    int  write_fs          = 0;
    int  wan_interfaces    = 0;
    int  status            = 0;
    int  boot_mode         = 0;
    int  boot_mode_id      = 0;
    int  spi               = 0;
    int  spi_dev           = 0;
    int  offset            = 0;
	char *payload          = NULL;
	char *in_file          = NULL;
	char *mtd_device       = NULL;
	char *sequence_number  = NULL;
	char *jffs2dir         = NULL;
    char *wan_interface    = NULL;
    int memdump_addr       = 0;
    int gpio=0, led=0, state=0, bootline=-1;
	enum {
		CMD_SIGNATURE_CHECK,
		CMD_GENERATE_UPDATE_IMAGE,
        CMD_WRITE_FLASH_IMAGE,
        CMD_GET_INFO,
        CMD_SET_INFO,
	} cmd = -1;


	while ((ch = getopt(argc, argv,

			"g:SIMlefriyqjbtkvwW:Z:Vzmoac:d:s:n:h:x:u:p:")) != -1)
		switch (ch) {
            case 'I':
                boot_mode = 1;
                break;
            case 'M':
                boot_mode_id = 1;
                break;
            case 'h':
                led = atoi(optarg)+1;
                break;
            case 'p':
                state = atoi(optarg);
				payload = optarg;
                break;
            case 'x':
                gpio = atoi(optarg)+1;
                break;
            case 'u':
                bootline = atoi(optarg);
                break;
            case 'i':
		return_image_type = 1;
		break;
            case 'y':
		return_size_check = 1;
		break;
            case 'g':
                memdump_addr =  strtol(optarg, NULL, 16);
                break;
			case 'j':
				eof_marker = 0;
				break;
			case 'b':
				return_block_size = 1;
				break;
			case 't':
				return_flash_type = 1;
				break;
			case 'c':
				return_chip_id = 1;
				break;
			case 'v':
				return_version_nr = 1;
				break;
			case 'r':
				return_iboard_id = 1;
				break;
			case 'm':
				return_model_name = 1;
				break;
			case 'o':
				return_oem = 1;
				offset = atoi(optarg);
				break;
			case 'w':
				start_offset = 0x20000;
                wan_interfaces = 1;
				break;
			case 'W':
				start_offset = strtol(optarg, NULL, 10);
				break;
			case 'Z':
				fsimg_size = strtol(optarg, NULL, 10);
				break;
			case 'V':
				verbose = 1;
				break;			
			case 'z':
				erasesize = 16*1024;
				break;
			case 'a':
				write_fs = 1;
				break;
			case 'n':
				jffs2dir = optarg;
				break;
			case 'd':
                wan_interface = optarg;
				mtd_device = optarg;
				break;
			case 's':
				sequence_number = optarg;
				spi_dev = atoi(optarg);
				break;
            case 'k':
                kernel_chip_id = 1;
				spi = 1;
                break;
            case 'f':
                kernel_flash_size = 1;
                break;
            case 'e':
                kernel_chip_rev = 1;
                break;
            case 'l':
                kernel_cfe_ver = 1;
                break;
            case 'q':
                write_cfe = 1;
                break;
            case 'S':
                status = 1;
                break;
            case '?':
			default:
				usage();
		}
	argc -= optind;
	argv += optind;

	if (argc < 1)
		usage();

	if (verbose) fprintf(stderr, "Verbose mode active\n");
	
	if        ((strcmp(argv[0], "check") == 0) && (argc == 2)) {
		cmd = CMD_SIGNATURE_CHECK;
		in_file = argv[1];
	} else if ((strcmp(argv[0], "update") == 0) && (argc == 2)) {
		cmd = CMD_GENERATE_UPDATE_IMAGE;
		in_file = argv[1];
    } else if ((strcmp(argv[0], "write") == 0) && (argc == 2)) {
        cmd = CMD_WRITE_FLASH_IMAGE;
        in_file = argv[1];
    } else if ((strcmp(argv[0], "info") == 0) && (argc >= 1)) {
        cmd = CMD_GET_INFO;
    } else if ((strcmp(argv[0], "set") == 0) && (argc >= 1)) {
        cmd = CMD_SET_INFO;
    } else {
		usage();
	}

	switch (cmd) {
		case CMD_SIGNATURE_CHECK:
			if (verbose) fprintf(stderr, "Checking signature.\n");
			signature_check(in_file, mtd_device, return_block_size, return_flash_type, return_chip_id, return_version_nr, return_image_type, return_size_check, return_iboard_id, return_model_name, return_oem);
			break;
		case CMD_GENERATE_UPDATE_IMAGE:
			if (verbose) fprintf(stderr, "Generating firmware image.\n");
			generate_image(in_file, sequence_number);
			break;
        case CMD_WRITE_FLASH_IMAGE:
            if (verbose) fprintf(stderr, "Writing firmware image to flash.\n");
            write_flash_image(in_file, write_cfe, write_fs);
            break;
        case CMD_GET_INFO:
            if (verbose) fprintf(stderr, "Getting kernel info.\n");
            get_info(memdump_addr, kernel_chip_id, kernel_flash_size, kernel_chip_rev, kernel_cfe_ver, wan_interfaces, status, boot_mode, boot_mode_id);
            break;
        case CMD_SET_INFO:
            if (verbose) fprintf(stderr, "Setting kernel info.\n");
            set_info(gpio, led, state, bootline, wan_interface, spi, spi_dev, payload);
            break;
        default:
			fprintf(stderr, "Unknown command %d\n", cmd);
			return 1;
	}


	return 0;
}
