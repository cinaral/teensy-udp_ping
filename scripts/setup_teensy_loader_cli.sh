#!/bin/bash

#* add teensy rules
wget https://www.pjrc.com/teensy/00-teensy.rules
sudo cp 00-teensy.rules /etc/udev/rules.d/00-teensy.rules
rm 00-teensy.rules

#* install libusb-dev
sudo apt install libusb-dev

#* clone teensy_loader_cli
git clone https://github.com/PaulStoffregen/teensy_loader_cli
cd teensy_loader_cli
#* add serial_number feature to teensy_loader_cli
git remote add serial_number https://github.com/hmaarrfk/teensy_loader_cli
git fetch serial_number
git cherry-pick 93781a0dd9f7060b7bb5efaac99c977bbf768711
make