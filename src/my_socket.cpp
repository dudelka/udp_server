#include "my_socket.h"

#include <stdexcept>
#include <string>
#include <cstring>

#include <fcntl.h>
#include <unistd.h>
#include <arpa/inet.h>

void UdpDatagramPackage::Serialize() {
	seq_number = htonl(seq_number);
	seq_total = htonl(seq_total);
	type = htonl(type);
}

void UdpDatagramPackage::Deserialize() {	
	seq_number = ntohl(seq_number);
	seq_total = ntohl(seq_total);
	type = ntohl(type);
}

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

bool UdpSocket::Send(uint32_t seq_number, uint32_t seq_total, uint8_t type,
		unsigned char* id, const std::vector<unsigned char>& data, const sockaddr_in& address) const {
	UdpDatagramPackage package = {.seq_number = seq_number, .seq_total = seq_total, .type = type};
	std::memcpy(&package.id, id, sizeof(id));
	std::memcpy(&package.data, data.data(), data.size());
	size_t package_size = 0;
	package_size += sizeof(package.seq_number);
	package_size += sizeof(package.seq_total);
	package_size += sizeof(package.type);
	package_size += sizeof(package.id);
	package_size += data.size();
	package.Serialize();
	int is_sent = sendto(sock_fd, (void* )&package, package_size, 0, (sockaddr* )&address, sizeof(sockaddr_in));
	return (is_sent > 0) ? true : false;
}

std::pair<int, UdpDatagramPackage> UdpSocket::Receive() const {
	UdpDatagramPackage received_package;
	int received_bytes = recvfrom(sock_fd, (void* )&received_package, sizeof(received_package), MSG_DONTWAIT, NULL, NULL);
	if (received_bytes > 0) {
		received_package.Deserialize();
	}
	return {received_bytes, received_package};
}

UdpSocket::~UdpSocket() {
	close(sock_fd);
}
