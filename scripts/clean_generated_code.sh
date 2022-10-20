#!/bin/bash

DIR="./lib/fn_PD_controller/interface"

#* remove build/ except build/_deps
if [ -d "$DIR" ]; then
rm -r ${DIR}
if [ $? -eq 0 ]; then
   echo "Removed ${DIR}"
fi
fi

