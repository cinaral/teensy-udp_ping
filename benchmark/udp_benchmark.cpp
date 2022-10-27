/*
 * teensy-pd_control
 * 2022 cinaral
 *
 * This file incorporates work from: https://stackoverflow.com/a/69445616
 */

#include <arpa/inet.h> //* htons, inet_addr
#include <chrono>
#include <iostream>
#include <netinet/in.h> //* sockaddr_in
#include <string>
#include <sys/socket.h> //* socket, sendto
#include <sys/types.h>  //* uint16_t
#include <unistd.h>     //* close

int elapsed_since(const std::chrono::time_point<std::chrono::high_resolution_clock> &start);

int
main(int, char const *[])
{
	std::string hostname{"192.168.1.75"};
	uint16_t port = 5000;

	int sock = socket(AF_INET, SOCK_DGRAM, 0);

	sockaddr_in destination;
	destination.sin_family = AF_INET;
	destination.sin_port = htons(port);
	destination.sin_addr.s_addr = inet_addr(hostname.c_str());

	const unsigned int msg_size = 1;
	const char msg_sent[msg_size] = {'a'};
	char msg_recv[msg_size];
	socklen_t sendsize = sizeof(destination);

	constexpr unsigned int num_of_trials = 100;

	int send_elapsed;
	int recv_elapsed;
	int sum_send_elapsed = 0;
	int sum_recv_elapsed = 0;
	int min_send_elapsed = 1e6;
	int min_recv_elapsed = 1e6;
	int max_send_elapsed = 0;
	int max_recv_elapsed = 0;

	for (unsigned int i = 0; i < num_of_trials; ++i) {
		{
			const auto start = std::chrono::high_resolution_clock::now();
			::sendto(sock, msg_sent, msg_size, 0, reinterpret_cast<sockaddr *>(&destination), sendsize);
			send_elapsed = elapsed_since(start);
		}
		{
			const auto start = std::chrono::high_resolution_clock::now();
			::recvfrom(sock, msg_recv, msg_size, 0, reinterpret_cast<sockaddr *>(&destination), &sendsize);
			recv_elapsed = elapsed_since(start);
		}

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
	}
	std::cout << "send elapsed (us) (min/max/mean): " << min_send_elapsed << " / " << max_send_elapsed << " / "
		  << static_cast<double>(sum_send_elapsed) / num_of_trials << std::endl;
	std::cout << "recv elapsed (us) (min/max/mean): " << min_recv_elapsed << " / " << max_recv_elapsed << " / "
		  << static_cast<double>(sum_recv_elapsed) / num_of_trials << std::endl;
	std::cout << msg_recv[0] << std::endl;

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
