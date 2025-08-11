#ifndef SOCKETREGISTRY_HPP
# define SOCKETREGISTRY_HPP

#include "Socket.hpp"
#include <exception>
#include <list>
#include <algorithm>
#include <unistd.h>

class SocketRegistry{
private:
	std::list<Socket *> registry;
public:
	SocketRegistry();
	~SocketRegistry();

	Socket*	makeSocket(int fd, std::string port);
	// Socket*	makeSocket(int fd, int forward_fd);
	Socket*	makeSocket(int fd, Socket *toSend);
	void		removeSocket(const Socket& toBeRemoved);
	bool		searchSocket(const Socket& other);


	class ObjectNotFound : public std::exception{
	private:
		std::string msg;
	public:
		ObjectNotFound(std::string msg);
		const char* what() const throw();
		~ObjectNotFound() throw();
	};
};

#endif
