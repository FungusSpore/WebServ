#include "MiniHttpRequest.hpp"
#include "Utils.hpp"
#include <exception>
#include <iostream>
#include <unistd.h>
#include <cstring>
#include <sys/socket.h>
#include <cstdlib>
#include <sstream>

MiniHttpRequest::MiniHttpRequest(int socket_fd) : _method(""), _path(""), _version(""), _body(""), _socket_fd(socket_fd) {
	// here maybe can start the parsing of the request
}

MiniHttpRequest::MiniHttpRequest(const MiniHttpRequest& other) 
	: _method(other._method), _path(other._path), _version(other._version), 
	  _headers(other._headers), _body(other._body), _socket_fd(other._socket_fd) {
}

MiniHttpRequest& MiniHttpRequest::operator=(const MiniHttpRequest& other) {
	if (this != &other) {
		_method = other._method;
		_path = other._path;
		_version = other._version;
		_headers = other._headers;
		_body = other._body;
		_socket_fd = other._socket_fd;
	}
	return *this;
}

MiniHttpRequest::~MiniHttpRequest() {
	if (_socket_fd != -1) {
		close(_socket_fd);
	}
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

const std::multimap<std::string, std::string>& MiniHttpRequest::getHeaders() const {
	return _headers;
}

const std::string& MiniHttpRequest::getBody() const {
	return _body;
}

int MiniHttpRequest::getSocketFd() const {
	return _socket_fd;
}

void MiniHttpRequest::setMethod(const std::string& method) {
	_method = method;
}

void MiniHttpRequest::setPath(const std::string& path) {
	_path = path;
}

void MiniHttpRequest::setVersion(const std::string& version) {
	_version = version;
}

void MiniHttpRequest::addHeader(const std::string& key, const std::string& value) {
	_headers.insert(std::make_pair(key, value));
}

void MiniHttpRequest::setBody(const std::string& body) {
	_body = body;
}

void MiniHttpRequest::setSocketFd(int socket_fd) {
	_socket_fd = socket_fd;
}

void MiniHttpRequest::loadHeader(std::string& request) {
	int bytes_read;
	char buffer[1024] = {0};

	while ((bytes_read = recv(_socket_fd, buffer, sizeof(buffer) -1, 0)) > 0) {
		buffer[bytes_read] = '\0';
		request.append(buffer);

		if (request.find("\r\n\r\n") != std::string::npos) {
			break;
		}
	}

	if (bytes_read < 0)
		throw std::runtime_error("Failed to read from socket");
	else if (bytes_read == 0)
		throw std::runtime_error("Connection closed by client");
	if (request.empty())
		throw std::runtime_error("Received empty request");
}

void MiniHttpRequest::parseHeader(const std::string& request) {
	std::istringstream iss(request);
	std::string line;

	if (std::getline(iss, line)) {
		std::istringstream methods(line);
		methods >> _method >> _path >> _version;
		if (_method.empty() || _path.empty() || _version.empty()) {
			throw std::runtime_error("Invalid HTTP request line");
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
			}
		}
		else {
			throw std::runtime_error("Invalid header format: " + line);
		}
	}

	if (_headers.empty()) {
		throw std::runtime_error("No headers found in the request");
	}
	// std::cout << "Parsed HTTP header." << std::endl;
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
	char buffer[1024] = {0};
	int bytes_read;

	while (chunk.find("\r\n\r\n") == std::string::npos) {
		bytes_read = recv(_socket_fd, buffer, sizeof(buffer) - 1, 0);
		if (bytes_read <= 0) {
			throw std::runtime_error("Failed to read trailer from socket");
		}
		buffer[bytes_read] = '\0';
		chunk.append(buffer);
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

void MiniHttpRequest::loadBody(bool isChunked, long long contentLength) {
	int bytes_read;
	char buffer[1024] = {0};

	if (isChunked) {
		std::string chunk;
		while (true) {
			bytes_read = recv(_socket_fd, buffer, sizeof(buffer) - 1, 0);
			if (bytes_read <= 0) {
				throw std::runtime_error("Failed to read chunked body from socket");
			}
			buffer[bytes_read] = '\0';
			chunk.append(buffer);

			while (true) {
				size_t pos = chunk.find("\r\n");
				if (pos == std::string::npos)
					break;
				std::string chunkSizeStr = chunk.substr(0, pos);
				// chunk.erase(0, pos + 2);
				long long chunkSize = std::strtoll(chunkSizeStr.c_str(), NULL, 16);
				if (chunkSize == 0) {
					chunk.erase(0, pos + 2);
					parseTrailer(chunk);
					return;
				}
				if (chunk.size() < pos + 2 + chunkSize + 2)
					break;

				_body.append(chunk.substr(pos + 2, chunkSize));
				chunk.erase(0, pos + 2 + chunkSize + 2);
			}
		}
	} else if (contentLength > 0) {
		long long bytesRead = 0;
		while (bytesRead < contentLength) {
			bytes_read = recv(_socket_fd, buffer, sizeof(buffer) - 1, 0);
			if (bytes_read <= 0) {
				throw std::runtime_error("Failed to read body from socket");
			}
			buffer[bytes_read] = '\0';
			_body.append(buffer);
			bytesRead += bytes_read;

			if (bytesRead >= contentLength) {
				break;
			}
		}
	}
	else {
		std::cout << "No body to load." << std::endl;
		return;
	}
	std::cout << "Loaded HTTP body." << std::endl;

	// maybe dont need body cout
}

void MiniHttpRequest::parseRequest() {
	std::cout << "Parsing HTTP request from socket " << _socket_fd << std::endl;

	std::string request;
	// long long contentLength = 0;
	// bool isChunked = false;

	loadHeader(request);
	parseHeader(request);
	// getBodyType(isChunked, contentLength);
	// loadBody(isChunked, contentLength);

	std::cout << "Full HTTP request:\n" << request << std::endl;

	std::cout << "Parsed HTTP request." << std::endl;
	std::cout << "Method: " << _method << std::endl;
	std::cout << "Path: " << _path << std::endl;
	std::cout << "Version: " << _version << std::endl;
	std::cout << "Body: " << _body << std::endl;

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
