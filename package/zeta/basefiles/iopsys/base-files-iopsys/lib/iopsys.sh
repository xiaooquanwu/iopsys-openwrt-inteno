#!/bin/sh
# (C) 2015 Inteno Broadband Technology AB

mk_mtd_devnode() {
	local part=$(awk -F: "/\"$1\"/ { print \$1 }" /proc/mtd)
	if [ -z "$part" ]; then
		echo notfound
		return
	fi

	local mtd_no=${part##mtd}
	local dev_node=/dev/mtdblock$mtd_no

	[ -e $dev_node ] || mknod -m 644 $dev_node b 31 $mtd_no

	echo $dev_node
}

copy_old_config() {

	local iVersion=$1
	local new_fs_type=$2
	local old_vol old_fs_mtd

	if [ "$iVersion" == "04 " -a "$new_fs_type" == "ubifs" ]; then

		# ubifs -> ubifs upgrade
		echo "Upgrading $new_fs_type from iVersion 4"

		if cat /proc/cmdline |grep -q "ubi:rootfs_0"; then
			old_vol="ubi:rootfs_1"
		else
			old_vol="ubi:rootfs_0"
		fi

		mount -t ubifs -o ro,noatime $old_vol /mnt
		if [ -e /mnt/overlay/SAVE_CONFIG ]; then
			echo "Copying overlay..."
			cp -rfdp /mnt/overlay/* /overlay/
			rm /overlay/SAVE_CONFIG
		fi
		umount /mnt

	elif [ "$iVersion" == "03 " ]; then

		# jffs2 -> jffs2/ubifs upgrade
		echo "Upgrading $new_fs_type from iVersion 3"

		if [ "$new_fs_type" == "jffs2" ]; then
			old_fs_mtd=$(mk_mtd_devnode rootfs_update)
		else
			old_fs_mtd=$(mk_mtd_devnode mtd_hi)
		fi

		mount -t jffs2 $old_fs_mtd /mnt
		if [ -e /mnt/overlay/SAVE_CONFIG ]; then
			if [ "$new_fs_type" == "ubifs" ]; then
				echo "Copying etc..."
				cp -rfdp /mnt/overlay/etc /overlay/
			else
				echo "Copying overlay..."
			cp -rfdp /mnt/overlay/* /overlay/
			fi
			rm -f /overlay/SAVE_CONFIG
		fi
		umount /mnt

	else
		echo "Unexpected iVersion \"$iVersion\" in nvram, config lost"
		echo 03 > /proc/nvram/iVersion
	fi

	# remove db to trigger init
	rm -f /overlay/lib/db/config/hw
}

build_minimal_rootfs() {

	cd $1

	mkdir bin
	cp /bin/busybox bin
	cp -d /bin/ash bin
	cp -d /bin/mount bin
	cp -d /bin/sh bin
	cp -d /bin/umount bin

	local ubi_ctrl_minor=$(awk -F= '/MINOR/ {print $2}' \
				/sys/devices/virtual/misc/ubi_ctrl/uevent)
	mkdir dev
	mknod -m 644 dev/kmsg     c   1 11
	mknod -m 644 dev/mtd0     c  90  0
	mknod -m 644 dev/mtd1     c  90  2
	mknod -m 644 dev/mtd2     c  90  4
	mknod -m 644 dev/mtd3     c  90  6
	mknod -m 644 dev/mtd4     c  90  8
	mknod -m 644 dev/mtd5     c  90 10
	mknod -m 644 dev/mtd6     c  90 12
	mknod -m 644 dev/ubi_ctrl c  10 $ubi_ctrl_minor
	mknod -m 644 dev/ubi0     c 254  0
	mknod -m 644 dev/ubi0_0   c 254  1
	mknod -m 644 dev/ubi0_1   c 254  2
	mknod -m 644 dev/ubi0_2   c 254  3

	mkdir lib
	cp /lib/ubi_fixup.sh lib
	cp -d /lib/libcrypt.so.0 lib
	cp /lib/libcrypt-*.so lib
	cp -d /lib/libm.so.0 lib
	cp /lib/libm-*.so lib
	cp /lib/libgcc_s.so.1 lib
	cp -d /lib/libc.so.0 lib
	cp /lib/libuClibc-*.so lib
	cp -d /lib/ld-uClibc.so.0 lib
	cp /lib/ld-uClibc-*.so lib

	mkdir old_root
	mkdir proc

	mkdir sbin
	cp -d /sbin/pivot_root sbin

	mkdir sys
	mkdir tmp
	mkdir usr

	mkdir usr/bin
	cp -d /usr/bin/awk usr/bin
	cp -d /usr/bin/env usr/bin

	mkdir usr/sbin
	cp -d /usr/sbin/chroot usr/sbin
	cp /usr/sbin/imagewrite usr/sbin
	cp /usr/sbin/ubiattach usr/sbin
	cp /usr/sbin/ubidetach usr/sbin
	cp /usr/sbin/ubimkvol usr/sbin
	cp /usr/sbin/ubinfo usr/sbin
	cp /usr/sbin/ubirsvol usr/sbin

	cd -
}

# iopsys_upgrade_handling
# This function needs to handle the following cases:
# - normal boot, no upgrade
# - first boot after upgrade jffs2->jffs2
# - first boot after upgrade jffs2->ubifs
# - restarted init after upgrade jffs2->ubifs
# - first boot after upgrade ubifs->ubifs
#
iopsys_upgrade_handling() {

	# Skip if not first boot
	[ -e /IOP3 ] || return

	mount proc /proc -t proc

	if cat /proc/mounts | grep -q '/tmp tmpfs'; then
		# preinit restart after upgrade jffs2 -> ubifs
		umount /tmp
		umount /proc
		rm /IOP3
		return
	fi

	local iVersion=$(cat /proc/nvram/iVersion)
	local fs_type=$(cat /proc/mounts |awk '/jffs2|ubifs/ {print $3}')
	copy_old_config "$iVersion" "$fs_type"

	if [ "$iVersion" == "04 " -o "$fs_type" == "jffs2" ]; then
		# upgrading ubifs -> ubifs or jffs2 -> jffs2
		umount /proc
		rm /IOP3
		return
	fi

	# upgrading jffs2 -> ubifs
	mount sysfs /sys -t sysfs

	echo "====== Start flash partition update ======"

	mount tmpfs /tmp -t tmpfs -o size=100M,mode=0755
	build_minimal_rootfs /tmp

	umount /sys
	umount /proc

	cd /tmp
	pivot_root . /tmp/old_root
	exec chroot . /lib/ubi_fixup.sh &> dev/kmsg

	# Never returns here, ubi_fixup.sh will respawn /etc/preinit
}


