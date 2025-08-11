#include <iostream>
#include "MiniHttp.hpp"
#include <unistd.h>

MiniHttp::MiniHttp(int socket_fd, WebServer& server)
	: _socket_fd(socket_fd), _server(server) {
	// Initialize the MiniHttp with the socket file descriptor and server reference
}

MiniHttp::MiniHttp(const MiniHttp& other)
	: _socket_fd(other._socket_fd), _server(other._server) {
	// Copy constructor implementation
}

MiniHttp& MiniHttp::operator=(const MiniHttp& other) {
	if (this != &other) {
		_socket_fd = other._socket_fd;
		_server = other._server;
		// Copy other members if needed
		// _request = other._request;
		// _response = other._response;
		// _route = other._route;
	}
	return *this;
}

MiniHttp::~MiniHttp() {
	// Destructor implementation
}

void MiniHttp::run() {
	try {

		MiniHttpRequest request(_socket_fd);
		request.parseRequest();

		// MiniHttpRoute route(request, _server);
		// route.parseRoute();

		MiniHttpResponse response(_server, request, _socket_fd);
		response.parseResponse();

		sendResponse();

		close(_socket_fd);
	} catch (const std::exception& e) {
		std::cerr << "Error: " << e.what() << std::endl;
	}
}
