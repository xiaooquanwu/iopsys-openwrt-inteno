#!/bin/sh

bcm_dsl_annex() {
        ANNEX=`cat /proc/nvram/dslAnnex`
#       echo $ANNEX

        if [ -f "/etc/adsl/adsl_phy.bin" ]
        then
                echo "DSL firmware symlink set"
        else
                if [ "$ANNEX" = "A" ]; then
                        echo "DSL Annex A detected"
                        ln -s /etc/dsl/a_adsl_phy.bin /etc/adsl/adsl_phy.bin
                elif [ "$ANNEX" = "B" ]; then
                        echo "DSL Annex B detected"
                        ln -s /etc/dsl/b_adsl_phy.bin /etc/adsl/adsl_phy.bin
                else
                        echo "DSL Annex A default"
                        ln -s /etc/dsl/a_adsl_phy.bin /etc/adsl/adsl_phy.bin
                fi
        fi
}

brcm_insmod() {
	echo Loading brcm modules
        test -e /lib/modules/3.4.11-rt19/extra/chipinfo.ko     && insmod /lib/modules/3.4.11-rt19/extra/chipinfo.ko
	test -e /lib/modules/3.4.11-rt19/extra/bcmxtmrtdrv.ko  && insmod /lib/modules/3.4.11-rt19/extra/bcmxtmrtdrv.ko
        test -e /lib/modules/3.4.11-rt19/extra/bcm_ingqos.ko   && insmod /lib/modules/3.4.11-rt19/extra/bcm_ingqos.ko
	test -e /lib/modules/3.4.11-rt19/extra/bcm_bpm.ko      && insmod /lib/modules/3.4.11-rt19/extra/bcm_bpm.ko
	test -e /lib/modules/3.4.11-rt19/extra/pktflow.ko      && insmod /lib/modules/3.4.11-rt19/extra/pktflow.ko
	test -e /lib/modules/3.4.11-rt19/extra/pktcmf.ko       && insmod /lib/modules/3.4.11-rt19/extra/pktcmf.ko
	test -e /lib/modules/3.4.11-rt19/extra/bcmfap.ko       && insmod /lib/modules/3.4.11-rt19/extra/bcmfap.ko
	test -e /lib/modules/3.4.11-rt19/extra/bcmxtmcfg.ko    && insmod /lib/modules/3.4.11-rt19/extra/bcmxtmcfg.ko
	test -e /lib/modules/3.4.11-rt19/extra/adsldd.ko       && insmod /lib/modules/3.4.11-rt19/extra/adsldd.ko
	test -e /lib/modules/3.4.11-rt19/extra/i2c_bcm6xxx.ko  && insmod /lib/modules/3.4.11-rt19/extra/i2c_bcm6xxx.ko
	test -e /lib/modules/3.4.11-rt19/extra/bcm_enet.ko     && insmod /lib/modules/3.4.11-rt19/extra/bcm_enet.ko
	test -e /lib/modules/3.4.11-rt19/extra/nciTMSkmod.ko   && insmod /lib/modules/3.4.11-rt19/extra/nciTMSkmod.ko
	test -e /lib/modules/3.4.11-rt19/extra/bcmsw.ko        && insmod /lib/modules/3.4.11-rt19/extra/bcmsw.ko && ifconfig bcmsw up
#	test -e /lib/modules/3.4.11-rt19/extra/bcm_usb.ko      && insmod /lib/modules/3.4.11-rt19/extra/bcm_usb.ko
	sleep 1
	test -e /lib/modules/3.4.11-rt19/extra/bcmarl.ko       && insmod /lib/modules/3.4.11-rt19/extra/bcmarl.ko
#	/usr/bin/taskset 2>/dev/null
	test -e /lib/modules/3.4.11-rt19/extra/wfd.ko          && insmod /lib/modules/3.4.11-rt19/extra/wfd.ko
	test -e /lib/modules/3.4.11-rt19/extra/wlcsm.ko        && insmod /lib/modules/3.4.11-rt19/extra/wlcsm.ko
	test -e /lib/modules/3.4.11-rt19/extra/wlemf.ko        && insmod /lib/modules/3.4.11-rt19/extra/wlemf.ko
	test -e /lib/modules/3.4.11-rt19/extra/dhd.ko          && insmod /lib/modules/3.4.11-rt19/extra/dhd.ko  firmware_path=/etc/wlan/dhd mfg_firmware_path=/etc/wlan/dhd/mfg
	test -e /lib/modules/3.4.11-rt19/extra/wl.ko           && insmod /lib/modules/3.4.11-rt19/extra/wl.ko
	test -e /lib/modules/3.4.11-rt19/extra/dect.ko         && insmod /lib/modules/3.4.11-rt19/extra/dect.ko
	test -e /lib/modules/3.4.11-rt19/extra/dectshim.ko     && insmod /lib/modules/3.4.11-rt19/extra/dectshim.ko
	test -e /lib/modules/3.4.11-rt19/extra/pcmshim.ko      && insmod /lib/modules/3.4.11-rt19/extra/pcmshim.ko
	test -e /lib/modules/3.4.11-rt19/extra/endpointdd.ko   && insmod /lib/modules/3.4.11-rt19/extra/endpointdd.ko
	test -e /lib/modules/3.4.11-rt19/extra/p8021ag.ko      && insmod /lib/modules/3.4.11-rt19/extra/p8021ag.ko
	test -e /lib/modules/3.4.11-rt19/extra/bcmvlan.ko      && insmod /lib/modules/3.4.11-rt19/extra/bcmvlan.ko
	test -e /lib/modules/3.4.11-rt19/extra/pwrmngtd.ko     && insmod /lib/modules/3.4.11-rt19/extra/pwrmngtd.ko
	test -e /lib/modules/3.4.11-rt19/rng-core.ko           && insmod /lib/modules/3.4.11-rt19/rng-core.ko
	test -e /lib/modules/3.4.11-rt19/extra/bcmtrng.ko      && insmod /lib/modules/3.4.11-rt19/extra/bcmtrng.ko
	echo brcm modules loaded
}

brcm_env() {
	echo "Y" > /sys/module/printk/parameters/time
	echo Setting up brcm environment
	/bin/mount -a
	echo "Copying device files from /lib/dev to /dev"
	cp -a /lib/dev/* /dev
	mknod /var/fuse c 10 229
	chmod a+rw /var/fuse
	mkdir -p /var/log /var/run /var/state/dhcp /var/ppp /var/udhcpd /var/zebra /var/siproxd /var/cache /var/tmp /var/samba /var/samba/share /var/samba/homes /var/samba/private /var/samba/locks
}

id_upgrade_reconfig() {

    local basemac=$(cat /proc/nvram/BaseMacAddr | tr '[a-z]' '[A-Z]')
    local boardid=$(cat /proc/nvram/BoardId)
    local vendorid=${basemac:0:8}
    if [ "$boardid" == "963268BU" ]; then
        if [ "$vendorid" == "00 22 07" ]; then
            echo "Setting new boardid and voiceboardid"
            echo DG301R0 > /proc/nvram/BoardId
            echo SI32176X2 > /proc/nvram/VoiceBoardId
            echo "00 00 00 01" >/proc/nvram/ulBoardStuffOption
            sync
            sleep 3
            /sbin/brcm_fw_tool set -x 17 -p 0
        fi
    fi
}

check_num_mac_address() {
    local nummac=$(cat /proc/nvram/NumMacAddrs)
    local boardid=$(cat /proc/nvram/BoardId)
    if [ "$boardid" != "CG300R0" ]; then
        if [ "$nummac" != "00 00 00 08 " ]; then
            echo "Setting NumMacAddrs to 8"
            echo "00 00 00 08" >/proc/nvram/NumMacAddrs
            sync
        fi
    fi
}

brcm_env
id_upgrade_reconfig
bcm_dsl_annex
check_num_mac_address
brcm_insmod

