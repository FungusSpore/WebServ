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

struct Socket{
	int		fd;
	std::string port;

	Socket(int fd, std::string port);
};

class Epoll{
private:
	struct epoll_event events[MAX_EVENTS];
	std::vector<int> listen_socks;
	int nfds, epollfd, idx;
	void get_new_events();
	void setnonblocking(int socket);

public:
	Epoll(const std::vector<std::string> port_list);
	Epoll(const Epoll& other);
	Epoll& operator=(const Epoll& other);
	~Epoll();
	std::vector<Socket *> get_conn_sock();
	Socket *get_conn_sock2();
};

#endif
