#pragma once

#include "my_socket.h"
#include "file.h"
#include "datagram.h"

#include <iostream>
#include <memory>
#include <unordered_map>
#include <unordered_set>

#include <sys/types.h>
#include <sys/socket.h>

class Server {
public:
	Server(std::unique_ptr<UdpSocket> server_socket);
	
	void ProcessPackages(const sockaddr_in& address_to_send, std::ostream& os = std::cout);
private:
	void AddChunkToFile(
		const UdpDatagramPackage& received_package,
		int received_bytes, 
		uint32_t id
	);

	void SendAnswerToClient(
		const sockaddr_in& address_to_send,
		const UdpDatagramPackage& received_package, 
		int received_bytes,
		uint32_t id,
		std::ostream& os
	);

	std::unique_ptr<UdpSocket> socket_;
	std::unordered_map<uint32_t, File> files;
	std::unordered_map<uint32_t, std::unordered_set<uint32_t>> id_to_seq_number;
};
