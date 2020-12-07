#pragma once

#include <cstdint>
#include <vector>

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

std::pair<UdpDatagramPackage, size_t> CreatePackage(
    uint32_t seq_number,
    uint32_t seq_total,
    uint32_t type,
    const unsigned char* id,
    const std::vector<unsigned char>& data
);
