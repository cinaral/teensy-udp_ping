
# 1. teensy-udp_ping

**WARNING**: Do not use this library for your projects. This functionality is now part of [udp_msg](https://github.com/cinaral/udp_msg),  a slightly more refined library that is intended to make communicating between computers and Teensy easier.

This is a testing and benchmark project to make Raspberry Pi 4 B and Teensy 4.1 communicate over TCP/UDP. This document also outlines the process of preparing the hardware and other prerequisites.

Flash ```src/main.cpp``` to your Teensy 4.1, this project uses PlatformIO and it depends on [QNEthernet](https://github.com/ssilverman/QNEthernet).

You can build ```benchmark/udp_benchmark.cpp```, a simple program to benchmark UDP latency, by running ```scripts/build_benchmark.sh```. Note that the benchmark program depends on Unix standard libraries. If you wish, you can modify it to use Windows standard libraries, see [this answer](https://stackoverflow.com/a/24560310).

# 2. Table of Contents
- [1. teensy-udp_ping](#1-teensy-udpping)
- [2. Table of Contents](#2-table-of-contents)
- [3. Quick Start](#3-quick-start)
- [4. Example](#4-example)
- [5. Complete Guide to Setup Raspberry Pi 4 B to communicate with Teensy 4.1 over TCP/UDP](#5-complete-guide-to-setup-raspberry-pi-4-b-to-communicate-with-teensy-41-over-tcpudp)
	- [5.1. Summary](#51-summary)
	- [5.2. Hardware You Need](#52-hardware-you-need)
	- [5.3. Installation and Prerequisites](#53-installation-and-prerequisites)
		- [5.3.1. Prepare the Raspberry Pi](#531-prepare-the-raspberry-pi)
		- [5.3.2. Set up a DHCP server on the Raspberry Pi](#532-set-up-a-dhcp-server-on-the-raspberry-pi)
		- [5.3.3. Prepare the Teensy 4.1](#533-prepare-the-teensy-41)
		- [5.3.4. Flash the Teensy](#534-flash-the-teensy)
		- [5.3.5. Example](#535-example)
	- [5.4. [Optional] How flash Teensy from the CLI](#54-optional-how-flash-teensy-from-the-cli)

# 3. Quick Start
1. Compile the firmware for the Teensy 4.1 using VS Code, e.g. ```ctrl + shift + B``` and then select ```PlatformIO: Build (teensy 4.1)```
2. Sync to the Raspberry Pi using the [sync script](scripts/sync_to_rasppi.sh), see ```Sync to Raspberry Pi``` VS Code task for an example.
3. Now connect to the Raspberry Pi. From the Raspberry Pi flash the firmware using the [upload script](scripts/teensy_loader_cli/upload.sh).
4. Build the benchmark executable using the [build script](scripts/build/build.sh).
5. Run the benchmark executable to benchmark. 

# 4. Example
```bash
$ ./udp_benchmark

pinging 10000 times...
Done. In microseconds (us):
send elapsed (min/max/mean):        10 /       160 /   12.9498 (includes Teensy receive elapsed)
recv elapsed (min/max/mean):        35 /       260 /   48.2842 (includes Teensy send elapsed)
Last received package was: "a3.14159"
```

# 5. Complete Guide to Setup Raspberry Pi 4 B to communicate with Teensy 4.1 over TCP/UDP

## 5.1. Summary
1. Install an operating system on the RaspPi, and ```ssh``` into it.
2. Setup a DHCP server on the RaspPi.
3. Prepare Teensy 4.1 by soldering the ethernet kit and flashing your program, and connect it to the RaspPi using the RJ45 ports with an ethernet cable.

## 5.2. Hardware You Need
- Raspberry Pi 4 B
- Micro-SD card (min 16 GB)
- Teensy 4.1
- Computer with an SD card reader (comp)

## 5.3. Installation and Prerequisites
### 5.3.1. Prepare the Raspberry Pi 

1. Download and install [Raspberry Pi Imager](https://www.raspberrypi.com/software/).
```bash
# ~ $
apt install rpi-imager
```
2. Flash an operating system to the SD card. Set user name and password. Uncheck "Eject storage when done". I chose Raspberry Pi OS Lite 64-bit (no desktop environment, headless). 
3. Create two empty files named ```wpa_supplicant.conf``` and ```ssh``` on the root of the SD card (e.g. Drive /d).
```bash 
# /d $
touch wpa_supplicant.conf
touch ssh
```
4. Format the ```wpa_supplicant.conf``` based on your home network. This is to communicate with the RaspPi from the computer.
```bash
# wpa_supplicant.conf:
ctrl_interface=DIR=/var/run/wpa_supplicant GROUP=netdev
update_config=1
country=US # your country

network={
        ssid="HomeWifi" # your wifi name
        key_mgmt=WPA-PSK # your wifi protocol
        psk="wifipassword" # your wifi password
}
```
or,
```bash
# hint for connecting to university networks:
network={
        ssid="UniNet" # your university network name
        scan_ssid=1 # to find the access point if its name is not broadcasted
        key_mgmt=WPA-EAP # your protocol
        eap=PEAP
        identity="user" # your username
        password="password" # your password
        phase1="peaplabel=auto peapver=auto"
        phase2="auth=MSCHAPV2"
}
```
5. Insert the SD card to the RaspPi, power up and wait 30 seconds.
6. Figure out the assigned IP address of the RaspPi. You have some options:
	- If you own the network, you can type 198.168.1.1 (or similar, look up for your router/internet provider) into your browser to go to the router login page, login and look for a connection to a device called raspberrypi.
	- If you have a micro HDMI cable and a spare keyboard, you can connect the RaspPi to a monitor and look up the device IP (e.g. section ```wlan0```, line ```inet 192.168.0.108```).
	```bash
	# pi@pi ~ $
	ifconfig
	```
	- You can use ```nmap``` to search for devices on your network, you need to figure out which range your local network assigns IPs (e.g. 10.xxx or 192.168.xxx). This may take a long time:
	```bash
	# ~ $
	apt install nmap
	# scan range 192.168.0.0-192.168.0.255 and filter for rasp
	nmap -sP -n 192.168.0-255.0-255 | grep "rasp" 
	``
7. Once you find the IP address of the RaspPi (e.g. 192.168.3.14), you may now ```ssh``` into it from your computer to complete the headless setup. For Windows you can use [PuTTY](https://www.putty.org/). 
```bash
# ~ $
ssh pi@192.168.3.14 # replace with your RaspPi user name and IPv4 address
```

### 5.3.2. Set up a DHCP server on the Raspberry Pi 
For this part you need to ssh into the RaspPi as explained in the previous section. We need a DHCP server for the Teensy to be assigned an IP address on the subnet. We will not connect Teensy to the internet, instead we are creating a subnet that includes only the RaspPi and the Teensy.

1. Setup the ethernet interface:
```bash
# pi@pi ~ $
sudo touch /etc/network/interfaces.d/temp.conf # you can name it whatever you want, e.g. your username
sudo echo "auto eth0" >> /etc/network/interfaces.d/temp.conf
sudo echo "allow-hotplug eth0" >> /etc/network/interfaces.d/temp.conf
sudo echo "iface eth0 inet static" >> /etc/network/interfaces.d/temp.conf # static IP for the RaspPi ethernet port
sudo echo "	address 192.168.1.1" >> /etc/network/interfaces.d/temp.conf # can be anything, try to stick to 192.168.x.x
```
2. restart the network interface
```bash
# pi@pi ~ $
sudo systemctl restart NetworkManager.service
```
3. Install a DHCP server. I chose dnsmasq for this purpose.
```bash
# pi@pi ~ $
sudo apt install dnsmasq
```
4. Setup the DHCP server:

```bash
# pi@pi ~ $
sudo touch /etc/dnsmasq.d/pi.dns # you can name it whatever you want, e.g. your username (few exceptions, see /etc/dnsmasq.d/README).
sudo echo "interface = eth0" >> /etc/dnsmasq.d/pi.dns # we will DHCP ethernet 
sudo echo "no-dhcp-interface = wlan0" >> /etc/dnsmasq.d/pi.dns # we won't DHCP wifi
sudo echo "domain-needed" >> /etc/dnsmasq.d/pi.dns # prevents packets from leaving the subnet
sudo echo "bogus-priv" >> /etc/dnsmasq.d/pi.dns # limits DNS to subnet
sudo echo "domain=pi.lan" >> /etc/dnsmasq.d/pi.dns # you choose the name
sudo echo dhcp-range=192.168.1.10,192.168.1.100,48h >> /etc/dnsmasq.d/pi.dns # probably should not include interface address
sudo echo "dhcp-option=3,192.168.0.1" >> /etc/dnsmasq.d/pi.dns # sets the default route
```
5. Enable the DHCP server:
```bash
# pi@pi ~ $
sudo systemctl enable dnsmasq # Run at startup
sudo systemctl restart dnsmasq
sudo systemctl status dnsmasq # Confirm DHCP service is running, look for (Active)
```
### 5.3.3. Prepare the Teensy 4.1
1. [Solder](https://www.pjrc.com/store/ethernet_kit.html) the Ethernet kit.

### 5.3.4. Flash the Teensy
1. We will use the [QNEthernet](https://github.com/ssilverman/QNEthernet) library for Teensy 4.1. I use PlatformIO on VS Code to build for the Teensy, and you can simply add QNEthernet to your project via PlatformIO.
2. Download an [example](https://github.com/ssilverman/QNEthernet/blob/master/examples/FixedWidthServer/FixedWidthServer.ino) program or provide your own program.
3. Build and copy ```firmware.hex``` to the RaspPi (You could also flash the Teensy from the PlatformIO by connecting it to the computer and stop here):
```bash
# ~ $
scp ./.pio/build/teensy41/firmware.hex pi@192.168.3.14:/home/pi
```
3. Flash the Teensy remotely from the RaspPi:
```bash
# pi@pi ~ $
./teensy_loader_cli/teensy_loader_cli --mcu=TEENSY41 -w firmware.hex # waits for the physical button press on the Teensy
# or
/teensy_loader_cli/teensy_loader_cli --mcu=TEENSY41 -s --serial-number=11462940 # use usb-devices to find the SN of the Teensy and identify the Teensy you want to flash
```

### 5.3.5. Example
1. We will use the same [example program](https://github.com/ssilverman/QNEthernet/blob/master/examples/FixedWidthServer/FixedWidthServer.ino) from the [previous section](#534-flash-the-teensy).
1. Physically connect the RaspPi and the Teensy via the RJ45 ports.
2. Install netcat on the RaspPi:
```bash
# pi@pi ~ $
sudo apt install netcat
```
3. Monitor the serial:
```bash
# pi@pi ~ $
cat /dev/ttyACM0 # or whichever port Teensy is connected to
```
4. Send a TCP packet to Teensy's IP and the specified port in the program: 
```bash
# pi@pi ~ $
nc 192.168.1.41 5000 # replace with your Teensy's IP and port
helloworld # fixed-width example is fixed-width
```

## 5.4. [Optional] How flash Teensy from the CLI

You can use the Raspberry Pi on the local network to flash Teensy's remotely. For usage see [upload_to_TEENSY41.sh](./scripts/upload_to_TEENSY41.sh).

1. Add Teensy rules:
```bash
# pi@pi ~ $
wget https://www.pjrc.com/teensy/00-teensy.rules
sudo mv 00-teensy.rules /etc/udev/rules.d/00-teensy.rules 
```

2. Install libusb-dev:
```bash
# pi@pi ~ $
sudo apt install libusb-dev
```

3. Install and build teensy_loader_cli
```bash
pi@pi ~ $
git clone https://github.com/PaulStoffregen/teensy_loader_cli
cd teensy_loader_cli
git remote add serial_number https://github.com/hmaarrfk/teensy_loader_cli # add serial_number feature to teensy_loader_cli, useful for multiple Teensy's (Not part of the master as of 2022-10-21) 
git fetch serial_number
git cherry-pick 93781a0dd9f7060b7bb5efaac99c977bbf768711
make # build
```

## Acknowledgements

This work was supported by the National Science Foundation DCSD Grant (No. 2029181).
