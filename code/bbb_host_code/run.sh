#!/usr/bin/env sh

/sbin/modprobe uio_pruss
echo PyBBIO-ADC > /sys/devices/bone_capemgr.9/slots
echo pru_enable > /sys/devices/bone_capemgr.9/slots

./main

