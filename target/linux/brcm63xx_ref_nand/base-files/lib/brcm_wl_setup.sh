#wl config script

# take down interface to default off state
wl -a wl1 bss down
wl -a wl1 down


#enable 20/40/80Mhz bw on 5g band
wl bw_cap 5g 0x7

#configure wlan1 to 5g channel 40 with 80 MHz bw
wl -a wl1 chanspec 5g36/80

#setup wlan1 channel 36
wl -a wl1 channel 36

wl -a wl1 bss down
wl -a wl1 ap 1

wl -a wl1 ssid ac_test
wl -a wl1 bss up
wl -a wl1 up

