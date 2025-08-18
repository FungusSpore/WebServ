#include "../../includes/Socket.hpp"
#include "MiniHttp.hpp"
#include "MiniHttpUtils.hpp"
#include "WebServer.hpp"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <cstring>
#include <iostream>

Socket::Socket(int fd, std::string port, WebServer& server): fd(fd), clientFd(-1), port(port), read_buffer(), write_buffer(), _serverKey("", port, ""), _ProphetHttp(*this, server) {
	this->read_buffer.reserve(READ_BUFFER_SIZE);
	this->write_buffer.reserve(WRITE_BUFFER_SIZE);
}

Socket::Socket(int fd, int clientFd, WebServer& server): fd(fd), clientFd(clientFd), port(""), read_buffer(), write_buffer(), _serverKey("", "", ""), _ProphetHttp(*this, server) {
	this->read_buffer.reserve(READ_BUFFER_SIZE);
	this->write_buffer.reserve(WRITE_BUFFER_SIZE);
}

Socket::Socket(const Socket& other): fd(other.fd), clientFd(other.clientFd), port(other.port), read_buffer(other.read_buffer), write_buffer(other.write_buffer), _serverKey(other._serverKey), _ProphetHttp(*this, other._ProphetHttp.getServer()) {}

Socket& Socket::operator=(const Socket& other){
	if (this != &other) {
		this->fd = other.fd;
		this->clientFd = other.clientFd;
		this->port = other.port;
		this->read_buffer = other.read_buffer;
		this->write_buffer = other.write_buffer;
		this->_serverKey = other._serverKey;
		// Note: _ProphetHttp contains references and cannot be assigned
		// this->_ProphetHttp = other._ProphetHttp;
	}
	return (*this);
}

Socket::~Socket(){
	// close(this->fd);
	// if (!toSend)
	// 	delete toSend;
}

bool Socket::operator==(const Socket& other) const{
	if (this->fd == other.fd)
		return (true);
	return (false);
}

void Socket::loadServerKey(int conn_sock) {
	struct sockaddr_in addr;
	socklen_t addr_len = sizeof(addr);
	
	memset(&addr, 0, sizeof(addr));

	if (getsockname(conn_sock, (struct sockaddr*)&addr, &addr_len) == -1) {
		throw std::runtime_error("Failed to get socket name");
	}
	
	char* ip_str = inet_ntoa(addr.sin_addr);
	
	// If the socket is bound to 0.0.0.0 (INADDR_ANY), use localhost instead
	if (addr.sin_addr.s_addr == INADDR_ANY) {
		_serverKey._ip = "0.0.0.0"; // either can choose to use "127.0.0.1" or "localhost", maybe server can detect all
	} else {
		_serverKey._ip = std::string(ip_str);
	}
	
	_serverKey._port = ft_toString(ntohs(addr.sin_port));
}

bool Socket::runHttp() {
	return _ProphetHttp.run();
}

bool Socket::validateCGI() {
	return _ProphetHttp.validateCGI();
}
