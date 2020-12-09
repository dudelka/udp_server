#include "data_reader.h"
#include "utils.h"

#include <cstdlib>
#include <exception>
#include <cstdint>
#include <utility>
#include <string>

#include <arpa/inet.h>

std::vector<File> ReadFiles(std::istream& is, std::ostream& os) {
	std::vector<File> result;
	os << "Enter the number of files to be read: ";
	int count = 0;
	is >> count;
	for (int i = 0; i < count; ++i) {
		os << "Enter the filename to be sent to the server: ";
		std::string filename = "";
		is >> filename;
		result.emplace_back(filename, MAX_UDP_PACKAGE_SIZE);
	}
	return result;
}

std::unique_ptr<UdpSocket> InitializeSocket(std::istream& is, std::ostream& os) {
	std::unique_ptr<UdpSocket> socket = std::make_unique<UdpSocket>();
	os << "Enter the port for socket to connect. If you dont't know what port to choose then enter 0." << std::endl;
	uint16_t port = 0;
	is >> port;
	socket->Bind(port);
	socket->SetToNonBlock();
	return std::move(socket);
}

std::string_view GetNumberFromIpAddress(std::string_view ip_address, uint32_t& number, char delimeter) {
	size_t pos = ip_address.find(delimeter);
	if (pos == ip_address.npos) {
		throw std::logic_error("Wrong IP address format\n");
	}
	std::string number_as_string(ip_address.substr(0, pos));
	number = static_cast<uint32_t>(std::atoi(number_as_string.data()));
	if (number >= static_cast<uint32_t>(256)) {
		throw std::logic_error("Wrong IP address format\n");
	}
	ip_address.remove_prefix(pos + 1);
	return ip_address;
}

sockaddr_in ParseAddress(std::string_view ip_address) {
	sockaddr_in address;
	uint32_t a = 0, b = 0, c = 0, d = 0;
	uint16_t port = 0;
	
	try {
		ip_address = GetNumberFromIpAddress(ip_address, a, '.');
		ip_address = GetNumberFromIpAddress(ip_address, b, '.');
		ip_address = GetNumberFromIpAddress(ip_address, c, '.');
		ip_address = GetNumberFromIpAddress(ip_address, d, ':');
	} catch(const std::logic_error& le) {
		throw;
	}
	port = static_cast<uint16_t>(std::atoi(ip_address.data()));
	if (!CheckBounds(port, static_cast<uint16_t>(0), static_cast<uint16_t>(65535))) {
		throw std::logic_error("Wrong IP address format\n");
	}

	uint32_t destination_address = (a << 24) | (b << 16) | (c << 8) | d;
	address.sin_family = AF_INET;
	address.sin_port = htons(port);
	address.sin_addr.s_addr = htonl(destination_address); 
	return address;
}

sockaddr_in ReadAddress(std::istream& is, std::ostream& os) {
	os << "Enter the address in format 000.000.000.000:0000 to send data" << std::endl;
	std::string ip_addr = "";
	is >> ip_addr;
	return ParseAddress(ip_addr);
}
