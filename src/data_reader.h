#pragma once

#include "my_socket.h"
#include "file.h"

#include <iostream>
#include <memory>
#include <vector>
#include <string_view>

std::vector<File> ReadFiles(std::istream& is = std::cin, std::ostream& os = std::cout);

std::unique_ptr<UdpSocket> InitializeSocket(std::istream& is = std::cin, std::ostream& os = std::cout);

sockaddr_in ParseAddress(std::string_view ip_address);

sockaddr_in ReadAddress(std::istream& is = std::cin, std::ostream& os = std::cout);
