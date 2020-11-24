#pragma once

#include <iostream>
#include <cstdint>
#include <utility>
#include <vector>

template <typename T>
bool CheckBounds(T number, T lower_bound, T upper_bound) {
	if (number < lower_bound || number >= upper_bound) {
		return false;
	}
	return true;
}

uint32_t crc32c(uint32_t crc, const unsigned char* buf, size_t len);

