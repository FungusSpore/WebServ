#include "MiniHttpRequest.hpp"
#include "MiniHttpUtils.hpp"
#include <cstddef>
#include <exception>
#include <iostream>
#include <unistd.h>
#include <cstring>
#include <sys/socket.h>
#include <cstdlib>
#include <sstream>
#include "Socket.hpp"

MiniHttpRequest::MiniHttpRequest(Socket& socket) 
	: _socket(socket), _buffer(""), _method(""), _path(""), _version(""), _body(""), _trailer(""), _headers(), _isHeaderLoaded(false) {}

// Copy constructor and assignment operator are intentionally not implemented
// because this class contains references that cannot be safely copied

MiniHttpRequest::~MiniHttpRequest() {
	// if (_socket_fd != -1) {
	// 	close(_socket_fd);
	// }
}

Socket& MiniHttpRequest::getSocket() const {
	return _socket;
}

const std::string& MiniHttpRequest::getMethod() const {
	return _method;
}

const std::string& MiniHttpRequest::getPath() const {
	return _path;
}

const std::string& MiniHttpRequest::getVersion() const {
	return _version;
}

const std::string& MiniHttpRequest::getBody() const {
	return _body;
}

const std::string& MiniHttpRequest::getTrailer() const {
	return _trailer;
}

const std::multimap<std::string, std::string>& MiniHttpRequest::getHeaders() const {
	return _headers;
}


void MiniHttpRequest::addHeader(const std::string& key, const std::string& value) {
	_headers.insert(std::make_pair(key, value));
}

bool MiniHttpRequest::loadHeader() {
	if (_buffer.find("\r\n\r\n") != std::string::npos) {
		return true;
	}
	if (_socket.read_buffer.find("\r\n\r\n") == std::string::npos) {
		_buffer += _socket.read_buffer;
		_socket.read_buffer.erase();
		return false;
	}
	_buffer += _socket.read_buffer;
	_socket.read_buffer.erase();
	return true;






	// int bytes_read;
	// char buffer[1024] = {0};
	//
	// while ((bytes_read = recv(_socket_fd, buffer, sizeof(buffer) -1, 0)) > 0) {
	// 	buffer[bytes_read] = '\0';
	// 	request.append(buffer);
	//
	// 	if (request.find("\r\n\r\n") != std::string::npos) {
	// 		break;
	// 	}
	// }

	// if (bytes_read < 0)
	// 	throw std::runtime_error("Failed to read from socket");
	// else if (bytes_read == 0)
	// 	throw std::runtime_error("Connection closed by client");
	// if (request.empty())
	// 	throw std::runtime_error("Received empty request");
}

void MiniHttpRequest::parseHeader() {
	std::istringstream iss(_buffer);
	std::string line;

	if (std::getline(iss, line)) {
		std::istringstream methods(line);
		methods >> _method >> _path >> _version;
		if (_method.empty() || _path.empty() || _version.empty()) {
			throw std::runtime_error("Invalid HTTP request line");
			// shouldnt throw but should return error response
		}
	}

	while (std::getline(iss, line)) {
		if (line.empty() || line == "\r")
			break;
		size_t pos = line.find(':');
		if (pos != std::string::npos) {
			std::string key = line.substr(0, pos);
			std::string val = line.substr(pos + 1);
			if (!key.empty()) {
				ft_strtrim(key);
				ft_strtrim(val);
				addHeader(key, val);
			}
			else {
				throw std::runtime_error("Invalid header format: " + line);
			// shouldnt throw but should return error response
			}
		}
		else {
			throw std::runtime_error("Invalid header format: " + line);
			// shouldnt throw but should return error response
		}
	}

	if (_headers.empty()) {
		throw std::runtime_error("No headers found in the request");
			// shouldnt throw but should return error response
	}
	// std::cout << "Parsed HTTP header." << std::endl;
	
	size_t pos = _buffer.find("\r\n\r\n");
	if (pos != std::string::npos) {
		_buffer.erase(0, pos + 4); // remove header part from buffer
	}
}

std::string MiniHttpRequest::getHeaderValue(const std::string& key) const {
	for (std::multimap<std::string, std::string>::const_iterator it = _headers.begin();
	it != _headers.end(); ++it) {
		if (strcasecmp(it->first.c_str(), key.c_str()) == 0) {
			return it->second;
		}
	}
	return "";
}

void MiniHttpRequest::getBodyType(bool& isChunked, long long& contentLength) {
	isChunked = false;
	contentLength = 0;

	std::string transferEncoding = getHeaderValue("Transfer-Encoding");
	if (transferEncoding == "chunked") {
		isChunked = true;
	} else {
		std::string contentLengthStr = getHeaderValue("Content-Length");
		if (!contentLengthStr.empty()) {
			try {
				contentLength = std::strtoll(contentLengthStr.c_str(), NULL, 10);
			} catch (const std::exception& e) {
				throw std::runtime_error("Invalid Content-Length value: " + contentLengthStr);
			}
		}
	}
}

void MiniHttpRequest::parseTrailer(std::string& chunk) {
	// char buffer[1024] = {0};
	// int bytes_read;
	//
	// while (chunk.find("\r\n\r\n") == std::string::npos) {
	// 	bytes_read = recv(_socket_fd, buffer, sizeof(buffer) - 1, 0);
	// 	if (bytes_read <= 0) {
	// 		throw std::runtime_error("Failed to read trailer from socket");
	// 	}
	// 	buffer[bytes_read] = '\0';
	// 	chunk.append(buffer);
	// }

	if (_buffer.find("\r\n\r\n") == std::string::npos) {
		// no trailer found
		return;
	}

	// std::string trailer = chunk.substr(0, chunk.find("\r\n\r\n"));
	std::istringstream iss(chunk);
	std::string line;

	while (std::getline(iss, line)) {
		if (line.empty() || line == "\r") {
			break;
		}
		size_t pos = line.find(':');
		if (pos != std::string::npos) {
			std::string key = line.substr(0, pos);
			std::string val = line.substr(pos + 1);
			if (!key.empty()) {
				ft_strtrim(key);
				ft_strtrim(val);
				addHeader(key, val);
			} else
				throw std::runtime_error("Invalid trailer header format: " + line);
		} else {
			throw std::runtime_error("Invalid trailer header format: " + line);
		}
	}
}

bool MiniHttpRequest::loadBody(bool isChunked, long long contentLength) {
	// size_t pos = _buffer.find("\r\n\r\n");
	// if (pos == std::string::npos) // dont really need this line because header is already checked
	// 	throw std::runtime_error("Invalid HTTP request format: no body found");
	// std::string temp_body = _buffer.substr(pos + 4);

	if (isChunked) {
		while (true) {
			size_t pos = _buffer.find("\r\n");
			if (pos == std::string::npos)
				return false;
			std::string chunkSizeStr = _buffer.substr(0, pos);
			long long chunkSize = std::strtoll(chunkSizeStr.c_str(), NULL, 16);
			if (chunkSize == 0) {
				_buffer.erase(0, pos + 2);
				parseTrailer(_buffer);
				return true;
			}
			if (_buffer.size() < pos + 2 + chunkSize + 2)
				return false;

			_body.append(_buffer.substr(pos + 2, chunkSize));
			_buffer.erase(0, pos + 2 + chunkSize + 2);
		}
	} else if (contentLength > 0) {
		if (_body.size() < static_cast<std::size_t>(contentLength)) {
			long long curSize = _body.size() + _buffer.size();
			if (curSize < contentLength) {
				_body.append(_buffer);
				_buffer.erase();
			} else {
				_body.append(_buffer.substr(0, contentLength - _body.size()));
				_buffer.erase(0, contentLength - _body.size());
			}
		}
	}
	else {
		std::cout << "No body to load." << std::endl;
		return true;
	}
	std::cout << "Loaded HTTP body." << std::endl;

	// maybe dont need body cout
	
	return true;
}

bool MiniHttpRequest::parseRequest() {
	std::cout << "Parsing HTTP request from socket [" << _socket.fd << "]" << std::endl;

	long long contentLength = 0;
	bool isChunked = false;

	if (!_isHeaderLoaded) {
		std::cout << "Loading HTTP header..." << std::endl;
		if (!loadHeader())
			return false;
		std::cout << "HTTP header loaded." << std::endl;
		std::cout << "\n_buffer: " << _buffer << std::endl;
		parseHeader();
		std::cout << "Parsed HTTP header." << std::endl;
		// shouldnt throw but should return error response
		_isHeaderLoaded = true;
	}

	_socket.keepAlive = !(getHeaderValue("Connection") == "close");
	
	getBodyType(isChunked, contentLength);
	if (!loadBody(isChunked, contentLength))
		return false;

	// std::cout << "Full HTTP request:\n" << request << std::endl;
	//
	std::cout << "Parsed HTTP request." << std::endl;
	std::cout << "Method: " << _method << std::endl;
	std::cout << "Path: " << _path << std::endl;
	std::cout << "Version: " << _version << std::endl;
	std::cout << "Body: " << _body << std::endl;

	return true;

	// after here we can do http response
	// for now just dummy response
	// std::string response = "HTTP/1.1 200 OK\r\n"
	// 	"Content-Type: text/plain\r\n"
	// 	"Content-Length: 13\r\n"
	// 	"\r\n"
	// 	"Hello, World!";
	// if (send(_socket_fd, response.c_str(), response.size(), 0) <
	//  			0) {
	// 	throw std::runtime_error("Failed to send response to socket");
	// }
	// std::cout << "Response sent to client." << std::endl;
	// close(_socket_fd);

}
