#include "../../includes/IO.hpp"
#include <cstring>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>


void	IO::try_write(Epoll& epoll, struct epoll_event& event){
	struct epoll_event ev;
	Socket&		sock = *static_cast<Socket *>(event.data.ptr);
	uint32_t	events = event.events;

	while (!sock.write_buffer.empty()){
		ssize_t size = send(sock.fd, &sock.write_buffer[0], sock.write_buffer.size(), MSG_NOSIGNAL);
		std::cout << sock.fd << " WROTE :" << size << std::endl;
		if (size == -1){
			if (!(events & EPOLLOUT)){
				events |= EPOLLOUT;
				ev.events = events;
				ev.data.ptr = &sock;
				if (epoll_ctl(epoll.get_epollfd(), EPOLL_CTL_MOD, sock.fd, &ev) == -1){
					epoll.closeSocket(sock);
					throw SystemFailure("Epoll CTL failed to mod socket");
				}
			}
			break ;
		}
		if (size == 0){
			epoll.closeSocket(sock);
			return ;
		}
		if (static_cast<size_t>(size) < sock.write_buffer.size()) {
				std::memmove(sock.write_buffer.data(), \
								 sock.write_buffer.data() + size, sock.write_buffer.size() - size);
				sock.write_buffer.resize(sock.write_buffer.size() - size);
		}
		else {
			std::cout << "Write buffer done" << std::endl;
			sock.write_buffer.clear();
			break ;
		}
	}

	if (sock.write_buffer.empty()) {
		if (events & EPOLLOUT){
			events &= ~EPOLLOUT;
			ev.events = events;
			ev.data.ptr = &sock;
			if (epoll_ctl(epoll.get_epollfd(), EPOLL_CTL_MOD, sock.fd, &ev) == -1){
				epoll.closeSocket(sock);
				throw SystemFailure("Epoll CTL failed to mod socket");
			}
		}
		if (sock.toSend != NULL){
			std::cout << "closing write end" << std::endl;
			std::cerr << "Server: shutting down CGI socket for fd=" << sock.fd << std::endl;
			shutdown(sock.fd, SHUT_WR);
			epoll.resetSocketTimer(sock);
			return ;
		}
		if (!sock.keepAlive){
			epoll.closeSocket(sock);
			return ;
		}
	}
	epoll.resetSocketTimer(sock);
}

int	IO::try_read(Epoll& epoll, struct epoll_event& event){
	char	buffer[READ_BUFFER];
	ssize_t		size;
	Socket&		sock = *static_cast<Socket *>(event.data.ptr);

	while (true){
		size = recv(sock.fd, buffer, READ_BUFFER, 0);
		std::cout << sock.fd << " READ :" << size << std::endl;
		epoll.resetSocketTimer(sock);
		if (size > 0)
			sock.read_buffer.insert(sock.read_buffer.end(), buffer, buffer+size);
		else if (size == -1) // assume EAGAIN
			return 0;
		else if (size == 0){
			epoll.closeSocket(sock);
			return -1;
		}
	}
}
