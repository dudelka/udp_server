#pragma once

#include "datagram.h"

#include <mutex>
#include <cstdint>
#include <utility>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

class UdpSocket {
public:
	UdpSocket();

	UdpSocket(const UdpSocket&) = delete;
	UdpSocket& operator=(const UdpSocket&) = delete;

	void Bind(const uint16_t port);
	void SetToNonBlock();

	bool Send(
        UdpDatagramPackage& package, 
        size_t package_size, 
        const sockaddr_in& address
    ) const;
	std::pair<int, UdpDatagramPackage> Receive() const;

	~UdpSocket();
private:
	int sock_fd;
	sockaddr_in address;

	mutable std::mutex m;
};
