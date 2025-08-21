#ifndef EPOLL_HPP
# define EPOLL_HPP

#include "Exceptions.hpp"
#include "SocketRegistry.hpp"
#include "WebServer.hpp"
		
#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include <cstring>
#include <string>
#include <vector>

#include <fcntl.h>
#include <cerrno>
#include <unistd.h>

#define MAX_EVENTS 10
#define EPOLL_TIMEOUT 5
#define LISTEN_BACKLOG 128
#define READ_BUFFER 2048


class Epoll{
private:
	SocketRegistry _clientRegistry;
	SocketRegistry _listenRegistry;
	struct epoll_event _events[MAX_EVENTS];
	int _nfds, _epollfd, _idx;
	void get_new_events();

	WebServer& _server;

public:
	Epoll(const std::vector<std::string> port_list, WebServer& server);
	Epoll(const Epoll& other);
	Epoll& operator=(const Epoll& other);
	~Epoll();

	std::vector<struct epoll_event> get_conn_sock();
	int get_epollfd() const;
	void closeSocket(Socket& other);
	Socket* makeClientSocket(int fd, int clientFd);
	void	resetSocketTimer(Socket& other);
};

#endif
