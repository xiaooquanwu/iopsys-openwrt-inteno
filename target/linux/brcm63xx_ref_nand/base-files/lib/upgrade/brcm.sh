#!/bin/sh
check_crc() {
	brcm_fw_tool check $1
}

get_flash_type() {
	brcm_fw_tool -t check $1
}

get_image_type() {
	brcm_fw_tool -i check $1
}

get_image_chip_id() {
	brcm_fw_tool -c check $1
}

get_image_board_id() {
	brcm_fw_tool -r check $1
}

check_image_size() {
	brcm_fw_tool -y check $1
}
