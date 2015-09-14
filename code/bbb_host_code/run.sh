#!/usr/bin/env sh

/sbin/modprobe uio_pruss
echo PyBBIO-ADC > /sys/devices/bone_capemgr.9/slots
echo pru_enable > /sys/devices/bone_capemgr.9/slots

while [ 1==1 ]; do
	echo "Starting host_main"
	./host_main

	echo "Restarting in 3 seconds"
	sleep 3s
done

