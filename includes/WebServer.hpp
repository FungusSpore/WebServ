#ifndef WEBSERVER_HPP
#define WEBSERVER_HPP

#include <vector>
#include <map>
#include <string>
#include <iostream>
#include <algorithm>
#include "Server.hpp"

class WebServer {
private:
	std::vector<std::string> _ports;
	std::map<ServerKey, Server> _serverMap;

	static void checkTopLevelBlock(const Node& topLevelNode);
	static void checkHttpBlock(const Node& httpNode);

public:
	WebServer(const char* filename);
	WebServer(const WebServer& other);
	WebServer& operator=(const WebServer& other);
	~WebServer();

	const Server* matchServer(const ServerKey& key) const;
	const std::vector<std::string>& getPorts() const;
};

#endif // !WEBSERVER_HPP
