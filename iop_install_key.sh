# this is a developer helper script to install the public ssh key in the created image
mkdir files/etc/dropbear
cat ~/.ssh/id_dsa.pub >>files/etc/dropbear/authorized_keys
cat ~/.ssh/id_rsa.pub >>files/etc/dropbear/authorized_keys

echo "::sysinit:/etc/init.d/rcS S boot" >files/etc/inittab
echo "::shutdown:/etc/init.d/rcS K shutdown" >>files/etc/inittab
echo "tty/0::askfirst:/bin/ash --login" >>files/etc/inittab
echo "ttyS0::askfirst:/bin/ash" >>files/etc/inittab

echo Done
