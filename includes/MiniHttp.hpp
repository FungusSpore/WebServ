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
	Socket& _socket;
	WebServer& _server;
	MiniHttpRequest _request;

	// Disallow copy and assignment
	MiniHttp(const MiniHttp& other);
	MiniHttp& operator=(const MiniHttp& other);

	// CGI Helper
	std::string getCgiHeader(std::vector<char>& cgiBuffer);

	// Cookie Handling
	void parseCgiCookie(std::string& cgiHeaders);

public:
	MiniHttp(Socket& socket, WebServer& server);
	~MiniHttp();

	bool run();
	bool validateCGI();
	
	// Getter for server reference
	WebServer& getServer() const { return _server; }
};

#endif
