#!/bin/bash

default_file=firmware.hex

echo "Usage: upload_to_TEENSY41.sh <SerialNumber (Hint: usb-devices)> <*.hex (Default: $default_file)>"
if [ "$#" -lt 1 ]; then
	echo "Uploading $default_file..."
	echo "No serial number provided, press the physical button of the Teensy 4.1 you want to flash to continue..."
	~/teensy_loader_cli/teensy_loader_cli --mcu=TEENSY41 -w $default_file
elif [ "$#" -lt 2 ]; then
	echo "Uploading $default_file to SerialNumber=$1..."
	~/teensy_loader_cli/teensy_loader_cli --mcu=TEENSY41 -s --serial-number=$1 $default_file
else
	echo "Uploading $2 to SerialNumber=$1..."
	~/teensy_loader_cli/teensy_loader_cli --mcu=TEENSY41 -s --serial-number=$1 $2
fi
echo "Done."
