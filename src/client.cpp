#include "data_reader.h"
#include "my_socket.h"
#include "file.h"

#include <algorithm>
#include <random>
#include <chrono>
#include <cstdint>
#include <future>
#include <mutex>
#include <iostream>
#include <fstream>
#include <unordered_map>
#include <string>
#include <cstring>

#include <unistd.h>

#define LOCK(m) std::lock_guard<std::mutex> guard(m)

static std::mutex m;

static std::default_random_engine rng{};

uint32_t SendDataToServer(const File& file, 
		const UdpSocket& socket, const sockaddr_in& address) {
	uint32_t received_sum = 0;

	using namespace std::chrono;
	std::unordered_map<size_t, steady_clock::time_point> sent;

	unsigned char id_from[8];
	uint32_t file_checksum = file.GetChecksum();
	std::memcpy(&id_from, &file_checksum, sizeof(file_checksum));

	auto data = file.Data();
	std::shuffle(begin(data), end(data), rng);
	size_t idx = 0;
	while (true) {
		if (idx != data.size()) {
			bool is_sent = false;
			{
				LOCK(m);
				is_sent = socket.Send(
					data[idx].first, static_cast<uint32_t>(data.size()), 1u, 
					id_from, data[idx].second, address
				);
			}
			if (is_sent) {
				std::cout << "Sent package with id: " << file_checksum << ", seq_number = " 
					<< data[idx].first << std::endl;
			} else {
				std::cout << "Failed to send package with id: " << file_checksum 
					<< ", seq_number = " << data[idx].first << std::endl;
			}
			sent[idx] = steady_clock::now();
			++idx;
		}
		int received_bytes = 0;
		UdpDatagramPackage received_package;
		{
			LOCK(m);
			auto [bytes, package] = socket.Receive();
			received_bytes = bytes;
			received_package = package;
		}
		if (received_bytes > 0 && received_package.type == 0u) {
			std::cout << "Received package from server with id: " << file_checksum 
				<< ", seq_number = " << received_package.seq_number << std::endl;
			if (auto it = sent.find(received_package.seq_number); it != sent.end()) {
				sent.erase(it);
			}
			if (received_package.seq_total == data.size()) {
				std::memcpy(&received_sum, received_package.data, sizeof(received_sum));
				break;
			}
		}
		auto current_time = steady_clock::now();
		for (auto& msg : sent) {
			if (duration_cast<seconds>(current_time - msg.second) >= seconds{5}) {
				{
				LOCK(m);
					socket.Send(
						data[msg.first].first, static_cast<uint32_t>(data.size()), 
						1u, id_from, data[msg.first].second, address
					);
				}
				msg.second = steady_clock::now();
				std::cout << "Resent package with id: " << file_checksum 
					<< ", seq_number = " << data[msg.first].first << std::endl;
			}
		}
	}
	return received_sum;
}

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

		std::vector<std::future<uint32_t>> received_sums;
		received_sums.reserve(files.size());
		for (const auto& file : files) {
			received_sums.push_back(
				std::async(SendDataToServer, std::ref(file), std::ref(socket), std::ref(address_to_send))
			);
		}

		sleep(5);
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
