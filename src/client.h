#pragma once

#include "my_socket.h"
#include "file.h"
#include "datagram.h"

#include <iostream>
#include <chrono>
#include <optional>
#include <unordered_map>
#include <memory>

#include <sys/types.h>
#include <sys/socket.h>

class Client {
public:
    Client(std::unique_ptr<UdpSocket> client_socket);

    uint32_t ProcessData(const File& file, const sockaddr_in& address);
private:
    void ProcessAndLogSinglePackage(
        UdpDatagramPackage* package, 
        size_t package_size,
        const sockaddr_in& address,
        uint32_t id,
        std::ostream& os = std::cout
    );

    std::optional<uint32_t> ProcessAndLogReceivedPackage(
        std::unordered_map<size_t, std::chrono::steady_clock::time_point>* sent,
		const UdpDatagramPackage& received_package,
        uint32_t total_size,
        uint32_t id,
        std::ostream& os = std::cout
    );

    void ResendPackages(
        std::unordered_map<size_t, std::chrono::steady_clock::time_point>* sent,
        const std::vector<File::FileChunk>& data,
        unsigned char* id_from,
        const sockaddr_in& address,
        uint32_t id,
        std::ostream& os = std::cout
    );

    std::unique_ptr<UdpSocket> socket_;
};
