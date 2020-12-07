#pragma once

#include <cstdint>
#include <fstream>
#include <vector>
#include <string>

class File {
public:
    struct FileChunk {
        uint32_t seq_number;
        std::vector<unsigned char> data;
    };
 
	File() = default;
	File(const std::string& filename, const uint16_t package_size);
	
	uint32_t CalculateChecksum();
	void AddChunk(FileChunk&& chunk);
	void Sort();

	std::string GetName() const;
	size_t GetFileSize() const;
	size_t GetChunksCount() const;
	std::vector<FileChunk> Data() const;
	uint32_t GetChecksum() const;
private:
	void DivideIntoChunks(std::fstream& file, const uint16_t package_size);

	std::string filename_;
	size_t file_size_;
	std::vector<FileChunk> chunks_;
	uint32_t checksum_;
};
