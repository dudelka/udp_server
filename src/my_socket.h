#pragma once

#include <cstdint>
#include <utility>
#include <vector>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define MAX_UDP_PACKAGE_SIZE 1455

struct UdpDatagramPackage {
	uint32_t seq_number;
	uint32_t seq_total;
	uint32_t type;
	unsigned char id[8];
	unsigned char data[MAX_UDP_PACKAGE_SIZE];

	void Serialize();
	void Deserialize();
};

class UdpSocket {
public:
	UdpSocket();

	void Bind(const uint16_t port);
	void SetToNonBlock();

	bool Send(uint32_t seq_number, uint32_t seq_total, uint8_t type, 
		unsigned char* id, const std::vector<unsigned char>& data, const sockaddr_in& address) const;
	std::pair<int, UdpDatagramPackage> Receive() const;

	~UdpSocket();
private:
	int sock_fd;
	sockaddr_in address;
};
