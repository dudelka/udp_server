#include "my_socket.h"

#include <mutex>
#include <stdexcept>
#include <string>
#include <cstring>

#include <fcntl.h>
#include <unistd.h>
#include <arpa/inet.h>

UdpSocket::UdpSocket() {
	sock_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (sock_fd <= 0) {
		throw std::runtime_error("Failed to create socket\n");
	}
}

void UdpSocket::Bind(const uint16_t port) {
	address.sin_family = AF_INET;
	address.sin_port = htons(port);
	address.sin_addr.s_addr = htonl(INADDR_ANY);
	if (bind(sock_fd, (const sockaddr* )&address, sizeof(sockaddr_in)) < 0) {
		throw std::runtime_error("Couldn't bind socket to port: " + std::to_string(port) + "\n");
	}
}

void UdpSocket::SetToNonBlock() {
	if (fcntl(sock_fd, F_SETFL, O_NONBLOCK, 1) < 0) {
		throw std::runtime_error("Failed to set non-blocking socket\n");
	}
}

static std::mutex m;

bool UdpSocket::Send(
        UdpDatagramPackage* package, 
        size_t package_size, 
        const sockaddr_in& address
) const {
	if (!package) {
		throw std::runtime_error("Trying to accsess empty package\n");
	}
	package->Serialize();
    std::lock_guard<std::mutex> guard(m);
	int is_sent = sendto(sock_fd, (void* )package, package_size, 0, (sockaddr* )&address, sizeof(sockaddr_in));
	return (is_sent > 0) ? true : false;
}

std::pair<int, UdpDatagramPackage> UdpSocket::Receive() const {
	UdpDatagramPackage received_package;
    std::lock_guard<std::mutex> guard(m);
	int received_bytes = recvfrom(sock_fd, (void* )&received_package, sizeof(received_package), MSG_DONTWAIT, NULL, NULL);
	if (received_bytes > 0) {
		received_package.Deserialize();
	}
	return {received_bytes, received_package};
}

UdpSocket::~UdpSocket() {
	close(sock_fd);
}
