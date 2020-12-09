#include "client.h"

#include <algorithm>
#include <random>
#include <utility>
#include <cstring>

static std::default_random_engine rng{};

Client::Client(std::unique_ptr<UdpSocket> client_socket) 
    : socket_(std::move(client_socket)) {
    if (!socket_) {
        throw std::runtime_error("Socket binded to the client is unitialized\n");
    }
}

uint32_t Client::ProcessData(const File& file, const sockaddr_in& address) {
    uint32_t received_sum = 0;

    std::unordered_map<size_t, std::chrono::steady_clock::time_point> sent;

    unsigned char id_from[8];
    uint32_t file_checksum = file.GetChecksum();
    std::memcpy(&id_from, &file_checksum, sizeof(file_checksum));

    auto data = file.Data();
    uint32_t data_size = static_cast<uint32_t>(data.size());
    std::shuffle(begin(data), end(data), rng);
    size_t idx = 0;
    while (true) {
        if (idx != data.size()) {
            auto [package, package_size] = CreatePackage(
                data[idx].seq_number, data_size, 1u, id_from, data[idx].data
            );
            ProcessAndLogSinglePackage(package, package_size, address, file_checksum);
            sent[idx] = std::chrono::steady_clock::now();
            ++idx;
		}

        auto [received_bytes, received_package] = socket_->Receive();
        if (received_bytes > 0 && received_package.type == 0u) {
            std::optional<uint32_t> received_result = ProcessAndLogReceivedPackage(
                sent, received_package, data_size, file_checksum
            );
            if (received_result) {
                received_sum = *received_result;
                break;
            }
        }
		
        ResendPackages(sent, data, id_from, address, file_checksum);
    }
    return received_sum;
}

void Client::ProcessAndLogSinglePackage(
	UdpDatagramPackage& package,
    size_t package_size,
    const sockaddr_in& address,
    uint32_t id,
    std::ostream& os
) {
	uint32_t seq_number = package.seq_number;
    bool is_sent = socket_->Send(package, package_size, address);
    if (is_sent) {
        os << "Sent package with id: " << id << ", seq_number = " 
            << seq_number << std::endl;
    } else {
        os << "Failed to send package with id: " << id
            << ", seq_number = " << seq_number << std::endl;
    }
}

std::optional<uint32_t> Client::ProcessAndLogReceivedPackage(
    std::unordered_map<size_t, std::chrono::steady_clock::time_point>& sent,
	const UdpDatagramPackage& received_package,
    uint32_t total_size,
	uint32_t id,
    std::ostream& os
) {
    os << "Received package from server with id: " << id
        << ", seq_number = " << received_package.seq_number << std::endl;
    if (auto it = sent.find(received_package.seq_number); it != sent.end()) {
        sent.erase(it);
    }
    if (received_package.seq_total == total_size) {
        uint32_t received_sum = 0;
        std::memcpy(&received_sum, received_package.data, sizeof(received_sum));
        return received_sum;
    }
    return std::nullopt;
}

void Client::ResendPackages(
	std::unordered_map<size_t, std::chrono::steady_clock::time_point>& sent,
	const std::vector<File::FileChunk>& data,
	unsigned char* id_from,
	const sockaddr_in& address,
	uint32_t id,
	std::ostream& os
) {
    using namespace std::chrono;
    auto current_time = steady_clock::now();
    for (auto& msg : sent) {
        if (duration_cast<seconds>(current_time - msg.second) >= seconds{5}) {
			uint32_t data_size = static_cast<uint32_t>(data.size());
            auto [package, package_size] = CreatePackage(
                data[msg.first].seq_number, data_size, 1u, id_from, data[msg.first].data
            );
            socket_->Send(package, package_size, address);
        	msg.second = steady_clock::now();
        	os << "Resending package with id: " << id
        	    << ", seq_number = " << data[msg.first].seq_number << std::endl;
		}
    }
}

