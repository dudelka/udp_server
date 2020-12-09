all: client server clean

run:
	./client < test_client.txt

clean: 
	rm -rf *.o
	rm -rf *.o

data_reader.o: data_reader.cpp
	g++ -std=c++17 -c -o data_reader.o data_reader.cpp

file.o: file.cpp
	g++ -std=c++17 -c -o file.o file.cpp

datagram.o: datagram.cpp
	g++ -std=c++17 -c -o datagram.o datagram.cpp

my_socket.o: my_socket.cpp
	g++ -std=c++17 -c -o my_socket.o my_socket.cpp

utils.o: utils.cpp
	g++ -std=c++17 -c -o utils.o utils.cpp

client.o: client.cpp
	g++ -std=c++17 -c -o client.o client.cpp

server.o: server.cpp
	g++ -std=c++17 -c -o server.o server.cpp

client_main.o:
	g++ -pthread -std=c++17 -c -o client_main.o client_main.cpp

server_main.o: server_main.cpp
	g++ -std=c++17 -c -o server_main.o server_main.cpp

client: client_main.o client.o data_reader.o file.o datagram.o my_socket.o utils.o
	g++ -pthread -std=c++17 -o client client_main.o client.o data_reader.o file.o datagram.o my_socket.o utils.o

server: server_main.o server.o data_reader.o file.o my_socket.o utils.o
	g++ -std=c++17 -o server server_main.o server.o data_reader.o file.o datagram.o my_socket.o utils.o
