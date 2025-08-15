#include "../../includes/IO.hpp"

void	IO::try_write(Epoll& epoll, struct epoll_event& event){
	ssize_t		size;
	struct epoll_event ev;
	Socket&		sock = *static_cast<Socket *>(event.data.ptr);
	uint32_t	events = event.events;

	if (sock.clientFd == -1)
		size = send(sock.fd, sock.write_buffer.c_str(), sock.write_buffer.size(), MSG_NOSIGNAL);
	else
		size = send(sock.clientFd, sock.write_buffer.c_str(), sock.write_buffer.size(), MSG_NOSIGNAL);

	if (size > 0)
		sock.write_buffer.erase(sock.write_buffer.begin(), sock.write_buffer.begin() + size);

	if (sock.write_buffer.empty() && events & EPOLLOUT){
		events &= ~EPOLLOUT;
		ev.events = events;
		ev.data.ptr = &sock;
		if (epoll_ctl(epoll.get_epollfd(), EPOLL_CTL_MOD, sock.fd, &ev) == -1){
			epoll.closeSocket(sock);
			throw SystemFailure("Epoll CTL failed to mod socket");
		}
	}

	if (size == -1 && !(events & EPOLLOUT)){
		events |= EPOLLOUT;
		ev.events = events;
		ev.data.ptr = &sock;
		if (epoll_ctl(epoll.get_epollfd(), EPOLL_CTL_MOD, sock.fd, &ev) == -1){
			epoll.closeSocket(sock);
			throw SystemFailure("Epoll CTL failed to mod socket");
		}
	}
}

int	IO::try_read(Epoll& epoll, struct epoll_event& event){
	char	buffer[READ_BUFFER];
	ssize_t		size;
	Socket&		sock = *static_cast<Socket *>(event.data.ptr);

	while (true){
		size = recv(sock.fd, buffer, READ_BUFFER, 0);
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
