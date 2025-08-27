#include "MiniHttpRequest.hpp"
#include "MiniHttpUtils.hpp"
#include <cstddef>
#include <exception>
#include <iostream>
#include <iterator>
#include <ostream>
#include <unistd.h>
#include <cstring>
#include <sys/socket.h>
#include <cstdlib>
#include <sstream>
#include "Socket.hpp"

MiniHttpRequest::MiniHttpRequest(Socket& socket) 
	: _socket(socket), _method(""), _path(""), _version(""), _trailer(""), _buffer(), _body(), _headers(), _isHeaderLoaded(false), _isErrorCode(-1) {}

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

const std::vector<char>& MiniHttpRequest::getBody() const {
	return _body;
}

const std::string& MiniHttpRequest::getTrailer() const {
	return _trailer;
}

const std::multimap<std::string, std::string>& MiniHttpRequest::getHeaders() const {
	return _headers;
}

int MiniHttpRequest::getErrorCode() const {
	return _isErrorCode;
}


void MiniHttpRequest::addHeader(const std::string& key, const std::string& value) {
	_headers.insert(std::make_pair(key, value));
}

void MiniHttpRequest::clearRequest() {
	_method.clear();
	_path.clear();
	_version.clear();
	_trailer.clear();
	_buffer.clear();
	_body.clear();
	_headers.clear();
	_isHeaderLoaded = false;
	_isErrorCode = -1;
}

bool MiniHttpRequest::loadHeader() {
	std::cout << "Finding delimeter....." << std::endl;
	if (ft_vectorFind(_buffer, "\r\n\r\n") != std::string::npos) {
		return true;
	}
	// std::cout << "Buffer content :" << std::string(_buffer.begin(), _buffer.end()) << std::endl;
	// std::cout << "Socket content :" << std::string(_socket.read_buffer.begin(), _socket.read_buffer.end()) << std::endl;
	//
	// if (ft_vectorFind(_socket.read_buffer, M_CRLF2) == std::string::npos)
	// 	std::cout << "Found M_CRLF2" << std::endl;
	// if (ft_vectorFind(_socket.read_buffer, "\n\n") == std::string::npos)
	// 	std::cout << "Found linux" << std::endl;

	std::cout << "Finding delimeter Again....." << std::endl;
	if (ft_vectorFind(_socket.read_buffer, M_CRLF2) == std::string::npos) {
		std::cout << "insert rest into buffer haven't found header....." << std::endl;
		_buffer.insert(_buffer.end(), _socket.read_buffer.begin(), _socket.read_buffer.end());
		_socket.read_buffer.clear();
		return false;
	}
	
	std::cout << "Append rest of content to http buffer found header...." << std::endl;
	_buffer.insert(_buffer.end(), _socket.read_buffer.begin(), _socket.read_buffer.end());
	_socket.read_buffer.clear();
	return true;



	// if (_buffer.find("\r\n\r\n") != std::string::npos) {
	// 	return true;
	// }
	// if (_socket.read_buffer.find("\r\n\r\n") == std::string::npos) {
	// 	_buffer += _socket.read_buffer;
	// 	_socket.read_buffer.erase();
	// 	return false;
	// }
	// _buffer += _socket.read_buffer;
	// _socket.read_buffer.erase();
	// return true;
}

void MiniHttpRequest::parseHeader() {
	std::istringstream iss(std::string(_buffer.begin(), _buffer.end()));
	std::string line;

	if (std::getline(iss, line)) {
		std::istringstream methods(line);
		methods >> _method >> _path >> _version;
		if (_method.empty() || _path.empty() || _version.empty()) {
			_isErrorCode = 400;
			throw std::runtime_error("Invalid HTTP request line");
		}
		if (_method != "GET" && _method != "POST" && _method != "PUT" && _method != "DELETE" && _method != "HEAD") {
			_isErrorCode = 405;
			throw std::runtime_error("Method not allowed: " + _method);
		}
		if (_version != "HTTP/1.1") {
			_isErrorCode = 505;
			throw std::runtime_error("HTTP version not supported: " + _version);
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
				_isErrorCode = 400;
				throw std::runtime_error("Invalid header format: " + line);
			}
		}
		else {
			_isErrorCode = 400;
			throw std::runtime_error("Invalid header format: " + line);
		}
	}

	if (_headers.empty()) {
		_isErrorCode = 400;
		throw std::runtime_error("No headers found in the request");
	}
	// std::cout << "Parsed HTTP header." << std::endl;
	
	size_t pos = ft_vectorFind(_buffer, M_CRLF2);
	if (pos != std::string::npos && _buffer.size() >= pos + 4) {
		_buffer.erase(_buffer.begin(), _buffer.begin() + pos + 4);
	}
	// std::cout << "\n_buffer after header parse: " << std::string(_


	// size_t pos = _buffer.find("\r\n\r\n");
	// if (pos != std::string::npos) {
	// 	_buffer.erase(0, pos + 4); // remove header part from buffer
	// }
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
				_isErrorCode = 400;
				throw std::runtime_error("Invalid Content-Length value: " + contentLengthStr);
			}
		}
	}
}

void MiniHttpRequest::parseTrailer() {
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

	if (_buffer.empty() || ft_vectorFind(_buffer, M_CRLF2) == std::string::npos) {
		// no trailer found
		return;
	}

	// std::string trailer = chunk.substr(0, chunk.find("\r\n\r\n"));
	std::istringstream iss(std::string(_buffer.begin(), _buffer.end()));
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
			} else {
				_isErrorCode = 400;
				throw std::runtime_error("Invalid trailer header format: " + line);
			}
		} else {
			_isErrorCode = 400;
			throw std::runtime_error("Invalid trailer header format: " + line);
		}
	}
}

bool MiniHttpRequest::loadBody(bool isChunked, long long contentLength) {
	// size_t pos = _buffer.find("\r\n\r\n");
	// if (pos == std::string::npos) // dont really need this line because header is already checked
	// 	throw std::runtime_error("Invalid HTTP request format: no body found");
	// std::string temp_body = _buffer.substr(pos + 4);
	
	_buffer.insert(_buffer.end(),_socket.read_buffer.begin(),_socket.read_buffer.end());
	_socket.read_buffer.clear();

	if (isChunked) {
		while (true) {
			size_t pos = ft_vectorFind(_buffer, M_CRLF);
			if (pos == std::string::npos)
				return false;
			// std::string chunkSizeStr = _buffer.substr(0, pos);
			std::string chunkSizeStr(_buffer.begin(), _buffer.begin() + pos);
			char* endPtr = NULL;
			long long chunkSize = std::strtoll(chunkSizeStr.c_str(), &endPtr, 16);
			if (endPtr == chunkSizeStr.c_str() || *endPtr != '\0') {
				_isErrorCode = 400;
				throw std::runtime_error("Invalid chunk size: " + chunkSizeStr);
			}
			if (chunkSize == 0) {
				// _buffer.erase(0, pos + 2);
				_buffer.erase(_buffer.begin(), _buffer.begin() + pos + 2);
				parseTrailer();
				return true;
			}
			if (_buffer.size() < pos + 2 + chunkSize + 2) {
				_isErrorCode = 400;
				return false;
			}

			// _body.append(_buffer.substr(pos + 2, chunkSize));
			// _buffer.erase(0, pos + 2 + chunkSize + 2);
			_body.insert(_body.end(), _buffer.begin() + pos + 2, _buffer.begin() + pos + 2 + chunkSize);
			_buffer.erase(_buffer.begin(), _buffer.begin() + pos + 2 + chunkSize + 2);
		}
	} else if (contentLength > 0) {
		if (_body.size() < static_cast<std::size_t>(contentLength)) {
			long long curSize = _body.size() + _buffer.size();
			if (curSize < contentLength) {
				// _body.append(_buffer);
				// _buffer.erase();
				_body.insert(_body.end(), _buffer.begin(), _buffer.end());
				_buffer.clear();
			} else {
				// _body.append(_buffer.substr(0, contentLength - _body.size()));
				// _buffer.erase(0, contentLength - _body.size());
				_body.insert(_body.end(), _buffer.begin(), _buffer.begin() + (contentLength - _body.size()));
				_buffer.erase(_buffer.begin(), _buffer.begin() + (contentLength - _body.size()));
			}
		}
		if (_body.size() < static_cast<std::size_t>(contentLength)) {
			// _isErrorCode = 400;
			// throw std::runtime_error("Incomplete body received: expected " + ft_toString(contentLength) + " bytes, got " + ft_toString(_body.size()) + " bytes");
			std::cout << "false body size : " << _body.size() << " contentLength: " << contentLength << std::endl;
			std::cout << "false buffer size : " << _buffer.size() << std::endl;
			std::cout << "false socket size : " << _socket.read_buffer.size() << std::endl;
			return false;
		}
		if (!_socket.keepAlive && !_buffer.empty()) {
			_isErrorCode = 400;
			throw std::runtime_error("Unexpected data after body: " + std::string(_buffer.begin(), _buffer.end()));
		}
	}
	else {
		// std::cout << "No body to load." << std::endl;
		return true;
	}
	// std::cout << "Loaded HTTP body." << std::endl;
	// maybe dont need body cout
	return true;
}

bool MiniHttpRequest::parseRequest() {
	std::cout << "===Parsing HTTP request from socket [" << _socket.fd << "]===" << std::endl;

	long long contentLength = 0;
	bool isChunked = false;

	try {
		if (!_isHeaderLoaded) {
			std::cout << "Loading HTTP header..." << std::endl;
			if (!loadHeader())
				return false;
			std::cout << "HTTP header loaded." << std::endl;
			// std::cout << "\n_buffer: " << _buffer << std::endl;
			parseHeader();
			std::cout << "Parsed HTTP header." << std::endl;
			// shouldnt throw but should return error response
			_isHeaderLoaded = true;
		}

		_socket.keepAlive = !(getHeaderValue("Connection") == "close");
		
		std::cout << "Getting Body Type..." << std::endl;
		getBodyType(isChunked, contentLength);
		std::cout << "Loading Body..." << std::endl;
		if (!loadBody(isChunked, contentLength))
			return false;
		std::cout << "Done loading body.." << std::endl;

	} catch (const std::exception& e) {
		std::cout << "Error parsing HTTP request: " << e.what() << std::endl;
		if (_isErrorCode > 0)
			return true;
	}

	// std::cout << "Full HTTP request:\n" << request << std::endl;

	std::cout << "\n===Parsed HTTP request.===" << std::endl;
	std::cout << "Method: " << _method << std::endl;
	std::cout << "Path: " << _path << std::endl;
	std::cout << "Version: " << _version << std::endl;
	std::cout << "Body size: " << _body.size() << std::endl;
	// std::cout << "Body: " << std::string(_body.begin(), _body.end()) << std::endl;
	std::cout << "===End===\n" << std::endl;

	return true;
}
