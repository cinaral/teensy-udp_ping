/*
 * teensy-pd_control
 *
 * MIT License
 *
 * Copyright (c) 2022 Cinar, A. L.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/*
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