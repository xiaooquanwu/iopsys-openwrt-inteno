CC = gcc
CFLAGS += -Wall

CFLAGS += -I$(STAGING_DIR)/usr/include/bcm963xx/shared/opensource/include/bcm963xx/
CFLAGS += -I$(STAGING_DIR)/usr/include/bcm963xx/bcmdrivers/opensource/include/bcm963xx/


%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $<

obj = brcm_fw_tool.o jffs2.o crc32.o
obj.brcm47xx = $(obj.brcm)

brcm_fw_tool: $(obj) $(obj.$(TARGET))
clean:
	rm -f *.o jffs2 
