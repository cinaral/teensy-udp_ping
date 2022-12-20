#!/bin/bash

#* project's relative path with respect to this script
PROJECT_PATH=".."
BUILD_DIR="$PROJECT_PATH"

#*  change the cwd to the script dir temporarily, and hide pushd popd output
pushd () { 
	command pushd "$@" > /dev/null 
}
popd () { 
	command popd "$@" > /dev/null 
}
pushd "$(dirname ${BASH_SOURCE:0})"
trap popd EXIT #*

#* cd to project path
pushd $PROJECT_PATH

if ! [[ -d "$BUILD_DIR" ]]; then
	mkdir $BUILD_DIR
fi

#* cd into project directory
pushd $PROJECT_PATH
trap popd EXIT #*

#* get current platform
UNAME=$(uname)

#* call g++
if [[ $UNAME == "Linux" ]] ; then
	g++ -O3 -o udp_benchmark benchmark/udp_benchmark.cpp
else 
	echo "BUILD FAILED: Unsupported platform!

	This project uses \"arpa/inet.h\" which is not available on this platform.
		This can be remedied in a few ways:
		1. Find a substitute (Not Recommended).
		2. Compile somewhere else, e.g. on a virtual machine.
		"
fi

trap popd EXIT 

echo "$0 done."
