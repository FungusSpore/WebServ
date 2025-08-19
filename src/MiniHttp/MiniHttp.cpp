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

MiniHttp::~MiniHttp() {
	// Destructor implementation
}

// void MiniHttp::sendResponse(MiniHttpResponse& response) {
// 	try {
// 		std::string httpResponse = response.buildResponse();
//
// 		// here need to refactor to use passed to socket buffer
//
// 		// ssize_t totalSent = 0;
// 		// ssize_t responseLength = httpResponse.length();
// 		//
// 		// while (totalSent < responseLength) {
// 		// 	ssize_t sent = send(_socket.fd, httpResponse.c_str() + totalSent, 
// 		// 					   responseLength - totalSent, 0);
// 		// 	if (sent <= 0) {
// 		// 		throw std::runtime_error("Failed to send response to client");
// 		// 	}
// 		// 	totalSent += sent;
// 		// }
//
// 		std::cout << "Response sent successfully (" << totalSent << " bytes)" << std::endl;
// 	} catch (const std::exception& e) {
// 		std::cerr << "Error sending response: " << e.what() << std::endl;
// 	}
// }


bool MiniHttp::run() {
	std::cout << "MiniHttp::run() called for socket fd: " << _socket.fd << std::endl;
	MiniHttpRequest request(_socket);
	if (!request.parseRequest()) {
		// maybe socket also need to edge cases where socket reading failed.
		// Since this just going to return back to main loop if it doesnt have enough request data
		return false;
	}
	std::cout << "Parsed HTTP request successfully." << std::endl;

	MiniHttpResponse response(_server, request, _socket);
	response.parseResponse();
	std::cout << "Parsed HTTP response successfully." << std::endl;

	// refactor into the write buffer of the socket
	// sendResponse(response);
	_socket.write_buffer = response.buildResponse();

	// try {
	//
	// 	MiniHttpRequest request(_socket);
	// 	if (!request.parseRequest()) {
	// 		// maybe socket also need to edge cases where socket reading failed.
	// 		// Since this just going to return back to main loop if it doesnt have enough request data
	// 		return false;
	// 	}
	//
	// 	MiniHttpResponse response(_server, request, _socket);
	// 	response.parseResponse();
	//
	// 	// refactor into the write buffer of the socket
	// 	// sendResponse(response);
	// 	_socket.write_buffer = response.buildResponse();
	//
	// 	// close(_socket.fd);
	// } catch (const std::exception& e) {
	// 	std::cerr << "Error in MiniHttp::run(): " << e.what() << std::endl;
	//
	// 	// Send a basic 500 error response if possible
	// 	try {
	// 		std::string errorResponse = 
	// 			"HTTP/1.1 500 Internal Server Error\r\n"
	// 			"Content-Type: text/html\r\n"
	// 			"Content-Length: 50\r\n"
	// 			"Connection: close\r\n"
	// 			"\r\n"
	// 			"<html><body><h1>500 Internal Server Error</h1></body></html>";
	//
	// 		send(_socket.fd, errorResponse.c_str(), errorResponse.length(), 0);
	// 	} catch (...) {
	// 		// Ignore any further errors
	// 	}
	//
	// 	// close(_socket.fd);
	// }
	return true; // Indicate that the request was processed
}

bool MiniHttp::validateCGI() {
	// Placeholder for CGI validation logic
	// This function should check if the request is for a CGI script and validate it accordingly
	return true; // For now, just return true
}
