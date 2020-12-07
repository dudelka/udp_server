#include "datagram.h"

#include <cstring>

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

std::pair<UdpDatagramPackage, size_t> CreatePackage(
        uint32_t seq_number,
        uint32_t seq_total,
        uint32_t type,
        const unsigned char* id,
        const std::vector<unsigned char>& data
) {
    UdpDatagramPackage package = {
        .seq_number = seq_number,
        .seq_total = seq_total,
        .type = type
    };
    std::memcpy(&package.id, id, sizeof(package.id));
    std::memcpy(&package.data, data.data(), data.size());

    size_t package_size = 0;
    package_size += sizeof(package.seq_number);
    package_size += sizeof(package.seq_total);
    package_size += sizeof(package.type);
    package_size += sizeof(package.id);
    package_size += data.size();

    return {package, package_size};
}
