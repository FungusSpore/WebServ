#include "../../includes/Socket.hpp"
#include "MiniHttp.hpp"
#include "WebServer.hpp"

Socket::Socket(int fd, std::string port, WebServer& server): _ProphetHttp(*this, server), fd(fd), clientFd(-1), port(port) {
	this->read_buffer.reserve(READ_BUFFER_SIZE);
	this->write_buffer.reserve(WRITE_BUFFER_SIZE);
}

Socket::Socket(int fd, int clientFd, WebServer& server): _ProphetHttp(*this, server), fd(fd), clientFd(clientFd){
	this->read_buffer.reserve(READ_BUFFER_SIZE);
	this->write_buffer.reserve(WRITE_BUFFER_SIZE);
}

Socket::Socket(const Socket& other): _ProphetHttp(other._ProphetHttp), fd(other.fd), clientFd(other.clientFd), port(other.port), read_buffer(other.read_buffer), write_buffer(other.write_buffer) {}

Socket& Socket::operator=(const Socket& other){
	if (this != &other) {
		this->fd = other.fd;
		this->clientFd = other.clientFd;
		this->port = other.port;
		this->read_buffer = other.read_buffer;
		this->write_buffer = other.write_buffer;
		this->_ProphetHttp = other._ProphetHttp;
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
