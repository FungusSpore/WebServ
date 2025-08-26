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

#define CGI_TIMEOUT 60

struct SocketPtrComparator {
	bool operator()(const Socket* lhs, const Socket* rhs){
		return (*lhs < *rhs);
	}
};

typedef std::multiset<Socket*, SocketPtrComparator>::iterator _registryIterator;
typedef std::multiset<Socket*, SocketPtrComparator>::const_iterator _registryConstIterator;

class SocketRegistry{
private:
	std::multiset<Socket*, SocketPtrComparator> _registry;
	std::map<std::string, long> _clientTimeoutMap;
public:
	SocketRegistry();
	SocketRegistry(WebServer& prophetServer);
	~SocketRegistry();

	Socket*	makeSocket(int fd, std::string port, WebServer& server);
	// Socket*	makeSocket(int fd, int forward_fd);
	Socket*	makeSocket(int fd, Socket* toSend, WebServer& server);
	void		removeSocket(Socket& toBeRemoved);
	bool		searchSocket(const Socket* other);
	void		timeoutSocket(Socket& other);
	void		resetSocketTimer(Socket& other);
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
