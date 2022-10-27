#!/bin/bash

cp .pio/build/teensy41/firmware.hex ./
rsync -av --exclude='.*' --exclude='*.ini' ./ beryl@10.66.203.90:/home/beryl/teensy-udp_ping/
rm firmware.hex
