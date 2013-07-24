. /lib/upgrade/brcm.sh
platform_check_image_size() {
	[ "$(check_image_size "$1")" == "SIZE_OK" ] || {
		return 1
	}
	return 0
}

platform_check_image() {
	local from

	[ "$ARGC" -gt 1 ] && return 1

	echo "Image platform check started ..." > /dev/console

	case "$1" in
		http://*|ftp://*) get_image "$1" "cat" > /tmp/firmware.img; from=/tmp/firmware.img;;
		*) from=$1;;
	esac

	

	[ "$(check_crc "$from")" == "CRC_OK" ] || {
		echo "CRC check failed" > /dev/console
		return 1
	}

	if [ "$(get_image_type "$from")" == "CFE+FS" ]; then
		echo 1 > /tmp/CFE_FS
	elif [ "$(get_image_type "$from")" == "FS" ]; then
		echo 0 > /tmp/CFE_FS
	else
		echo "Unknown image type" > /dev/console
		return 1
	fi

	if [ "$(get_flash_type "$from")" == "NAND" ]; then
		echo 1 > /tmp/IS_NAND
	elif [ "$(get_flash_type "$from")" == "NOR" ]; then
		echo 0 > /tmp/IS_NAND
	else
		echo "Unknown flash type" > /dev/console
		return 1
	fi

	[ "$(check_image_size "$from")" == "SIZE_OK" ] || {
		echo "Image size is too large" > /dev/console
		return 1
	}

	echo "Image platform check completed" > /dev/console

	return 0
}

# use default for platform_do_upgrade()
