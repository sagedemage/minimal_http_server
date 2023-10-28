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
#include <filesystem>
#include <sstream>

namespace fs = std::filesystem;

#define PORT 8000

static struct sockaddr_in setup_address() {
	/* Setup Address */
	struct sockaddr_in address;
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(PORT);
	return address;
}

std::string find_requested_html_file(std::string file_route, std::string path_route) {
	/* Recursively look through the directory to find the requested html file */
	std::string file_name = "static/404.html";
	for (const auto & entry : fs::directory_iterator(path_route)) {
		fs::path path(entry);
		std::error_code ec;
		std::string path_string = path.string();
		if (file_route.compare(path_string) == 0) {
			file_name = file_route;
			return file_name;
		}
		if (fs::is_directory(path, ec)) {
			file_name = find_requested_html_file(file_route, path_string);
		}
		else if (ec)
		{
			std::cerr << "Error in is_directory: " << ec.message();
		}
	}
	return file_name;
}

char* read_html_file(std::string html_file_path) {
	/* Read html file and returns its buffer */
	std::ifstream HtmlFile(html_file_path, std::ifstream::binary);

	if (!HtmlFile) {
		std::cout << "Error: Can't read file: " + html_file_path << std::endl;
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
	int opt = 1;

	// Setup address
	struct sockaddr_in address = setup_address();
	socklen_t addrlen = sizeof(address);

	int server_fd = socket(AF_INET, SOCK_STREAM, 0);

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

	std::cout << "Server is running" << std::endl;
	std::cout << "-----------------" << std::endl;

	while (true) {
		// Accept for connections
		int new_socket = accept(server_fd, (struct sockaddr *)&address, &addrlen);

		if (new_socket == -1) {
			perror("accept");
			exit(EXIT_FAILURE);
		}

		int read_buf[1024] = { 0 };

		// Read buffer of request
		ssize_t read_status = read(new_socket, read_buf, 1024);

		if (read_status == -1) {
			std::cout << "Reading the message failed \n" << std::endl;
			return -1;
		}

		char* temp2 = (char*)read_buf;

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

		std::string request_items[3];
		int i = 0;
		std::stringstream ssin(request);
		while (ssin.good() && i < 3){
			ssin >> request_items[i];
			++i;
		}

		// change file route
		std::string file_route = request_items[1];
		file_route = "static" + file_route;
		char file_route_last_char = file_route[file_route.length() -1];
		// file with .ico file extension
		std::string ico_extension = file_route.substr(file_route.length()-4, -1);
		// file with .html file extension
		std::string html_extension = file_route.substr(file_route.length()-5, -1);
		if (file_route_last_char == '/') {
			file_route += "index.html";
		}
		else if (html_extension != ".html" && ico_extension != ".ico") {
			file_route += "/index.html";
		}
		

		// find if requested file exists in web server
		std::string file_name = find_requested_html_file(file_route, "static");

		// read HTML file
		char* buf = read_html_file(file_name);

		// Store HTML char pointer data
		std::string response = "HTTP/1.1 200 OK\n\n";
		std::string temp = response + (std::string)buf;
		const char* html = temp.c_str();

		// Send the buffer of html web page
		ssize_t send_status = send(new_socket, html, strlen(html), 0);

		if (send_status == -1) {
			std::cout << "Sending the message failed \n" << std::endl;
			return -1;
		}
		else {
			std::cout << (char*)read_buf << std::endl;
		}

		// close the socket
		close(new_socket);
	}
}
