#include "../../includes/SocketRegistry.hpp"
#include "WebServer.hpp"
#include <vector>

SocketRegistry::SocketRegistry(){}

SocketRegistry::SocketRegistry(WebServer& prophetServer){
	std::vector<std::string> port_list = prophetServer.getPorts();
	for (size_t i = 0; i < port_list.size(); i++){
		std::string port = port_list.at(i);
		_clientTimeoutMap[port] = prophetServer.getClientTimeout(port);
	}
}

SocketRegistry::~SocketRegistry(){
	_registryIterator it = _registry.begin();
	for (; it != _registry.end(); it++){
		close((*it)->fd);
		delete (*it);
	}
	_registry.clear();
}

Socket*	SocketRegistry::makeSocket(int fd, std::string port, WebServer& server) {
	Socket* temp = new Socket( fd, port, server );
	_registry.insert( temp );
	return (temp);
}

Socket*	SocketRegistry::makeSocket(int fd,  int clientFd, WebServer& server) {
	Socket* temp = new Socket( fd, clientFd, server );
	_registry.insert( temp );
	return (temp);
}

void		SocketRegistry::removeSocket(Socket& toBeRemoved){
	toBeRemoved.is_alive = false;
	close(toBeRemoved.fd);
}

bool		SocketRegistry::searchSocket(const Socket* other){
	_registryIterator it = std::find(_registry.begin(), _registry.end(), other);
	if (it == _registry.end())
		return false;
	return true;
}

void		SocketRegistry::timeoutSocket(Socket& other){
	long serverTimeoutValue = _clientTimeoutMap[other.port];
	if (time(NULL) - other.last_active >= serverTimeoutValue){
		close(other.fd);
		other.is_alive = false;
	}
}

void		SocketRegistry::resetSocketTimer(Socket& other){
	_registryIterator it = std::find(_registry.begin(), _registry.end(), &other);
	if (it == _registry.end())
		return ;
	other.last_active = time(NULL);
	_registry.erase(it);
	_registry.insert(&other);
}

void		SocketRegistry::cleanRegistry(){
	_registryIterator it = _registry.begin();
	for(;it != _registry.end();){
		timeoutSocket(**it);
		if ((*it)->is_alive)
			return ;
		Socket* toBeRemoved = *it;
		_registry.erase(it++);
		delete(toBeRemoved);
	}
}

SocketRegistry::ObjectNotFound::ObjectNotFound(std::string msg):msg(msg){}
const char *SocketRegistry::ObjectNotFound::what() const throw(){ return (this->msg.c_str()); }
SocketRegistry::ObjectNotFound::~ObjectNotFound() throw(){}
