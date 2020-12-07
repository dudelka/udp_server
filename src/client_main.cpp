#include "client.h"
#include "data_reader.h"
#include "my_socket.h"
#include "file.h"

#include <cstdint>
#include <thread>
#include <future>
#include <iostream>
#include <utility>

int main() {
	try {
		std::vector<File> files = ReadFiles();
		std::vector<uint32_t> checksums;
		checksums.reserve(files.size());
		for (auto& file : files) {
			checksums.push_back(file.CalculateChecksum());
		}

		UdpSocket socket = InitializeSocket();
		sockaddr_in address_to_send = ReadAddress();
        Client client(std::make_unique<UdpSocket>(std::move(socket)));

		std::vector<std::future<uint32_t>> received_sums;
		received_sums.reserve(files.size());
		for (const auto& file : files) {
			received_sums.push_back(
				std::async(&Client::ProcessData, std::ref(client), std::ref(file), std::ref(address_to_send))
			);
		}

		std::this_thread::sleep_for(std::chrono::seconds(5));
		for (size_t i = 0; i < received_sums.size(); ++i) {
			uint32_t received_sum = received_sums[i].get();
			if (received_sum == checksums[i]) {
				std::cout << "File " << files[i].GetName() << " was successfuly sent to server" << std::endl;
			} else {
				std::cout << "Received wrong checksum for file " << files[i].GetName()  << std::endl;
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
