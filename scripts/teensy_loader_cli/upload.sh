#!/bin/bash

#* #* project's relative path with respect to this script
PROJECT_PATH=../..

#* modify this line for your Teensy 4.1 devices
SerialNumbers="11463280 11462940"

#*  change the cwd to the script dir temporarily, and hide pushd popd output
pushd () { 
	command pushd "$@" > /dev/null 
}
popd () { 
	command popd "$@" > /dev/null 
}
pushd "$(dirname ${BASH_SOURCE:0})"
trap popd EXIT #*

USB_DEVICES=$(usb-devices)

for val in $SerialNumbers; do
	if [[ $USB_DEVICES == *$val* ]]; then
		echo "Uploading to Serial Number $val..."
		bash ./upload_to_TEENSY41.sh $val $PROJECT_PATH/.pio/build/teensy41/firmware.hex
	fi
done
echo "$0 done."

