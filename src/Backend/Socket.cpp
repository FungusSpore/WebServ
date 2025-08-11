#include "../../includes/Socket.hpp"

Socket::Socket(int fd, std::string port):fd(fd), toSend(NULL), port(port){}

Socket::Socket(int fd, Socket* toSend):fd(fd), toSend(toSend){}

Socket::~Socket(){
	close(this->fd);
	if (!toSend)
		delete toSend;
}

bool Socket::operator==(const Socket& other){
	if (this->toSend == other.toSend && \
		this->fd == other.fd && \
		this->port == other.port)
		return (true);
	return (false);
}
