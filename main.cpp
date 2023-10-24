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

char* read_html_file(std::string html_file_path) {
	/* read html file */
	std::ifstream HtmlFile(html_file_path, std::ifstream::binary);

	if (!HtmlFile) {
		std::cout << "Error: Can't read file" << std::endl;
		exit (EXIT_FAILURE);
	}

	// get length of file:
	HtmlFile.seekg (0, HtmlFile.end);
	int length = HtmlFile.tellg();
	HtmlFile.seekg (0, HtmlFile.beg);

	char* buf = new char [length];
	HtmlFile.read(buf, length);

	// close file
	HtmlFile.close();

	return buf;
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

	while (true) {
		// Accept for connections
		new_socket = accept(server_fd, (struct sockaddr *)&address, &addrlen);

		if (new_socket == -1) {

			perror("accept");
			exit(EXIT_FAILURE);
		}

		int buf2[1024] = { 0 };

		// Read buffer of request
		ssize_t read_status = read(new_socket, buf2, 1024);

		char* temp2 = (char*)buf2;

		std::string temp3 = (std::string)temp2;

		char temp4[1024] = { 0 };

		for (long unsigned int i = 0; i < temp3.length(); i++) {
			temp4[i] = temp3[i];
		}

		char *line = strtok(temp4, "\n");

		std::string request = "";

		while (line != NULL) {
			request = (std::string)line;
			line = strtok(NULL, "\n");
			break;
		}

		char* buf = (char*)"";

		std::cout << request.compare("GET / HTTP/1.1") << std::endl;

		if (request.compare("GET / HTTP/1.1") == 1) {
			buf = read_html_file("static/index.html");
		}
		else if (request.compare("GET /about HTTP/1.1") == 1) {
			buf = read_html_file("static/about/index.html");
		}

		if (read_status == -1) {
			std::cout << "Reading the message failed \n" << std::endl;
			return -1;
		}
		else {
			std::cout << (char*)buf2 << std::endl;
		}

		// Store HTML char pointer data
		std::string temp = response + (std::string)buf;
		const char* html = temp.c_str();

		// Send the buffer of html web page
		send_status = send(new_socket, html, strlen(html), 0);

		if (send_status == -1) {
			std::cout << "Sending the message failed \n" << std::endl;
			return -1;
		}
		else {
			std::cout << "Sent http request to the client \n" << std::endl;
		}

		// close the socket
		close(new_socket);
	}
}
