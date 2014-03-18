# this is a developer helper script to install the public ssh key in the created image
mkdir files/etc/dropbear
cat ~/.ssh/id_dsa.pub >>files/etc/dropbear/authorized_keys
cat ~/.ssh/id_rsa.pub >>files/etc/dropbear/authorized_keys
echo Done
