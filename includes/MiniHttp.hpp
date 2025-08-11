#ifndef MINIHTTP_HPP
#define MINIHTTP_HPP

#include "MiniHttpRequest.hpp"
#include "MiniHttpResponse.hpp"
#include "WebServer.hpp"
#include <iostream>
#include "Epoll.hpp"

class MiniHttp {
private:
	// MiniSocket _socket;
	// MiniHttpRequest _request;
	// MiniHttpResponse _response;
	// MiniHttpRoute _route;
	// int _socket_fd;
	Socket _socket;
	WebServer& _server;

	void handleRequest();
	void sendResponse();
public:
	MiniHttp(int socket_fd, WebServer& server);
	MiniHttp(const MiniHttp& other);
	MiniHttp& operator=(const MiniHttp& other);
	~MiniHttp();

	void run();
};

#endif
