#ifndef SOCKET_HPP
# define SOCKET_HPP

#include <string>
#include <unistd.h>
#include "MiniHttp.hpp"
#include "WebServer.hpp"

// class MiniHttp;
// class Webserver;
class Epoll;

#define READ_BUFFER_SIZE 8192
#define WRITE_BUFFER_SIZE 8192

/// Socket is to store socket fd, and which port it connected to. 
/// Forward fd is used for server cgi data forwarding by default will be -1 and port will be left empty
struct Socket{
public:
	int		fd;
	int		clientFd;
	std::string port;
	std::string read_buffer;
	std::string write_buffer;

	bool isCgi;
	std::string cgiPath;
	std::vector<std::string> cgiEnvs;
	std::string cgiBody;

	bool keepAlive;

	Socket(int fd, std::string port, WebServer& server);
	// Socket(int fd, int forward_fd);
	Socket(int fd, int clientFd, WebServer& server);
	Socket(const Socket& other);
	Socket& operator=(const Socket& other);
	~Socket();
	bool operator==(const Socket& other) const;

	bool runHttp();
	bool validateCGI();
	bool executeCGI(Epoll& epoll);

	void loadServerKey(int conn_sock);
	ServerKey& getServerKey() { return _serverKey; }

private:
	ServerKey _serverKey;
	MiniHttp _ProphetHttp;

};

#endif // !SOCKET_HPP
