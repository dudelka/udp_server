#include "server.h"

#include <stdexcept>
#include <utility>
#include <cstring>

Server::Server(std::unique_ptr<UdpSocket> server_socket) 
	: socket_(std::move(server_socket)) {
	if (!socket_) {
		throw std::runtime_error("Socket is empty\n");
	}
}

void Server::ProcessPackages(const sockaddr_in& address_to_send, std::ostream& os) {
	while (true) {
		auto [received_bytes, received_package] = socket_->Receive();
		if (received_bytes > 0 && received_package.type == 1u) {
			uint32_t id = 0;
			std::memcpy(&id, &received_package.id, sizeof(id));

			os << "Received package from client with id: " << id 
				<< ", seq_number = " << received_package.seq_number << std::endl;

			if (id_to_seq_number[id].find(received_package.seq_number) == 
					id_to_seq_number[id].end()) {
				AddChunkToFile(received_package, received_bytes, id);
			}

			SendAnswerToClient(address_to_send, received_package, received_bytes, id, os);
		}
	}
}

void Server::AddChunkToFile(
	const UdpDatagramPackage& received_package, 
	int received_bytes, 
	uint32_t id
) {
	id_to_seq_number[id].insert(received_package.seq_number);
	File::FileChunk chunk;
	chunk.seq_number = received_package.seq_number;
	int chunk_size = received_bytes - 20;  // 20 байт на начало пакета
	chunk.data.reserve(chunk_size);
	for (int i = 0; i < chunk_size; ++i) {
		chunk.data.push_back(received_package.data[i]);
	}
	files[id].AddChunk(std::move(chunk));
}

void Server::SendAnswerToClient(
	const sockaddr_in& address_to_send,
	const UdpDatagramPackage& received_package,
	int received_bytes,
	uint32_t id,
	std::ostream& os
) {
	std::vector<unsigned char> data;
	if (received_package.seq_total == files[id].GetChunksCount()) {
		files[id].Sort();
		uint32_t checksum = files[id].CalculateChecksum();
		unsigned char buf[sizeof(checksum)];
		std::memcpy(buf, &checksum, sizeof(checksum));
		std::vector<unsigned char> data_to_send(sizeof(buf));
		std::memcpy(data_to_send.data(), buf, sizeof(buf));
		data = data_to_send;
	}
	auto [package, package_size] = CreatePackage(
		received_package.seq_number, 
		static_cast<uint32_t>(files[id].GetChunksCount()), 
		0u, received_package.id, data               
	);
	socket_->Send(package, package_size, address_to_send);
	os << "Sent to the client package with id: " << id 
		<< ", seq_number = " << files[id].GetChunksCount() << std:: endl;
}

