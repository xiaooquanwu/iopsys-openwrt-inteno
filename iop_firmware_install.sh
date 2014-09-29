scp $1 root@192.168.1.1:/tmp/firmware
ssh root@192.168.1.1 "sysupgrade $2 /tmp/firmware"