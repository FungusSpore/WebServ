#include "../../includes/SocketRegistry.hpp"

SocketRegistry::SocketRegistry(){}

SocketRegistry::~SocketRegistry(){
	while (!registry.empty()){
		removeSocket(registry.back());
		registry.pop_back();
	}
}

Socket*	SocketRegistry::makeSocket(int fd, std::string port){
	this->registry.push_back( Socket(fd, port) );
	return (&this->registry.back());
}

Socket*	SocketRegistry::makeSocket(int fd,  int clientFd){
	this->registry.push_back( Socket(fd, clientFd) );
	return (&this->registry.back());
}

void		SocketRegistry::removeSocket(const Socket& toBeRemoved){
	std::list<Socket>::iterator it = std::find(registry.begin(), registry.end(), toBeRemoved);
	if (it == registry.end())
		throw ObjectNotFound("No such socket");
	// delete *it;
	close(it->fd);
	registry.erase(it);
}

bool		SocketRegistry::searchSocket(const Socket& other){
	std::list<Socket>::iterator it = std::find(registry.begin(), registry.end(), other);
	if (it == registry.end())
		return false;
	return true;
}

SocketRegistry::ObjectNotFound::ObjectNotFound(std::string msg):msg(msg){}
const char *SocketRegistry::ObjectNotFound::what() const throw(){ return (this->msg.c_str()); }
SocketRegistry::ObjectNotFound::~ObjectNotFound() throw(){}
