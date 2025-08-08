#include "../../includes/Epoll.hpp"
#include "../../includes/Utils.hpp"

Socket::Socket(int fd, std::string port):fd(fd), forward_fd(-1), port(port){}

Socket::Socket(int fd, int forward_fd):fd(fd), forward_fd(forward_fd), port(""){}

Epoll::Epoll(const std::vector<std::string> port_list):nfds(0),idx(0){
	struct addrinfo hints, *result, *rp;
	struct epoll_event ev; // epoll_ctl will make its own copy
	int listen_sock;

	// set type of socket needed
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	this->epollfd = epoll_create(1);// 1 is just a placeholder does nothing
	if (this->epollfd == -1)
		throw SystemFailure("Epoll create failed");

	std::vector<std::string>::const_iterator it = port_list.begin();
	for (; it != port_list.end(); it++){
		// try and get a list of potential addrs based on hints
		if (getaddrinfo(NULL, it->c_str(), &hints, &result) != 0)
			throw SystemFailure("Failed to get addr info");

		// try and get a socket and bind it
		for (rp = result; rp != NULL; rp = rp->ai_next) {
			listen_sock = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol); 
			if (listen_sock == -1)
				continue ;
			if (bind(listen_sock, rp->ai_addr, rp->ai_addrlen) == 0)
				break ;
			close(listen_sock);
		}

		if (rp == NULL){
			freeaddrinfo(result);
			throw SystemFailure("Listen Socket Failed to initialize");
		}
		freeaddrinfo(result);

		if (listen(listen_sock, LISTEN_BACKLOG) == -1){
			close(listen_sock);
			throw SystemFailure("Failed listen on socket");
		}

		ev.events = EPOLLIN | EPOLLET;
		ev.data.ptr = new Socket(listen_sock, *it);
		Utils::setnonblocking(listen_sock);
		if (epoll_ctl(epollfd, EPOLL_CTL_ADD, listen_sock, &ev) == -1){
			close(listen_sock);
			throw SystemFailure("Epoll CTL failed to add listen socket");
		}

		this->listen_socks.push_back(listen_sock);
	}
}

Epoll::~Epoll(){
	close(this->epollfd);
	for (size_t i = 0; i < this->listen_socks.size(); i++)
		close(this->listen_socks.at(i));
}

void	Epoll::get_new_events(){
		this->nfds = epoll_wait(this->epollfd, this->events, MAX_EVENTS, -1);
		if (this->nfds == -1) throw SystemFailure("Epoll wait failed");
		this->idx = 0;
}


std::vector<Socket *> Epoll::get_conn_sock(){
	std::vector<Socket *> result;

	if (this->idx >= this->nfds)
		get_new_events();

	for (; idx < nfds; idx++){
		Socket *sock = (Socket *)events[idx].data.ptr;
		if (find(listen_socks.begin(), listen_socks.end(), sock->fd) != listen_socks.end()){
			int conn_sock = accept(sock->fd, NULL, NULL); 
			if (conn_sock == -1) throw SystemFailure("Accept has failed");
			Utils::setnonblocking(conn_sock);
			struct epoll_event ev;
			ev.events = EPOLLIN | EPOLLET;
			ev.data.ptr = new Socket(conn_sock, sock->port);
			if (epoll_ctl(epollfd, EPOLL_CTL_ADD, conn_sock, &ev) == -1)
				throw SystemFailure("Epoll CTL failed to add listen socket");
		}
		else
			result.push_back(sock);
	}
	return (result);
}

int Epoll::get_epollfd() const{
	return (this->epollfd);
}
// void Epoll::add_socket(int fd, std::string port){
// 	struct epoll_event ev;
// 	ev.events = EPOLLIN | EPOLLET;
// 	ev.data.ptr = new Socket(conn_sock, sock->port);
// 	if (epoll_ctl(epollfd, EPOLL_CTL_ADD, conn_sock, &ev) == -1)
// 		throw SystemFailure("Epoll CTL failed to add listen socket");
// }

// IF IN THE FUTURE RETURNING 1 Socket is prefered

// Socket *Epoll::get_conn_sock2(){
// 	while (true){
// 		if (this->idx >= this->nfds)
// 			get_new_events();
// 		std::vector<int>::iterator it;
// 		it = find(listen_socks.begin(), listen_socks.end(), ((Socket *)events[idx].data.ptr)->fd);
// 		if (it == listen_socks.end()) break;
//
// 		// if client ip and port need fill in the NULL
// 		int conn_sock = accept(*it, NULL, NULL); 
// 		if (conn_sock == -1) throw SystemFailure("Accept has failed");
// 		setnonblocking(conn_sock);
// 		struct epoll_event ev;
// 		ev.events = EPOLLIN | EPOLLET;
// 		ev.data.ptr = new Socket(conn_sock, ((Socket *)this->events[idx].data.ptr)->port);
// 		if (epoll_ctl(epollfd, EPOLL_CTL_ADD, conn_sock, &ev) == -1)
// 			throw SystemFailure("Epoll CTL failed to add listen socket");
// 		idx++;
// 	}
// 	return ((Socket *)this->events[idx++].data.ptr);
// }

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
