#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <unistd.h>
#include <fstream>
#include <cstring>
#include <string.h>

#define PORT 8000

static struct sockaddr_in setup_address() {
	/* Setup Address */
	struct sockaddr_in address;
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(PORT);
	return address;
}

int main() {
	int server_fd, new_socket;
	struct sockaddr_in address;
	int opt = 1;
	ssize_t send_status;
	socklen_t addrlen = sizeof(address);

	std::string response = "HTTP/1.1 200 OK\n\n";

	// Setup address
	address = setup_address();

	server_fd = socket(AF_INET, SOCK_STREAM, 0);

	if (server_fd == -1) {
		perror("socket failed");
		exit(EXIT_FAILURE);

	}
	// Set the socket options
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
		perror("setsockopt");
		exit(EXIT_FAILURE);
	}

	// Bind address to the socket
	if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) == -1) {
		perror("bind failed");
		exit(EXIT_FAILURE);
	}

	// Listen for connections
	if (listen(server_fd, 3) == -1) {
		perror("listen");
		exit(EXIT_FAILURE);
	}

	std::string msg1 = "Server is running";
	std::string msg2 = "-----------------";
	std::cout << msg1 << std::endl;
	std::cout << msg2 << std::endl;

	std::ifstream HtmlFile("index.html", std::ifstream::binary);

	if (!HtmlFile) {
		std::cout << "Error: Can't read file" << std::endl;

	}

	// get length of file:
	HtmlFile.seekg (0, HtmlFile.end);
	int length = HtmlFile.tellg();
	HtmlFile.seekg (0, HtmlFile.beg);

	char* buf = new char [length];
	HtmlFile.read(buf, length);

	while (true) {
		// Accept for connections
		new_socket = accept(server_fd, (struct sockaddr *)&address, &addrlen);

		if (new_socket == -1) {

			perror("accept");
			exit(EXIT_FAILURE);
		}

		// Store html char pointer data
		std::string temp = response + (std::string)buf;
		const char* html = temp.c_str();

		// Send the buffer of html web page
		send_status = send(new_socket, html, strlen(html), 0);

		if (send_status == -1) {
			std::cout << "Sending the message failed \n" << std::endl;
			return -1;
		}
		else {
			std::cout << "Sent message to client \n" << std::endl;
		}

		std::cout << "Buffer: " << (char*)buf << std::endl;

		// close the socket
		close(new_socket);
	}
	// close file
	HtmlFile.close();
}
