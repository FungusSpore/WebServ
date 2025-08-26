#include "../../includes/Socket.hpp"
#include "../../includes/MiniHttp.hpp"
#include "../../includes/MiniHttpUtils.hpp"
#include "../../includes/WebServer.hpp"
#include "../../includes/CGI.hpp"
#include <ctime>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <cstring>
#include <iostream>

Socket::Socket(int fd, std::string port, WebServer& server): \
	fd(fd), toSend(NULL), is_alive(true), port(port), read_buffer(), write_buffer(),\
	isCgi(false), cgiPath(""), cgiEnvs(), cgiBody(), keepAlive(true), _serverKey("", port, ""), _ProphetHttp(*this, server) {
	this->read_buffer.reserve(READ_BUFFER_SIZE);
	this->write_buffer.reserve(WRITE_BUFFER_SIZE);
	this->last_active = time(NULL);
}

Socket::Socket(int fd, Socket* toSend, WebServer& server): \
	fd(fd), toSend(toSend), is_alive(true), port(""), read_buffer(), write_buffer(),\
	isCgi(false), cgiPath(""), cgiEnvs(), cgiBody(), keepAlive(true), _serverKey("", "", ""), _ProphetHttp(*this, server) {
	this->read_buffer.reserve(READ_BUFFER_SIZE);
	this->write_buffer.reserve(WRITE_BUFFER_SIZE);
	this->last_active = time(NULL);
}

Socket::Socket(const Socket& other): \
	fd(other.fd), toSend(other.toSend), is_alive(true), port(other.port), read_buffer(other.read_buffer), \
	write_buffer(other.write_buffer), isCgi(other.isCgi), cgiPath(other.cgiPath), cgiEnvs(other.cgiEnvs), \
	cgiBody(other.cgiBody), keepAlive(other.keepAlive), _serverKey(other._serverKey), _ProphetHttp(*this, other._ProphetHttp.getServer()) {}

Socket& Socket::operator=(const Socket& other){
	if (this != &other) { this->fd = other.fd;
		this->toSend = other.toSend;
		this->is_alive = other.is_alive;
		this->last_active = other.last_active;
		this->port = other.port;
		this->read_buffer = other.read_buffer;
		this->write_buffer = other.write_buffer;
		this->isCgi = other.isCgi;
		this->cgiPath = other.cgiPath;
		this->cgiEnvs = other.cgiEnvs;
		this->cgiBody = other.cgiBody;
		this->keepAlive = other.keepAlive;
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

bool Socket::operator<(const Socket& other) const{
	return (this->last_active < other.last_active);
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
	if (_ProphetHttp.run() && isCgi)
		return false;
	return true;
}

bool Socket::validateCGI() {
	return _ProphetHttp.validateCGI();
}

bool Socket::executeCGI(Epoll& epoll) {
	try {
		std::vector<char*> envp;
		for (size_t i = 0; i < cgiEnvs.size(); ++i) {
			// std::cout << cgiEnvs[i] << std::endl;
			envp.push_back(const_cast<char*>(cgiEnvs[i].c_str()));
		}
		envp.push_back(NULL);

		// for (size_t i = 0; i < envp.size(); ++i) {
		// 	std::cout << "CGI Env: " << envp[i] << std::endl;
		// }
		
		struct epoll_event inputSocket = CGI::exec(cgiPath.c_str(), &envp[0], epoll, *this);
		
		// Send the cgibody to CGI process (for POST requests)
		std::cout << "CGI BODY" << std::endl;
		if (cgiBody.empty()) {
			epoll.closeSocket(*static_cast<Socket*>(inputSocket.data.ptr));
		}
		else{
			// std::cout << std::string(cgiBody.begin(), cgiBody.end()) << std::endl;
			static_cast<Socket*>(inputSocket.data.ptr)->write_buffer = cgiBody;
			IO::try_write(epoll, inputSocket);
		}
		
		// isCgi = false;
		// read_buffer.clear();
		// cgiBody.clear();
		
		return true;
		
	} catch (const std::exception& e) {
		std::cerr << "CGI execution failed: " << e.what() << std::endl;
		// reason for try catch is only for when throw from CGI::exec
		std::string errStr = "HTTP/1.1 500 Internal Server Error\r\n"
					   "Content-Type: text/html\r\n"
					   "Content-Length: 50\r\n"
					   "Connection: close\r\n"
					   "\r\n"
					   "<html><body><h1>500 Internal Server Error</h1></body></html>";
		write_buffer.assign(errStr.begin(), errStr.end());

		isCgi = false;
		cgiBody.clear();
		return false;
	}
}
