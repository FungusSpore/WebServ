#ifndef WEBSERVER_HPP
#define WEBSERVER_HPP

#include <vector>
#include <map>
#include <set>
#include <string>
#include <iostream>
#include <algorithm>
#include "Server.hpp"
#include "Cookie.hpp"

class WebServer {
private:
	std::vector<std::string> _ports;
	std::vector<ServerKey> _serverKeys;
	std::map<ServerKey, Server> _serverMap;
	std::set<Cookie> _cookieSet;

	static void checkTopLevelBlock(const Node& topLevelNode);
	static void checkHttpBlock(const Node& httpNode);

public:
	WebServer(const char* filename);
	WebServer(const WebServer& other);
	WebServer& operator=(const WebServer& other);
	~WebServer();

	const Server* matchServer(const ServerKey& key) const;
	const std::vector<std::string>& getPorts() const;
	long getClientTimeout(const std::string& port) const;

	const Cookie* matchCookieValue(const std::string& value);
	void addCookie(const std::string& content);
};

#endif // !WEBSERVER_HPP
