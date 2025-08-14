#include <iostream>
#include "MiniHttp.hpp"
#include "Socket.hpp"
#include <unistd.h>
#include <sys/socket.h>

MiniHttp::MiniHttp(Socket& socket, WebServer& server) : _socket(socket), _server(server) {}

// MiniHttp::MiniHttp(int socket_fd, WebServer& server)
// 	: _socket_fd(socket_fd), _server(server) {
// 	// Initialize the MiniHttp with the socket file descriptor and server reference
// }

MiniHttp::MiniHttp(const MiniHttp& other)
	: _socket(other._socket), _server(other._server) {}

MiniHttp& MiniHttp::operator=(const MiniHttp& other) {
	if (this != &other) {
		_socket = other._socket;
		_server = other._server;
	}
	return *this;
}

MiniHttp::~MiniHttp() {
	// Destructor implementation
}

void MiniHttp::sendResponse(MiniHttpResponse& response) {
	try {
		std::string httpResponse = response.buildResponse();

		// here need to refactor to use passed to socket buffer
		
		ssize_t totalSent = 0;
		ssize_t responseLength = httpResponse.length();

		while (totalSent < responseLength) {
			ssize_t sent = send(_socket.fd, httpResponse.c_str() + totalSent, 
							   responseLength - totalSent, 0);
			if (sent <= 0) {
				throw std::runtime_error("Failed to send response to client");
			}
			totalSent += sent;
		}
		
		std::cout << "Response sent successfully (" << totalSent << " bytes)" << std::endl;
	} catch (const std::exception& e) {
		std::cerr << "Error sending response: " << e.what() << std::endl;
	}
}


void MiniHttp::run() {
	try {

		MiniHttpRequest request(_socket);
		if (!request.parseRequest()) {
			// maybe socket also need to edge cases where socket reading failed.
			// Since this just going to return back to main loop if it doesnt have enough request data
			return;
		}

		MiniHttpResponse response(_server, request, _socket.fd);
		response.parseResponse();

		sendResponse(response);

		close(_socket.fd);
	} catch (const std::exception& e) {
		std::cerr << "Error in MiniHttp::run(): " << e.what() << std::endl;
		
		// Send a basic 500 error response if possible
		try {
			std::string errorResponse = 
				"HTTP/1.1 500 Internal Server Error\r\n"
				"Content-Type: text/html\r\n"
				"Content-Length: 50\r\n"
				"Connection: close\r\n"
				"\r\n"
				"<html><body><h1>500 Internal Server Error</h1></body></html>";
			
			send(_socket.fd, errorResponse.c_str(), errorResponse.length(), 0);
		} catch (...) {
			// Ignore any further errors
		}
		
		close(_socket.fd);
	}
}


