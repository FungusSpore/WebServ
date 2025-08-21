#include "../../includes/Socket.hpp"
#include "MiniHttp.hpp"
#include "MiniHttpUtils.hpp"
#include "WebServer.hpp"
#include <ctime>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include "CGI.hpp"

Socket::Socket(int fd, std::string port, WebServer& server): \
	fd(fd), clientFd(-1), is_alive(true), port(port), read_buffer(), write_buffer(), isCgi(false), cgiPath(""), cgiEnvs(), cgiBody(""), keepAlive(true), _serverKey("", port, ""), _ProphetHttp(*this, server) {
	this->read_buffer.reserve(READ_BUFFER_SIZE);
	this->write_buffer.reserve(WRITE_BUFFER_SIZE);
	this->last_active = time(NULL);
}

Socket::Socket(int fd, int clientFd, WebServer& server): \
	fd(fd), clientFd(clientFd), is_alive(true), port(""), read_buffer(), write_buffer(), isCgi(false), cgiPath(""), cgiEnvs(), cgiBody(""), keepAlive(true), _serverKey("", "", ""), _ProphetHttp(*this, server) {
	this->read_buffer.reserve(READ_BUFFER_SIZE);
	this->write_buffer.reserve(WRITE_BUFFER_SIZE);
	this->last_active = time(NULL);
}

Socket::Socket(const Socket& other): \
	fd(other.fd), clientFd(other.clientFd), is_alive(true), port(other.port), read_buffer(other.read_buffer), write_buffer(other.write_buffer), isCgi(other.isCgi), cgiPath(other.cgiPath), cgiEnvs(other.cgiEnvs), cgiBody(other.cgiBody), keepAlive(other.keepAlive), _serverKey(other._serverKey), _ProphetHttp(*this, other._ProphetHttp.getServer()) {}

Socket& Socket::operator=(const Socket& other){
	if (this != &other) {
		this->fd = other.fd;
		this->clientFd = other.clientFd;
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
			envp.push_back(const_cast<char*>(cgiEnvs[i].c_str()));
		}
		envp.push_back(NULL);

		// for (size_t i = 0; i < envp.size(); ++i) {
		// 	std::cout << "CGI Env: " << envp[i] << std::endl;
		// }
		
		int cgiFd = CGI::exec(cgiPath.c_str(), &envp[0], epoll, *this);
		
		// Send the cgibody to CGI process (for POST requests)
		if (!cgiBody.empty()) {
			ssize_t written = write(cgiFd, cgiBody.c_str(), cgiBody.size());
			if (written == -1) {
				std::cerr << "Failed to write request body to CGI process" << std::endl;
			} else if (written < static_cast<ssize_t>(cgiBody.size())) {
				std::cerr << "Partial write to CGI process: " << written << "/" << cgiBody.size() << std::endl;
			}
		}
		
		isCgi = false;
		read_buffer.clear();
		cgiBody.clear();
		
		return true;
		
	} catch (const std::exception& e) {
		std::cerr << "CGI execution failed: " << e.what() << std::endl;
		// reason for try catch is only for when throw from CGI::exec
		write_buffer = "HTTP/1.1 500 Internal Server Error\r\n"
					   "Content-Type: text/html\r\n"
					   "Content-Length: 50\r\n"
					   "Connection: close\r\n"
					   "\r\n"
					   "<html><body><h1>500 Internal Server Error</h1></body></html>";
		
		isCgi = false;
		cgiBody.clear();
		return false;
	}
}
