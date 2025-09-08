#include "../../includes/Epoll.hpp"
#include <cerrno>
#include <cstddef>
#include <sys/epoll.h>

/// To setup what type of socket is needed
static void	setupHints(struct addrinfo& hints){
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
}

/// create a socket and try to bind it
static void	tryBindAddr(struct addrinfo& hints, int& listen_sock, const std::string& port){
	struct addrinfo *result, *rp;

	if (getaddrinfo(NULL, port.c_str(), &hints, &result) != 0)
		throw SystemFailure("Failed to get addr info");

	for (rp = result; rp != NULL; rp = rp->ai_next) {
		listen_sock = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol); 
		if (listen_sock == -1)
			continue ;
		// Set SO_REUSEADDR to allow immediate reuse of address
		int opt = 1;
		if (setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
			close(listen_sock);
			continue;
		}
		if (bind(listen_sock, rp->ai_addr, rp->ai_addrlen) == 0)
			break ;
		close(listen_sock);
	}
	if (rp == NULL){
		freeaddrinfo(result);
		throw SystemFailure("Listen Socket Failed to initialize");
	}
	freeaddrinfo(result);
}


Epoll::Epoll(const std::vector<std::string> port_list, WebServer& prophetServer):_clientRegistry(prophetServer), _nfds(0),_idx(0) , _server(prophetServer) {
	struct addrinfo hints;
	struct epoll_event ev; // epoll_ctl will make its own copy
	int listen_sock;

	setupHints(hints);
	_epollfd = epoll_create(1);// 1 is just a placeholder does nothing
	if (_epollfd == -1)
		throw SystemFailure("Epoll create failed");
	std::vector<std::string>::const_iterator it = port_list.begin();

	for (; it != port_list.end(); it++){
		tryBindAddr(hints, listen_sock, *it);
		if (listen(listen_sock, LISTEN_BACKLOG) == -1){
			close(listen_sock);
			throw SystemFailure("Failed listen on socket");
		}
		ev.events = EPOLLIN | EPOLLET;
		ev.data.ptr = _listenRegistry.makeSocket(listen_sock, *it, prophetServer);
		Utils::setnonblocking(listen_sock);
		if (epoll_ctl(_epollfd, EPOLL_CTL_ADD, listen_sock, &ev) == -1){
			close(listen_sock);
			throw SystemFailure("Epoll CTL failed to add listen socket");
		}
	}
}

Epoll::~Epoll(){ close(_epollfd); }

void	Epoll::get_new_events(){
	for (;;){
		_nfds = epoll_wait(_epollfd, _events, MAX_EVENTS, EPOLL_TIMEOUT);
		if (_nfds == -1 && errno != EINTR)
			throw SystemFailure("Epoll wait failed");
		else if (_nfds == 0)
			_clientRegistry.cleanRegistry();
		else
			break ;
	}
	_idx = 0;
}

std::vector<struct epoll_event> Epoll::get_conn_sock(){
	std::vector<struct epoll_event> result;
	struct epoll_event							ev;
	Socket*													sock;
	Socket*													clientSocket;
	int															conn_sock;

	_clientRegistry.cleanRegistry();
	if (_idx >= _nfds)
		get_new_events();
	for (; _idx < _nfds; _idx++){
		sock = (Socket *)_events[_idx].data.ptr;
		if (!_listenRegistry.searchSocket(sock)){
			result.push_back(_events[_idx]);
			continue ;
		}
		conn_sock = accept(sock->fd, NULL, NULL); 
		if (conn_sock == -1) 
			throw SystemFailure("Accept has failed");
		Utils::setnonblocking(conn_sock);
		ev.events = EPOLLIN | EPOLLET | EPOLLHUP | EPOLLERR;
		clientSocket = _clientRegistry.makeSocket(conn_sock, sock->port, _server);
		clientSocket->loadServerKey(conn_sock); // Load server key on the client socket
		ev.data.ptr = clientSocket;
		if (epoll_ctl(_epollfd, EPOLL_CTL_ADD, conn_sock, &ev) == -1)
			throw SystemFailure("Epoll CTL failed to add listen socket");
	}
	return (result);
}

int Epoll::get_epollfd() const{
	return (_epollfd);
}

Socket* Epoll::makeClientSocket(int fd, Socket* toSend){
	return _clientRegistry.makeSocket(fd, toSend, _server);
}

Socket* Epoll::makeClientSocket(int fd, std::string port){
	return _clientRegistry.makeSocket(fd, port, _server);
}

void Epoll::closeSocket(Socket& other){
	_clientRegistry.removeSocket(other);
}

void	Epoll::resetSocketTimer(Socket& other){
	_clientRegistry.resetSocketTimer(other);
}

//==========================//
//			EPOLL EVENTS				//
//==========================//

void Epoll::handle_epollin(Socket* mysock, struct epoll_event& event){
	if (IO::try_read(*this, event) == -1)
		return ;
	if (mysock->toSend != NULL && mysock->validateCGI()){
		struct epoll_event ev;
		ev.events = EPOLLIN | EPOLLET | EPOLLHUP | EPOLLERR;
		ev.data.ptr = mysock->toSend;
		IO::try_write(*this, ev);
		return ;
	}
	if (mysock->runHttp())
		IO::try_write(*this, event);
	else if (mysock->isCgi && !mysock->executeCGI(*this))
		IO::try_write(*this, event);
}

void Epoll::handle_epollout(struct epoll_event& event){
		IO::try_write(*this, event);
}

void Epoll::handle_epollhup(Socket* mysock, struct epoll_event& event){
	handle_epollin(mysock, event);
	this->closeSocket(*mysock);
}

void Epoll::handle_epollerr(Socket* mysock){
	// Get the specific socket error
	int socket_error = 0;
	socklen_t len = sizeof(socket_error);
	int sockfd = mysock->fd;
	if (getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &socket_error, &len) == 0 && socket_error != 0) {
			switch (socket_error) {
					case ECONNRESET:
							std::cout << "Connection reset by peer";
							break;
					case EPIPE:
							std::cout << "Broken pipe (remote end closed)";
							break;
					case ENOBUFS:
							std::cout << "No buffer space available";
							break;
					case ENOMEM:
							std::cout << "Out of memory";
							break;
					case ENETDOWN:
							std::cout << "Network is down";
							break;
					case ENETUNREACH:
							std::cout << "Network is unreachable";
							break;
					case EHOSTUNREACH:
							std::cout << "Host is unreachable";
							break;
					case ETIMEDOUT:
							std::cout << "Connection timed out";
							break;
					case ECONNREFUSED:
							std::cout << "Connection refused";
							break;
					case ENOTCONN:
							std::cout << "Socket is not connected";
							break;
					default:
							std::cout << strerror(socket_error) << " (errno: " << socket_error << ")";
							break;
			}
	} else
			std::cout << strerror(errno) << " (errno: " << errno << ")";
	std::cout << std::endl;
	this->closeSocket(*mysock);
}

// #include <iostream>
// int main(){
// 	try{
// 		std::vector<std::string> port_list;
// 		port_list.push_back("1234");
// 		port_list.push_back("1235");
// 		port_list.push_back("1236");
// 		Epoll test(port_list);
// 		char buffer[READ_BUFFER];
//
// 		// Will need a outer while loop
// 		for (;;){
// 			std::vector<Socket *> mysocks = test.get_conn_sock();
// 			for (size_t i = 0; i < mysocks.size(); i++){
// 				Socket *mysock = mysocks[i];
// 				while (true){
// 					int size = read(mysock->fd, buffer, READ_BUFFER);
// 					if (size == -1){
// 						if (errno == EAGAIN || errno == EWOULDBLOCK)
// 							break;
// 						throw  SystemFailure("Read Failed");
// 					}
// 					write(mysock->fd, buffer, size);
// 				}
// 				close(mysock->fd);
// 				delete mysock;
// 			}
// 		}
// 	}catch (std::exception& e){
// 		std::cout << e.what() << std::endl;
// 	}
// }
