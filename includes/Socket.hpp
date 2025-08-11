#ifndef SOCKET_HPP
# define SOCKET_HPP

#include <string>
#include <unistd.h>

/// Socket is to store socket fd, and which port it connected to. 
/// Forward fd is used for server cgi data forwarding by default will be -1 and port will be left empty
struct Socket{
	int		fd;
	Socket *toSend;
	// int		forward_fd; // where to send the data to
	std::string port;

	Socket(int fd, std::string port);
	// Socket(int fd, int forward_fd);
	Socket(int fd, Socket *toSend);
	~Socket();
	bool operator==(const Socket& other);
};

#endif // !SOCKET_HPP
