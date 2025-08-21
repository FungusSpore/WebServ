#include <iostream>
#include "MiniHttp.hpp"
#include "CGI.hpp"
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
	MiniHttpRequest request(_socket);
	if (!request.parseRequest()) {
		// maybe socket also need to edge cases where socket reading failed.
		// Since this just going to return back to main loop if it doesnt have enough request data
		return false;
	}
	// std::cout << "Parsed HTTP request successfully." << std::endl;

	MiniHttpResponse response(_server, request, _socket);
	response.parseResponse();
	// std::cout << "Parsed HTTP response successfully." << std::endl;

	// refactor into the write buffer of the socket
	// sendResponse(response);
	_socket.write_buffer = response.buildResponse();

	return true;
}

std::string MiniHttp::getCgiHeader(std::string& cgiBuffer) {
	std::string header;
	std::string::size_type s = cgiBuffer.find("\r\n\r\n");
	if (s == std::string::npos) {
		// Try with just \n\n for Unix-style line endings
		s = cgiBuffer.find("\n\n");
		if (s == std::string::npos)
			return header;
		header = cgiBuffer.substr(0, s + 2);
		cgiBuffer.erase(0, s + 2);
	} else {
		header = cgiBuffer.substr(0, s + 4);
		cgiBuffer.erase(0, s + 4);
	}
	return header;
}

bool MiniHttp::validateCGI() {
	if (_socket.read_buffer.empty()) {
		return false;
	}
	
	try {
		// std::cout << "Validating CGI output..." << std::endl;
		std::string cgiBuffer = _socket.read_buffer;
		std::string cgiHeaders = getCgiHeader(cgiBuffer);

		if (cgiHeaders.empty()) {
			return false;
		}
		
		std::ostringstream response;
		response << "HTTP/1.1 200 OK\r\n";
		
		if (!cgiHeaders.empty()) {
			size_t headerEndPos = cgiHeaders.find("\r\n\r\n");
			if (headerEndPos != std::string::npos) {
				cgiHeaders = cgiHeaders.substr(0, headerEndPos);
			} else {
				headerEndPos = cgiHeaders.find("\n\n");
				if (headerEndPos != std::string::npos) {
					cgiHeaders = cgiHeaders.substr(0, headerEndPos);
				}
			}
			response << cgiHeaders << "\r\n";
		}
		
		if (cgiHeaders.find("Content-Length:") == std::string::npos && 
			cgiHeaders.find("content-length:") == std::string::npos) {
			response << "Content-Length: " << cgiBuffer.size() << "\r\n";
		}

		// shouldnt close connection here?
		// response << "Connection: close\r\n" << "\r\n" << cgiBuffer;
		response << "\r\n" << cgiBuffer;
		
		_socket.write_buffer = response.str();
		_socket.read_buffer.clear();

		// std::cout << "CGI response: " << _socket.write_buffer << std::endl;
		//
		// std::cout << "CGI response built successfully" << std::endl;
		return true;
		
	} catch (const std::exception& e) {
		std::cerr << "Error processing CGI output: " << e.what() << std::endl;
		// dont exit just return eerror code	
		_socket.write_buffer = "HTTP/1.1 500 Internal Server Error\r\n"
							   "Content-Type: text/html\r\n"
							   "Content-Length: 50\r\n"
							   "Connection: close\r\n"
							   "\r\n"
							   "<html><body><h1>500 Internal Server Error</h1></body></html>";
		_socket.read_buffer.clear();
		return true;
	}
}
