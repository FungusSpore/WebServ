#ifndef SOCKET_HPP
# define SOCKET_HPP

#include <string>
#include <unistd.h>

#define READ_BUFFER_SIZE 8192
#define WRITE_BUFFER_SIZE 8192

/// Socket is to store socket fd, and which port it connected to. 
/// Forward fd is used for server cgi data forwarding by default will be -1 and port will be left empty
struct Socket{
// private:
		// MiniHttp Prophet;
// 	Socket(const Socket&);
// 	Socket& operator=(const Socket&);
public:
	int		fd;
	int		clientFd;
	std::string port;
	std::string read_buffer;
	std::string write_buffer;
	// Socket *toSend;
	// uint32_t	events;
	// std::vector<char> read_buffer;
	// std::vector<char> write_buffer;

	Socket(int fd, std::string port);
	// Socket(int fd, int forward_fd);
	Socket(int fd, int clientFd);
	~Socket();
	bool operator==(const Socket& other) const;
};

#endif // !SOCKET_HPP
