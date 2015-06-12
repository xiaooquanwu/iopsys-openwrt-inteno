#!/bin/sh

do_brcmref() {
	. /lib/brcmref.sh
}

boot_hook_add preinit_main do_brcmref
