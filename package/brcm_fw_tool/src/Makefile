CC = gcc
CFLAGS += -Wall
CFLAGS += -I$(BUILD_DIR)/bcmkernel-4.12/4.12/bcmdrivers/opensource/include/bcm963xx/
CFLAGS += -I$(BUILD_DIR)/bcmkernel-4.12/4.12/shared/opensource/include/bcm963xx/
CFLAGS += -I$(BUILD_DIR)/bcmopen-consumer/4.12/bcmdrivers/opensource/include/bcm963xx/
CFLAGS += -I$(BUILD_DIR)/bcmopen-consumer/4.12/shared/opensource/include/bcm963xx/
CFLAGS += -I$(BUILD_DIR)/bcmkernel-3.4-4.14/4.14/bcmdrivers/opensource/include/bcm963xx/
CFLAGS += -I$(BUILD_DIR)/bcmkernel-3.4-4.14/4.14/shared/opensource/include/bcm963xx/


obj = brcm_fw_tool.o jffs2.o crc32.o
obj.brcm47xx = $(obj.brcm)

brcm_fw_tool: $(obj) $(obj.$(TARGET))
clean:
	rm -f *.o jffs2 
