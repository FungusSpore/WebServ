#ifndef MINIHTTP_HPP
#define MINIHTTP_HPP

#include "MiniHttpRequest.hpp"
#include "MiniHttpResponse.hpp"
#include "WebServer.hpp"
#include <iostream>
// #include "Epoll.hpp"

struct Socket;

class MiniHttp {
private:
	// MiniSocket _socket;
	// MiniHttpRequest _request;
	// MiniHttpResponse _response;
	// MiniHttpRoute _route;
	Socket& _socket;
	// int _socket_fd;
	WebServer& _server;

	// Copy constructor and assignment operator are declared private and not implemented
	// to prevent copying objects with reference members (C++98 idiom)
	MiniHttp(const MiniHttp& other);
	MiniHttp& operator=(const MiniHttp& other);

	// void sendResponse(MiniHttpResponse& response);
public:
	MiniHttp(Socket& socket, WebServer& server);
	// MiniHttp(int socket_fd, WebServer& server);
	~MiniHttp();

	bool run();
	bool validateCGI();
	
	// Getter for server reference
	WebServer& getServer() const { return _server; }
};

#endif
