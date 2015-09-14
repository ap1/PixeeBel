#!/bin/bash

echo ADAFRUIT-SPI0 > /sys/devices/bone_capemgr.9/slots
./pb_visualize.py

