#pragma once

#include <cstdint>
#include <fstream>
#include <utility>
#include <vector>
#include <string>

class File {
public: 
	File() = default;
	File(const std::string& filename, const uint16_t package_size);
	
	uint32_t CalculateChecksum();
	void AddChunk(std::pair<uint32_t, std::vector<unsigned char>> chunk);
	void Sort();

	std::string GetName() const;
	size_t GetFileSize() const;
	size_t GetChunksCount() const;
	std::vector<std::pair<uint32_t, std::vector<unsigned char>>> Data() const;
	uint32_t GetChecksum() const;
private:
	void DivideIntoChunks(std::fstream& file, const uint16_t package_size);

	std::string filename_;
	size_t file_size_;
	std::vector<std::pair<uint32_t, std::vector<unsigned char>>> chunks_;
	uint32_t checksum_;
};
