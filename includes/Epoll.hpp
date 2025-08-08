#ifndef EPOLL_HPP
# define EPOLL_HPP

#include "Exceptions.hpp"
		
#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include <cstring>
#include <string>
#include <vector>
#include <algorithm>

#include <fcntl.h>
#include <cerrno>
#include <unistd.h>

#define MAX_EVENTS 10
#define LISTEN_BACKLOG 128
#define READ_BUFFER 2048

/// Socket is to store socket fd, and which port it connected to. 
/// Forward fd is used for server cgi data forwarding by default will be -1 and port will be left empty
struct Socket{
	int		fd;
	int		forward_fd; // where to send the data to
	std::string port;

	Socket(int fd, std::string port);
	Socket(int fd, int forward_fd);
};

class Epoll{
private:
	struct epoll_event events[MAX_EVENTS];
	std::vector<int> listen_socks;
	int nfds, epollfd, idx;
	void get_new_events();

public:
	Epoll(const std::vector<std::string> port_list);
	Epoll(const Epoll& other);
	Epoll& operator=(const Epoll& other);
	~Epoll();
	std::vector<Socket *> get_conn_sock();
	Socket *get_conn_sock2();

	int get_epollfd() const;
	// void add_socket();
};

#endif
