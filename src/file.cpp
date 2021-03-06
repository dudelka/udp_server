#include "file.h"
#include "utils.h"

#include <algorithm>
#include <exception>
#include <utility>

File::File(const std::string& filename, const uint16_t package_size) 
		: filename_(filename), file_size_(0){
	std::fstream file(filename);
	if (!file.is_open()) {
		throw std::runtime_error("Can't open file with filename: " + filename + "\n");
	}
	DivideIntoChunks(file, package_size);
	file.close();
}

void File::DivideIntoChunks(std::fstream& file, const uint16_t package_size) {
	uint32_t current_idx = 1;
	while (!file.eof()) {
		std::vector<unsigned char> chunk;
		chunk.reserve(package_size);
		for (uint16_t i = 0; i < package_size; ++i) {
			if (!file.eof()) {
				unsigned char c;
				file >> c;
				chunk.push_back(c);
			}
		}
		file_size_ += chunk.size();
		chunk.shrink_to_fit();
		chunks_.push_back({current_idx++, std::move(chunk)});
	}
}

uint32_t File::CalculateChecksum() {
	uint32_t crc = 0;
	for (const auto& chunk : chunks_) {
		crc = crc32c(crc, chunk.data.data(), chunk.data.size());
	}
	checksum_ = crc;
	return checksum_;
}

void File::AddChunk(File::FileChunk&& chunk) {
	chunks_.push_back(std::move(chunk));
}

void File::Sort() {
	std::sort(
		begin(chunks_),
		end(chunks_),
		[](const FileChunk& lhs, FileChunk& rhs) {
			return lhs.seq_number < rhs.seq_number;
		}
	);
}

std::string File::GetName() const {
	return filename_;
}

size_t File::GetFileSize() const {
	return file_size_;
}

size_t File::GetChunksCount() const {
	return chunks_.size();
}

std::vector<File::FileChunk> File::Data() const {
	return chunks_;
}

uint32_t File::GetChecksum() const {
	return checksum_;
}

