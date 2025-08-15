#include "../../includes/Socket.hpp"

Socket::Socket(int fd, std::string port):fd(fd), clientFd(-1), port(port){
	this->read_buffer.reserve(READ_BUFFER_SIZE);
	this->write_buffer.reserve(WRITE_BUFFER_SIZE);
}

Socket::Socket(int fd, int clientFd):fd(fd), clientFd(clientFd){
	this->read_buffer.reserve(READ_BUFFER_SIZE);
	this->write_buffer.reserve(WRITE_BUFFER_SIZE);
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
