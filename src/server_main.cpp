#include "server.h"
#include "my_socket.h"
#include "data_reader.h"

#include <iostream>
#include <utility>

int main() {
	try {
		std::unique_ptr<UdpSocket> socket(InitializeSocket());
		sockaddr_in address_to_send = ReadAddress();
		Server server(std::move(socket));
		server.ProcessPackages(address_to_send);
	} catch (const std::runtime_error& re) {
		std::cout << re.what();
		exit(1);
	} catch (const std::logic_error& le) {
		std::cout << le.what();
		exit(2);
	}

	return 0;
}
