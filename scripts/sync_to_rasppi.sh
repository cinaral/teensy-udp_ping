#!/bin/bash

#* project's relative path with respect to this script
PROJECT_PATH=".."

echo "Usage: copy_to_rasppi.sh <TargetFolderPath (Example: usr@192.168.1.1:~/project)>"

#*  change the cwd to the script dir temporarily, and hide pushd popd output
pushd () { 
	command pushd "$@" > /dev/null 
}
popd () { 
	command popd "$@" > /dev/null 
}
pushd "$(dirname ${BASH_SOURCE:0})"
trap popd EXIT #*

#* get current platform
UNAME=$(uname)

#* cd to project path
pushd $PROJECT_PATH
if [ "$#" -lt 1 ]; then
	echo "No target folder path provided!"
else
	echo "Syncing to $1..."
	rsync -aR \
		.pio/build/teensy41/firmware.hex \
		README.md \
		scripts/teensy_loader_cli/ \
		scripts/build/build.sh \
		benchmark/ \
		$1/
fi
popd

echo "Done."
