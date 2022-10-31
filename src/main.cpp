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

namespace qn = qindesign::network;

constexpr uint32_t dhcp_timeout = 10000; //* 10 seconds
constexpr uint16_t listening_port = 5000;
constexpr int msg_size = 5; //* Pretend the protocol specifies 10 bytes
char packet_buffer[msg_size]; //* buffer to hold incoming packet,

qn::EthernetUDP server;

void
setup()
{
	while (!Serial && millis() < 4000) { //* Wait for Serial to initialize
	}
	qn::stdPrint = &Serial; //* Make printf work (a QNEthernet feature)

	if (!qn::Ethernet.begin()) {
		printf("Failed to start Ethernet\n");
		return;
	}

	if (!qn::Ethernet.waitForLocalIP(dhcp_timeout)) {
		printf("Failed to get IP address from DHCP\n");
	} else {
		IPAddress ip = qn::Ethernet.localIP();
		printf("    Local IP    = %u.%u.%u.%u\n", ip[0], ip[1], ip[2], ip[3]);
		ip = qn::Ethernet.subnetMask();
		printf("    Subnet mask = %u.%u.%u.%u\n", ip[0], ip[1], ip[2], ip[3]);
		ip = qn::Ethernet.gatewayIP();
		printf("    Gateway     = %u.%u.%u.%u\n", ip[0], ip[1], ip[2], ip[3]);
		ip = qn::Ethernet.dnsServerIP();
		printf("    DNS         = %u.%u.%u.%u\n", ip[0], ip[1], ip[2], ip[3]);
		printf("Listening for clients on port %u...\n", listening_port);
		
		server.begin(listening_port);
	}
}

void
loop()
{
	int packet_size = server.parsePacket();

	if (packet_size >= 0) {	
		server.read(packet_buffer, msg_size); //* receive a packet over UDP
		server.send(server.remoteIP(), server.remotePort(), (uint8_t*)packet_buffer, msg_size); //* and then send back
	}
}