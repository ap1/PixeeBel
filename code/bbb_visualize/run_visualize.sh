#!/bin/bash

echo ADAFRUIT-SPI0 > /sys/devices/bone_capemgr.9/slots

while [ 1==1 ]; do
	echo "Starting pb_visualize.py"
	./pb_visualize.py
	sleep 3
done

