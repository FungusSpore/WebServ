#ifndef SOCKETREGISTRY_HPP
# define SOCKETREGISTRY_HPP

#include "Socket.hpp"
#include <exception>
#include <list>
#include <algorithm>
#include <queue>
#include <map>
#include <unistd.h>
#include "WebServer.hpp"

struct SocketPtrComparator {
	bool operator()(const Socket* lhs, const Socket* rhs){
		return (*lhs < *rhs);
	}
};

typedef std::set<Socket*>::iterator _registryIterator;
typedef std::set<Socket*>::const_iterator _registryConstIterator;

class SocketRegistry{
private:
	std::set<Socket*, SocketPtrComparator> _registry;
	std::map<std::string, long> _clientTimeoutMap;
public:
	SocketRegistry();
	SocketRegistry(WebServer& prophetServer);
	~SocketRegistry();

	Socket*	makeSocket(int fd, std::string port, WebServer& server);
	// Socket*	makeSocket(int fd, int forward_fd);
	Socket*	makeSocket(int fd, int clientFd, WebServer& server);
	void		removeSocket(Socket& toBeRemoved);
	bool		searchSocket(const Socket* other);
	void		timeoutSocket(Socket& other);
	void		cleanRegistry();


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
