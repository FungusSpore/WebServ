#include "WebServer.hpp"

// --------------- WEBSERVER ----------------
void WebServer::checkTopLevelBlock(const Node& topLevelNode) {
	size_t childrenSize = topLevelNode.getChildren().size();
	if (childrenSize == 0) {
		std::string msg = "config file is empty";
		throw (Parser::ParseErrorException(msg));
	}
	if (childrenSize > 1 || topLevelNode.getChildren()[0].getName() != std::string("http")) {
		std::string msg = "config file only accept one http block";
		throw (Parser::ParseErrorException(msg));
	}
}

void WebServer::checkHttpBlock(const Node& httpNode) {
	if (httpNode.getArguments().size() != 0) {
		const Node& arg = httpNode.getArguments()[0];
		std::string msg = "unexpected " + arg.getName() + " at line: " +
			intToString(arg.getLine()) + " col: " + intToString(arg.getColumn());
		throw (Parser::ParseErrorException(msg));
	}
	if (httpNode.getChildren().size() < 1) {
		std::string msg = "http block at line: " + intToString(httpNode.getLine()) +
			" col: " + intToString(httpNode.getColumn()) + " is empty";
		throw (Parser::ParseErrorException(msg));
	}
}

WebServer::WebServer(const char* filename) {
	Parser parser(filename);
	Node root = parser.parseRoot();
	
	checkTopLevelBlock(root);

	const Node& http = root.getChildren()[0];
	checkHttpBlock(http);

	// init Server
	const std::vector<Node>& servers = http.getChildren();
	std::vector<Node>::const_iterator it_servers = servers.begin();
	for ( ; it_servers != servers.end(); ++it_servers) {
		Server serv(*it_servers);
		ServerKey serv_key(serv.getIp(), serv.getPort(), serv.getServerName());
		std::vector<ServerKey>::const_iterator it = _serverKeys.begin();
		for ( ; it != _serverKeys.end(); ++it) {
			if (serv_key._port == it->_port && serv_key._serverName == it->_serverName) {
				std::string msg = "duplicate port " + serv_key._port
					+ " and server_name " + serv_key._serverName;
				throw (Parser::ParseErrorException(msg));
			}
		}
		if (_ports.empty())
			_ports.push_back(serv_key._port);
		else {
			std::vector<std::string>::const_iterator itPort =
				std::find(_ports.begin(), _ports.end(), serv_key._port);
			if (itPort == _ports.end())
				_ports.push_back(serv_key._port);
		}
		_serverKeys.push_back(serv_key);
		_serverMap.insert(std::pair<ServerKey, Server>(serv_key, serv));
	}
}

WebServer::WebServer(const WebServer& other) :
	_ports(other._ports),
	_serverKeys(other._serverKeys),
	_serverMap(other._serverMap),
	_cookieVector(other._cookieVector) {}

WebServer& WebServer::operator=(const WebServer& other) {
	if (this != &other) {
		this->_ports = other._ports;
		this->_serverKeys = other._serverKeys;
		this->_serverMap = other._serverMap;
		this->_cookieVector = other._cookieVector;
	}
	return (*this);
}

WebServer::~WebServer() {}

static bool matchLocalhost(const std::string& src, const std::string& query) {
	if (src.substr(0, 3) == "127" && (query.substr(0, 3) == "127" || query == "localhost")) {
		return (true);
	}
	return (false);
}

static bool matchExactIp(const std::string& src, const std::string& query) {
	if (src == query) {
		return (true);
	}
	return (false);
}

const Server* WebServer::matchServer(const ServerKey& key) const {
	std::map<ServerKey, Server>::const_iterator it = _serverMap.begin();
	for ( ; it != _serverMap.end(); ++it) {
		if (matchLocalhost(it->first._ip, key._ip)
			|| matchExactIp(it->first._ip, key._ip)) {
			// std::cout << "Matched localhost or exact IP" << std::endl;
			if (key._port == it->first._port) {
				// std::cout << "Matched port" << std::endl;
				return(&it->second);
			}
		}
	}
	it = _serverMap.begin();
	for ( ; it != _serverMap.end(); ++it) {
		if (it->first._ip == "0.0.0.0") {
			// std::cout << "Matched wildcard" << std::endl;
			if (key._port == it->first._port) {
				// std::cout << "Matched port" << std::endl;
				return(&it->second);
			}
		}
	}
	return (NULL);
}

const std::vector<std::string>& WebServer::getPorts() const {
	return (_ports);
}

long WebServer::getClientTimeout(const std::string& port) const {
	std::map<ServerKey, Server>::const_iterator it = _serverMap.begin();

	for (; it != _serverMap.end(); it++)
		if (it->first._port == port)
			return it->second.getClientTimeout();
	return (-1);
}

Cookie* WebServer::matchCookieValue(const std::string& value) {
	std::vector<Cookie>::iterator it = _cookieVector.begin();

	for ( ; it != _cookieVector.end(); ++it) {
		if (value == it->getValue())
			return (&(*it));
	}
	return (NULL);
}

Cookie* WebServer::addCookie(const std::string& content) {
	Cookie newCookie(_cookieVector, content);
	_cookieVector.push_back(newCookie);
	std::string value = newCookie.getValue();

	std::cout << "Value: " << value << std::endl;
	return (this->matchCookieValue(value));
}

bool WebServer::deleteCookie(const std::string& value) {
	std::vector<Cookie>::iterator it = _cookieVector.begin();
	for ( ; it != _cookieVector.end(); ++it) {
		if (value == it->getValue()) {
			_cookieVector.erase(it);
			return true;
		}
	}
	return false;
}
