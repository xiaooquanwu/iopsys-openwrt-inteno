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

	# Customer name check should be carried out
	# only if a regarding parameter set in config.
	# For now skip customer name check.
	if [ 1 -eq 0 ]; then
		[ -f /lib/db/version/iop_customer ] \
			&& [ "$(get_image_customer "$from")" != "$(cat /lib/db/version/iop_customer)" ] && {
			echo "Image customer doesn't match" > /dev/console
			return 1
		}
		# NOTE: expr interprets $(db get hw.board.hardware) as a
		# regexp which could give unexpected results if the harware
		# name contains any magic characters.
		expr "$(get_image_model_name "$from")" : "$(db get hw.board.hardware)" || {
			echo "Image model name doesn't match board hardware" > /dev/console
			return 1
		}
	fi

	echo "Image platform check completed" > /dev/console

	return 0
}

# use default for platform_do_upgrade()
