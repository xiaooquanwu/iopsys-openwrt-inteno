#!/bin/sh

RAM_ROOT=/tmp/root

ldd() { LD_TRACE_LOADED_OBJECTS=1 $*; }
libs() { ldd $* | awk '{print $3}'; }

install_file() { # <file> [ <file> ... ]
	for file in "$@"; do
		dest="$RAM_ROOT/$file"
		[ -f $file -a ! -f $dest ] && {
			dir="$(dirname $dest)"
			mkdir -p "$dir"
			cp $file $dest
		}
	done
}

install_bin() { # <file> [ <symlink> ... ]
	src=$1
	files=$1
	[ -x "$src" ] && files="$src $(libs $src)"
	install_file $files
	[ -e /lib/ld.so.1 ] && {
		install_file /lib/ld.so.1
	}
	shift
	for link in "$@"; do {
		dest="$RAM_ROOT/$link"
		dir="$(dirname $dest)"
		mkdir -p "$dir"
		[ -f "$dest" ] || ln -s $src $dest
	}; done
}

supivot() { # <new_root> <old_root>
	/bin/mount | grep "on $1 type" 2>&- 1>&- || /bin/mount -o bind $1 $1
	mkdir -p $1$2 $1/proc $1/sys $1/dev $1/tmp $1/overlay && \
	/bin/mount -o noatime,move /proc $1/proc && \
	pivot_root $1 $1$2 || {
		/bin/umount -l $1 $1
		return 1
	}

	/bin/mount -o noatime,move $2/sys /sys
	/bin/mount -o noatime,move $2/dev /dev
	/bin/mount -o noatime,move $2/tmp /tmp
	/bin/mount -o noatime,move $2/overlay /overlay 2>&-
	return 0
}

run_ramfs() { # <command> [...]
	install_bin /bin/busybox /bin/ash /bin/sh /bin/mount /bin/umount	\
		/sbin/pivot_root /usr/bin/wget /sbin/reboot /bin/sync /bin/dd	\
		/bin/grep /bin/cp /bin/mv /bin/tar /usr/bin/md5sum "/usr/bin/["	\
		/bin/dd /bin/vi /bin/ls /bin/cat /usr/bin/awk /usr/bin/hexdump	\
		/bin/sleep /bin/zcat /usr/bin/bzcat /usr/bin/printf /usr/bin/wc \
		/bin/cut /usr/bin/printf /bin/sync /bin/mkdir /bin/rmdir	\
		/bin/rm /usr/bin/basename /bin/kill /bin/chmod

	install_bin /sbin/mtd
	install_bin /sbin/ubi
	install_bin /sbin/mount_root
	install_bin /sbin/snapshot
	install_bin /sbin/snapshot_tool
	install_bin /usr/sbin/ubiupdatevol
	install_bin /usr/sbin/ubiattach
	install_bin /usr/sbin/ubiblock
	install_bin /usr/sbin/ubiformat
	install_bin /usr/sbin/ubidetach
	install_bin /usr/sbin/ubirsvol
	install_bin /usr/sbin/ubirmvol
	install_bin /usr/sbin/ubimkvol
	for file in $RAMFS_COPY_BIN; do
		install_bin ${file//:/ }
	done
	install_file /etc/resolv.conf /lib/*.sh /lib/functions/*.sh /lib/upgrade/*.sh $RAMFS_COPY_DATA

	[ -L "/lib64" ] && ln -s /lib $RAM_ROOT/lib64

	supivot $RAM_ROOT /mnt || {
		echo "Failed to switch over to ramfs. Please reboot."
		exit 1
	}

	/bin/mount -o remount,ro /mnt
	/bin/umount -l /mnt

	grep /overlay /proc/mounts > /dev/null && {
		/bin/mount -o noatime,remount,ro /overlay
		/bin/umount -l /overlay
	}

	# spawn a new shell from ramdisk to reduce the probability of cache issues
	exec /bin/busybox ash -c "$*"
}

kill_remaining() { # [ <signal> ]
	local sig="${1:-TERM}"
	echo -n "Sending $sig to remaining processes ... "

	local my_pid=$$
	local my_ppid=$(cut -d' ' -f4  /proc/$my_pid/stat)
	local my_ppisupgraded=
	grep -q upgraded /proc/$my_ppid/cmdline >/dev/null && {
		local my_ppisupgraded=1
	}

	local stat
	for stat in /proc/[0-9]*/stat; do
		[ -f "$stat" ] || continue

		local pid name state ppid rest
		read pid name state ppid rest < $stat
		name="${name#(}"; name="${name%)}"

		local cmdline
		read cmdline < /proc/$pid/cmdline

		# Skip kernel threads
		[ -n "$cmdline" ] || continue

		if [ $$ -eq 1 ] || [ $my_ppid -eq 1 ] && [ -n "$my_ppisupgraded" ]; then
			# Running as init process, kill everything except me
			if [ $pid -ne $$ ] && [ $pid -ne $my_ppid ]; then
				echo -n "$name "
				kill -$sig $pid 2>/dev/null
			fi
		else
			case "$name" in
				# Skip essential services
				*procd*|*ash*|*init*|*watchdog*|*ssh*|*dropbear*|*telnet*|*login*|*hostapd*|*wpa_supplicant*|*nas*|*cwmpd*|*ice*) : ;;

				# Killable process
				*)
					if [ $pid -ne $$ ] && [ $ppid -ne $$ ]; then
						echo -n "$name "
						kill -$sig $pid 2>/dev/null
					fi
				;;
			esac
		fi
	done
	echo ""
}

run_hooks() {
	local arg="$1"; shift
	for func in "$@"; do
		eval "$func $arg"
	done
}

ask_bool() {
	local default="$1"; shift;
	local answer="$default"

	[ "$INTERACTIVE" -eq 1 ] && {
		case "$default" in
			0) echo -n "$* (y/N): ";;
			*) echo -n "$* (Y/n): ";;
		esac
		read answer
		case "$answer" in
			y*) answer=1;;
			n*) answer=0;;
			*) answer="$default";;
		esac
	}
	[ "$answer" -gt 0 ]
}

v() {
	[ "$VERBOSE" -ge 1 ] && echo "$@" > /dev/console
}

rootfs_type() {
	/bin/mount | awk '($3 ~ /^\/$/) && ($5 !~ /rootfs/) { print $5 }'
}

get_image_sequence_number() {
	if [ $2 -eq 1 ]; then
		brcm_fw_tool -s -1 -w update $1
	else
		brcm_fw_tool -s -1 update $1
	fi
}

get_rootfs_sequence_number() {
	echo $(ls /cferam.* | cut -d. -f2 | sed 's/^0\+\([0-9]\)/\1/')
}

update_sequence_number() {
	local from=$1
	local seqn=$2
	local ofs=$3
	local size=$4

	[ $seqn -eq 0 ] && seqn=$(get_rootfs_sequence_number)
	[ -z "$ofs" ] && ofs=0
	[ -z "$size" ] && size=0

	brcm_fw_tool -s "$seqn" -W $ofs -Z $size update $1
}

is_inteno_image() {
	[ "$(dd if=$1 bs=1 count=10 2>/dev/null)" == "IntenoBlob" ] && return 0
	return 1
}

get_inteno_tag_val() {
	local from="$1"
	local tag="$2"

	head -c 1024 $from |awk "/^$tag / {print \$2}"
}

check_crc() {
	local from=$1
	local file_sz calc csum

	if is_inteno_image $from; then
		case $(get_inteno_tag_val $from integrity) in
		MD5SUM)
			file_sz=$(ls -l $from |awk '{print $5}')
			calc=$(head -c $(($file_sz-32)) $from |md5sum |awk '{print $1}')
			csum=$(tail -c 32 $from)
			[ "$calc" == "$csum" ] && echo "CRC_OK" || echo "CRC_BAD"
			;;
		*)
			echo "UNKNOWN"
			;;
		esac
	else
		brcm_fw_tool check $from
	fi
}

get_flash_type() {
	if is_inteno_image $1; then
		echo "NAND"
	else
		brcm_fw_tool -t check $1
	fi
}

get_image_type() {
	if is_inteno_image $1; then
		echo "INTENO"
	else
		brcm_fw_tool -i check $1
	fi
}

get_image_chip_id() {
	if is_inteno_image $1; then
		get_inteno_tag_val $1 chip
	else
		brcm_fw_tool -c check $1
	fi
}

get_image_board_id() {
	if is_inteno_image $1; then
		get_inteno_tag_val $1 board
	else
		brcm_fw_tool -r check $1
	fi
}

get_image_model_name() {
	if is_inteno_image $1; then
		get_inteno_tag_val $1 model
	else
		brcm_fw_tool -m check $1
	fi
}

get_image_customer() {
	if is_inteno_image $1; then
		get_inteno_tag_val $1 customer
	else
		brcm_fw_tool -o check $1
	fi
}


check_image_size() {
	if is_inteno_image $1; then
		# FIXME!
		echo "SIZE_OK"
	else
		brcm_fw_tool -y check $1
	fi
}

get_image() { # <source> [ <command> ]
	local from="$1"
	local conc="$2"
	local cmd

	case "$from" in
		http://*|ftp://*) cmd="wget -O- -q";;
		*) cmd="cat";;
	esac
	if [ -z "$conc" ]; then
		local magic="$(eval $cmd $from 2>/dev/null | dd bs=2 count=1 2>/dev/null | hexdump -n 2 -e '1/1 "%02x"')"
		case "$magic" in
			1f8b) conc="zcat";;
			425a) conc="bzcat";;
		esac
	fi

	eval "$cmd $from 2>/dev/null ${conc:+| $conc}"
}

get_magic_word() {
	(get_image "$@" | dd bs=2 count=1 | hexdump -v -n 2 -e '1/1 "%02x"') 2>/dev/null
}

get_magic_long() {
	(get_image "$@" | dd bs=4 count=1 | hexdump -v -n 4 -e '1/1 "%02x"') 2>/dev/null
}

refresh_mtd_partitions() {
	mtd refresh rootfs
}

jffs2_copy_config() {
	if grep rootfs_data /proc/mtd >/dev/null; then
		# squashfs+jffs2
		mtd -e rootfs_data jffs2write "$CONF_TAR" rootfs_data
	else
		# jffs2
		mtd jffs2write "$CONF_TAR" rootfs
	fi
}

find_mtd_no() {
	local part=$(awk -F: "/\"$1\"/ { print \$1 }" /proc/mtd)
	echo ${part##mtd}
}

cfe_image_upgrade() {
	local from=$1
	local ofs=$2
	local size=$3

	local mtd_no=$(find_mtd_no "nvram")
	dd if=/dev/mtd$mtd_no bs=1 count=1k skip=1408 \
	   of=$from seek=$(( $ofs + 1408 )) conv=notrunc
	imagewrite -c -k $ofs -l $size /dev/mtd$mtd_no $from
}

make_nvram2_image() {
	local img=$1
	local mtd_no

	echo -ne "NVRAM\n\377\377\000\000\000\001\377\377\377\377" > $img
	cat /dev/zero | tr '\000' '\377' | head -c $(( 2048 - 16 )) >> $img

	mtd_no=$(find_mtd_no "nvram")
	dd if=/dev/mtd$mtd_no bs=1 count=1k skip=1408 \
	   of=$img seek=16 conv=notrunc
}

build_springboard_rootfs() {
	local root_dir=$1
	local new_seqn=$2

	cd $root_dir

	touch ./cferam.$(echo $new_seqn | awk '{printf("%03u",$1)}')
	cp /vmlinux.lz ./

	mkdir bin
	cp /bin/busybox bin
	cp -d /bin/ash bin
	cp -d /bin/mount bin
	cp -d /bin/sh bin
	cp -d /bin/sleep bin
	cp -d /bin/sync bin

	mkdir dev
	mknod -m 644 dev/mtd1     c  90  2

	mkdir etc

	mkdir lib
	cp -d /lib/libcrypt.so.0 lib
	cp /lib/libcrypt-*.so lib
	cp -d /lib/libm.so.0 lib
	cp /lib/libm-*.so lib
	cp /lib/libgcc_s.so.1 lib
	cp -d /lib/libc.so.0 lib
	cp /lib/libuClibc-*.so lib
	cp -d /lib/ld-uClibc.so.0 lib
	cp /lib/ld-uClibc-*.so lib

	mkdir lib/upgrade
	mkdir overlay
	mkdir proc

	mkdir sbin
	cp -d /sbin/reboot sbin

	mkdir sys
	mkdir tmp
	mkdir usr
	mkdir usr/bin

	mkdir usr/sbin
	cp /usr/sbin/imagewrite usr/sbin

	cd -
}

copy_config_to_springboard() {
	local newroot=$1

	if [ -e /SAVE_CONFIG ]; then
		touch $newroot/overlay/SAVE_CONFIG
		cp -dpfr /overlay/etc $newroot/overlay && true
	fi
}

create_springboard_preinit() {
	local dest=$1
	local from=$2
	local k_ofs=$3
	local k_sz=$4
	local ubifs_ofs=$5
	local ubifs_sz=$6

	local mtd_no=1

	cat <<-EOF > $dest
		#!/bin/sh
		export PATH=/bin:/sbin:/usr/bin:/usr/sbin
		mount -t sysfs sysfs /sys
		echo "=== Springboard upgrade ==="
		echo "Write secondary nvram..."
		imagewrite -s 0 -b 3 /dev/mtd$mtd_no /lib/upgrade/nvram2.img
		echo "Wipe kernel areas..."
		imagewrite -s 3 -b 80 /dev/mtd$mtd_no
		echo "Write UBI image..."
		imagewrite -s 83 -b 0 \
			   -k $ubifs_ofs -l $ubifs_sz \
			   --ubi -n 0 -S -20 --vol-name=rootfs_0 \
			   /dev/mtd$mtd_no $from
		echo "Write kernel..."
		imagewrite -c -s 3 -b 40 -k $k_ofs -l $k_sz \
			   /dev/mtd$mtd_no $from
		echo "Reboot into new filesystem..."
		sync
		reboot -f
		sleep 10
	EOF

	chmod +x $dest
}

inteno_image_upgrade() {
	local from=$1
	local cfe_ofs cfe_sz nvram_ofs nvram_sz k_ofs k_sz
	local ubi_ofs ubi_sz ubifs_ofs ubifs_sz
	local cur_vol upd_vol mtd_no seqn

	v "Parsing image header..."

	cfe_ofs=1024
	cfe_sz="$(get_inteno_tag_val $from cfe)"
	[ $cfe_sz -gt 0 ] && v "- CFE: offset=$cfe_ofs, size=$cfe_sz"

	k_ofs=$(( $cfe_ofs + $cfe_sz ))
	k_sz="$(get_inteno_tag_val $from vmlinux)"
	[ $k_sz -gt 0 ] && v "- kernel: offset=$k_ofs, size=$k_sz"

	ubifs_ofs=$(( $k_ofs + $k_sz ))
	ubifs_sz="$(get_inteno_tag_val $from ubifs)"
	[ $ubifs_sz -gt 0 ] && v "- ubifs: offset=$ubifs_ofs, size=$ubifs_sz"

	if [ $(( $ubifs_ofs + $ubifs_sz )) -gt \
	     $(ls -l $from |awk '{print $5}') ]; then
		echo "Image file too small, upgrade aborted!" >&2
		return
	fi

	if [ $cfe_sz -gt 0 -a $k_sz -eq 0 -a $ubifs_sz -eq 0 ]; then

		# CFE only upgrade

		v "Writing CFE image to nvram (boot block) partition ..."
		cfe_image_upgrade $from $cfe_ofs $cfe_sz

	elif [ $k_sz -gt 0 -a $ubifs_sz -gt 0 ]; then

		# Kernel + filesystem upgrade

		if [ $cfe_sz -gt 0 ]; then
			v "Writing CFE image to nvram (boot block) partition ..."
			cfe_image_upgrade $from $cfe_ofs $cfe_sz
		fi

		if cat /proc/cmdline |grep -q "ubi:rootfs_"; then
		
			# Upgrade for ubi flash layout

			if cat /proc/cmdline |grep -q "ubi:rootfs_0"; then
				cur_vol=0
				upd_vol=1
			else
				cur_vol=1
				upd_vol=0
			fi

			v "Killing old kernel..."
			mtd_no=$(find_mtd_no "kernel_$upd_vol")
			imagewrite -b 8 /dev/mtd$mtd_no

			v "Writing UBIFS data to rootfs_$upd_vol volume ..."
			ubiupdatevol /dev/ubi0_$upd_vol \
				     --size=$ubifs_sz --skip=$ubifs_ofs $from

			v "Writing kernel image to kernel_$upd_vol partition ..."
			mtd_no=$(find_mtd_no "kernel_$cur_vol")
			seqn=$(get_image_sequence_number /dev/mtd$mtd_no 0)
			update_sequence_number $from $seqn $k_ofs $k_sz
			mtd_no=$(find_mtd_no "kernel_$upd_vol")
			imagewrite -c -k $k_ofs -l $k_sz /dev/mtd$mtd_no $from

		else

			# Upgrade for jffs2 to ubi flash layout

			if cat /proc/cmdline |grep -q "bank=low"; then

				# Running from low bank, need to springboard

				local newroot=/tmp/rootfs
				seqn=$(get_rootfs_sequence_number)

				v "Creating springboard fs..."
				mkdir -p $newroot
				build_springboard_rootfs \
					$newroot $(($seqn+1))
				copy_config_to_springboard $newroot
				create_springboard_preinit \
					$newroot/etc/preinit \
					/lib/upgrade/image.bin \
					$k_ofs $k_sz $ubifs_ofs $ubifs_sz
				update_sequence_number \
					$from $(($seqn+1)) $k_ofs $k_sz
				mv $from $newroot/lib/upgrade/image.bin
				make_nvram2_image \
					$newroot/lib/upgrade/nvram2.img

				echo "/$(cd $newroot; ls -1 cferam.*)" > /tmp/viplist
				echo "/vmlinux.lz" >> /tmp/viplist

				v "Writing springboard fs to high bank..."
				mtd_no=$(find_mtd_no "rootfs_update")
				mkfs.jffs2 \
					-e 128KiB --big-endian --squash \
					--no-cleanmarkers --pad \
					-N /tmp/viplist -S /tmp/viplist \
					-d $newroot | \
					imagewrite -c -i /dev/mtd$mtd_no

			else

				# Running from high bank, OK to upgrade directly

				mtd_no=$(find_mtd_no "rootfs_update")

				v "Write secondary nvram..."
				make_nvram2_image /tmp/nvram2.img
				imagewrite -s 0 -b 3 \
					   /dev/mtd$mtd_no /tmp/nvram2.img

				v "Wipe kernel areas..."
				imagewrite -s 3 -b 80 /dev/mtd$mtd_no

				v "Write UBI image..."
				imagewrite -s 83 -b 0 \
					   -k $ubifs_ofs -l $ubifs_sz \
					   --ubi -n 0 -S -20 --vol-name=rootfs_0 \
					   /dev/mtd$mtd_no $from

				v "Write kernel..."
				update_sequence_number $from 0 $k_ofs $k_sz
				imagewrite -c -s 3 -b 40 -k $k_ofs -l $k_sz \
					   /dev/mtd$mtd_no $from

			fi
		fi

	else
		echo "Unexpected image file contents, upgrade aborted!" >&2
	fi
}

default_do_upgrade() {
	sync
	local from mtd_no
	local cfe_fs
	local is_nand

	cfe_fs=$(cat /tmp/CFE_FS)
	is_nand=$(cat /tmp/IS_NAND)

	case "$1" in
		http://*|ftp://*) from=/tmp/firmware.bin;;
		*) from=$1;;
	esac

	if [ $cfe_fs -eq 1 ]; then
		mtd_cfe="$(grep "\"CFE\"" /proc/mtd | awk -F: '{print $1}')"
		if [ $is_nand -eq 0 ]; then
			v "Writing CFE ..."
			dd if=$from of=/tmp/cfe bs=64k count=1
			dd if=/dev/$mtd_cfe of=/tmp/cfe bs=1 count=1k skip=1408 seek=1408 conv=notrunc
			mtd write /tmp/cfe CFE
		fi
	fi

	if [ $is_nand -eq 1 ]; then

		v "Setting bootline parameter to boot from newly flashed image"
		brcm_fw_tool set -u 0
		v "Current Software Upgrade Count: $(get_rootfs_sequence_number)"

		if [ "$SAVE_CONFIG" -eq 1 -a -z "$USE_REFRESH" ]; then
			v "Creating save config file marker"
			touch /SAVE_CONFIG
			sync
		else
			v "Not saving config files"
			touch /SAVE_CONFIG
			rm /SAVE_CONFIG
			sync
		fi

		if [ $cfe_fs -eq 2 ]; then
			inteno_image_upgrade $from
		else
			# Old/Brcm format image
			if [ $cfe_fs -eq 1 ]; then
				v "Writing CFE ..."
				cfe_image_upgrade $from 0 131072
			fi

			v "Writing File System ..."
			mtd_no=$(find_mtd_no "rootfs_update")
			if [ $cfe_fs -eq 1 ]; then
				update_sequence_number $from 0 131072
				imagewrite -c -k 131072 /dev/mtd$mtd_no $from
			else
				update_sequence_number $from 0
				imagewrite -c /dev/mtd$mtd_no $from
			fi
		fi

		v "Upgrade completed!"
		rm -f $from
		[ -n "$DELAY" ] && sleep "$DELAY"
		v "Rebooting system ..."
		sync
		reboot -f
	else
		if [ "$SAVE_CONFIG" -eq 1 -a -z "$USE_REFRESH" ]; then
			v "Writing File System with Saved Config ..."
			if [ $cfe_fs -eq 1 ]; then
				mtd -j "$CONF_TAR" write $from -i 0x00010000 linux
			else
				mtd -j "$CONF_TAR" write $from linux
			fi
		else
			v "Writing File System ..."
			if [ $cfe_fs -eq 1 ]; then
				mtd write $from -i 0x00010000 linux
			else
				mtd write $from linux
			fi
		fi
	fi
}

do_upgrade() {
	v "Performing system upgrade ..."
	if type 'platform_do_upgrade' >/dev/null 2>/dev/null; then
		platform_do_upgrade "$ARGV"
	else
		default_do_upgrade "$ARGV"
	fi

	[ "$SAVE_CONFIG" -eq 1 -a -n "$USE_REFRESH" ] && {
		v "Refreshing partitions ..."
		if type 'platform_refresh_partitions' >/dev/null 2>/dev/null; then
			platform_refresh_partitions
		else
			refresh_mtd_partitions
		fi
		if type 'platform_copy_config' >/dev/null 2>/dev/null; then
			platform_copy_config
		else
			jffs2_copy_config
		fi
	}
	v "Upgrade completed"
	[ -n "$DELAY" ] && sleep "$DELAY"
	ask_bool 1 "Reboot" && {
		v "Rebooting system..."
		reboot -f
		sleep 5
		echo b 2>/dev/null >/proc/sysrq-trigger
	}
}
