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
 * Neupokoev, N.V (Oct 5, 2021) https://stackoverflow.com/a/69445616
 */

#include <arpa/inet.h> //* htons, inet_addr
#include <chrono>
#include <iostream>
#include <netinet/in.h> //* sockaddr_in
#include <string>
#include <sys/socket.h> //* socket, sendto
#include <sys/types.h>  //* uint16_t
#include <unistd.h>     //* close
#include <cstdio>	   //* printf

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

union real_t
{
    float f;
    char b[sizeof(float)];
};


int elapsed_since(const std::chrono::time_point<std::chrono::high_resolution_clock> &start);

int
main(int, char const *[])
{
	//* start socket
	std::string hostname{"192.168.1.10"};
	uint16_t port = 5000;

	int sock = socket(AF_INET, SOCK_DGRAM, 0);

	sockaddr_in destination;
	destination.sin_family = AF_INET;
	destination.sin_port = htons(port);
	destination.sin_addr.s_addr = inet_addr(hostname.c_str());

	const int msg_size = 5;

	real_t val;
	val.f = M_PI;
	const char msg[msg_size] = {val.b[0], val.b[1], val.b[2], val.b[3], 'a'};
	char msg_recv[msg_size];
	socklen_t sendsize = sizeof(destination);

	//* benchmark stuff
	constexpr unsigned num_of_trials = 1e4;
	printf("pinging %u times...\n", num_of_trials);
	int send_elapsed;
	int recv_elapsed;
	int sum_send_elapsed = 0;
	int sum_recv_elapsed = 0;
	int min_send_elapsed = 1e6;
	int min_recv_elapsed = 1e6;
	int max_send_elapsed = 0;
	int max_recv_elapsed = 0;
	//*

	for (unsigned i = 0; i < num_of_trials; ++i) { //* benchmark loop
		{
			const auto start = std::chrono::high_resolution_clock::now();
			::sendto(sock, msg, msg_size, 0, reinterpret_cast<sockaddr *>(&destination), sendsize); //* send
			send_elapsed = elapsed_since(start);
		}
		{
			const auto start = std::chrono::high_resolution_clock::now();
			::recvfrom(sock, msg_recv, msg_size, 0, reinterpret_cast<sockaddr *>(&destination), &sendsize); //* receive
			recv_elapsed = elapsed_since(start);
		}
		//* benchmark stuff
		sum_send_elapsed += send_elapsed;
		sum_recv_elapsed += recv_elapsed;

		if (send_elapsed < min_send_elapsed) {
			min_send_elapsed = send_elapsed;
		}
		if (recv_elapsed < min_recv_elapsed) {
			min_recv_elapsed = recv_elapsed;
		}
		if (send_elapsed > max_send_elapsed) {
			max_send_elapsed = send_elapsed;
		}
		if (recv_elapsed > max_recv_elapsed) {
			max_recv_elapsed = recv_elapsed;
		}
		//*		
	}
	//* benchmark stuff
	printf("Done. In microseconds (us):\n");
	printf("send elapsed (min/max/mean): %9d / %9d / %9g (includes Teensy recv elapsed)\n", min_send_elapsed, max_send_elapsed, static_cast<double>(sum_send_elapsed) / num_of_trials);
	printf("recv elapsed (min/max/mean): %9d / %9d / %9g (includes Teensy send elapsed)\n", min_recv_elapsed, max_recv_elapsed, static_cast<double>(sum_recv_elapsed) / num_of_trials);
		  
	printf("", num_of_trials);
	real_t val_recv;
	val_recv.b[0] = msg_recv[0];
	val_recv.b[1] = msg_recv[1];
	val_recv.b[2] = msg_recv[2];
	val_recv.b[3] = msg_recv[3];
	char cmd =  msg_recv[4];
	printf("Last received package was: \"%c%g\"\n", cmd, val_recv.f);
	printf("\n");

	//* close socket
	::close(sock);

	return 0;
}

int
elapsed_since(const std::chrono::time_point<std::chrono::high_resolution_clock> &start)
{
	const auto end = std::chrono::high_resolution_clock::now();
	const auto diff = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
	return static_cast<int>(diff.count());
}
