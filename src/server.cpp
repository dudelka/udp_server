#include "my_socket.h"
#include "file.h"
#include "data_reader.h"

#include <iostream>
#include <utility>
#include <unordered_map>
#include <unordered_set>
#include <cstring>

int main() {
	try {
		UdpSocket socket = InitializeSocket();
		sockaddr_in address_to_send = ReadAddress();

		std::unordered_map<uint32_t, File> files;
		std::unordered_map<uint32_t, std::unordered_set<uint32_t>> id_to_seq_number;
		while (true) {
			auto [received_bytes, received_package] = socket.Receive();
			if (received_bytes > 0 && received_package.type == 1u) {
				uint32_t id = 0;
				std::memcpy(&id, &received_package.id, sizeof(id));

				std::cout << "Received package from client with id: " << id 
					<< ", seq_number = " << received_package.seq_number << std::endl;
	
				if (id_to_seq_number[id].find(received_package.seq_number) == 
						id_to_seq_number[id].end()) {
					id_to_seq_number[id].insert(received_package.seq_number);
					std::pair<uint32_t, std::vector<unsigned char>> chunk;
					chunk.first = received_package.seq_number;
					chunk.second.reserve(received_bytes - 20); // 20 байт на начало пакета
					for (int i = 0; i < received_bytes - 20; ++i) {
						chunk.second.push_back(received_package.data[i]);
					}
					files[id].AddChunk(std::move(chunk));
				}
				
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
				socket.Send(
					received_package.seq_number, 
					static_cast<uint32_t>(files[id].GetChunksCount()), 
					0u, received_package.id, data, address_to_send
				);
				std::cout << "Sent to the client package with id: " << id 
					<< ", seq_number = " << files[id].GetChunksCount() << std:: endl;
			}
		}
	} catch (const std::runtime_error& re) {
		std::cout << re.what();
		exit(1);
	} catch (const std::logic_error& le) {
		std::cout << le.what();
		exit(2);
	}

	return 0;
}
