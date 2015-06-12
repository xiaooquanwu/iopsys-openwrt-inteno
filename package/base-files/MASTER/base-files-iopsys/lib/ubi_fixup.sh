#!/bin/sh
# (C) 2015 Inteno Broadband Technology AB


find_mtd_no() {
	local part=$(awk -F: "/\"$1\"/ { print \$1 }" /proc/mtd)
	echo ${part##mtd}
}


umount /old_root

mount proc /proc -t proc
mount sysfs /sys -t sysfs

ubi_mtd=$(find_mtd_no ubi)
lo_mtd=$(find_mtd_no mtd_lo)
hi_mtd=$(find_mtd_no mtd_hi)

ubidetach /dev/ubi_ctrl -m $lo_mtd
imagewrite /dev/mtd$hi_mtd
ubiattach /dev/ubi_ctrl -m $ubi_mtd

lo_ebs=$(ubinfo /dev/ubi0_0 |awk '/^Size/ { print $2 }')
avail_ebs=$(ubinfo /dev/ubi0 |awk '/^Amount/ { print $6 }')
new_size=$(( ($lo_ebs + $avail_ebs) / 2 ))
echo "lo_ebs=$lo_ebs, avail_ebs=$avail_ebs, new_size=$new_size"

ubirsvol /dev/ubi0 -N rootfs_0 -S $new_size
ubimkvol /dev/ubi0 -n 1 -N rootfs_1 -m

# Mark repartitioning as completed
echo 04 >/proc/nvram/iVersion

umount /proc
umount /sys

# Always rootfs_0 on FIRST upgrade to new layout
mount -t ubifs -o rw,noatime ubi:rootfs_0 /old_root

echo "====== Restart normal boot ======"
cd /old_root
pivot_root . /old_root/tmp
exec env -i /usr/sbin/chroot . /etc/preinit noinitrd &> dev/kmsg

