/*
 * teensy-pd_control
 * 2022 cinaral
 *
 * This file incorporates work by:
 * SPDX-FileCopyrightText: (c) 2021-2022 Shawn Silverman <shawn@pobox.com>
 * SPDX-License-Identifier: MIT
 */

#include <Arduino.h>
#include <QNEthernet.h>

constexpr uint32_t dhcp_timeout = 10000; //* 10 seconds
constexpr uint16_t listening_port = 5000;
constexpr int msg_size = 6; //* Pretend the protocol specifies 10 bytes
char packet_buffer[msg_size]; //* buffer to hold incoming packet,

qindesign::network::EthernetUDP server;

void
setup()
{
	Serial.begin(115200);

	while (!Serial && millis() < 4000) { //* Wait for Serial to initialize
	}
	qindesign::network::stdPrint = &Serial; //* Make printf work (a QNEthernet feature)

	uint8_t mac[6];
	qindesign::network::Ethernet.macAddress(mac); //* This is informative; it retrieves, not sets
	printf("MAC = %02x:%02x:%02x:%02x:%02x:%02x\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	printf("Starting Ethernet with DHCP...\n");

	if (!qindesign::network::Ethernet.begin()) {
		printf("Failed to start Ethernet\n");
		return;
	}

	if (!qindesign::network::Ethernet.waitForLocalIP(dhcp_timeout)) {
		printf("Failed to get IP address from DHCP\n");
	} else {
		IPAddress ip = qindesign::network::Ethernet.localIP();
		printf("    Local IP    = %u.%u.%u.%u\n", ip[0], ip[1], ip[2], ip[3]);
		ip = qindesign::network::Ethernet.subnetMask();
		printf("    Subnet mask = %u.%u.%u.%u\n", ip[0], ip[1], ip[2], ip[3]);
		ip = qindesign::network::Ethernet.gatewayIP();
		printf("    Gateway     = %u.%u.%u.%u\n", ip[0], ip[1], ip[2], ip[3]);
		ip = qindesign::network::Ethernet.dnsServerIP();
		printf("    DNS         = %u.%u.%u.%u\n", ip[0], ip[1], ip[2], ip[3]);
		printf("Listening for clients on port %u...\n", listening_port);
		server.begin(listening_port);
	}
}

void
loop()
{
	int packet_size = server.parsePacket();

	if (packet_size) {
		//* receive a packet over UDP
		server.read(packet_buffer, msg_size);
		//* and then send back
		server.beginPacket(server.remoteIP(), server.remotePort());
		server.write(packet_buffer);
		server.endPacket();
	}
}