#include "Server.hpp"

// --------------- CONSTRUCTORS/DESTRUCTORS ---------------
Server::Server(const Node& serverNode) :
	_allowedMethods(getDefaultHttpMethods()),
	_clientMaxBodySize(DEFAULT_CLIENT_MAX_BODY_SIZE),
	_clientTimeout(DEFAULT_CLIENT_TIMEOUT),
	_isListenSet(false),
	_isServerNameSet(false),
	_isRootPathSet(false),
	_isAllowedMethodsSet(false),
	_isClientMaxBodySizeSet(false),
	_isClientTimeoutSet(false)
{
	checkServerBlock(serverNode);
	assignRootPath(*this, serverNode);
	assignDefaultErrorPages(*this);

	const std::vector<Node>& child = serverNode.getChildren();
	std::vector<Node>::const_iterator it = child.begin();
	for ( ; it != child.end(); ++it) {
		switch (it->findDirective(it->getName())) {
			case (DIR_LOCATION)				: break ;
			case (DIR_LISTEN)				: handleListenDir(*this, *it); break ;
			case (DIR_SERVER_NAME)			: handleServerNameDir(*this, *it); break ;
			case (DIR_ROOT)					: break ;
			case (DIR_ALIAS)				: handleUnsupportedDir(*it); break ;
			case (DIR_ERROR_PAGE)			: handleErrorPageDir(*this, *it); break ;
			case (DIR_ALLOWED_METHOD)		: handleAllowedMethodDir(*this, *it); break ;
			case (DIR_INDEX)				: handleIndexDir(*this, *it); break ;
			case (DIR_CGI)					: handleUnsupportedDir(*it); break ;
			case (DIR_AUTOINDEX)			: handleUnsupportedDir(*it); break ;
			case (DIR_PROXY_PASS)			: handleUnsupportedDir(*it); break ;
			case (DIR_REDIRECTION)			: handleUnsupportedDir(*it); break ;
			case (DIR_RETURN)				: handleUnsupportedDir(*it); break ;
			case (DIR_CLIENT_MAX_BODY_SIZE)	: handleClientMaxBodySizeDir(*this, *it); break ;
			case (DIR_CLIENT_TIMEOUT)		: handleClientTimeoutDir(*this, *it); break;
			default							: handleUnknownDir(*it);
		}
	}
	for (it = child.begin(); it != child.end(); ++it) {
		if (it->findDirective(it->getName()) == DIR_LOCATION) {
			handleLocationDir(*this, *it);
		}
	}
}

Server::Server(const Server& other) : 
	_ip(other._ip),
	_port(other._port),
	_serverName(other._serverName),
	_rootPath(other._rootPath),
	_allowedMethods(other._allowedMethods),
	_clientMaxBodySize(other._clientMaxBodySize),
	_clientTimeout(other._clientTimeout),
	_indexFiles(other._indexFiles),
	_errorPages(other._errorPages),
	_locations(other._locations),
	_locationTargets(other._locationTargets),
	_isListenSet(other._isListenSet),
	_isServerNameSet(other._isServerNameSet),
	_isRootPathSet(other._isRootPathSet),
	_isAllowedMethodsSet(other._isAllowedMethodsSet),
	_isClientMaxBodySizeSet(other._isClientMaxBodySizeSet),
	_isClientTimeoutSet(other._isClientTimeoutSet) {}

Server& Server::operator=(const Server& other) {
	if (this != &other) {
		this->_ip = other._ip;
		this->_port = other._port;
		this->_serverName = other._serverName;
		this->_rootPath = other._rootPath;
		this->_allowedMethods = other._allowedMethods;
		this->_clientMaxBodySize = other._clientMaxBodySize;
		this->_clientTimeout = other._clientTimeout;
		this->_indexFiles = other._indexFiles;
		this->_errorPages = other._errorPages;
		this->_locations = other._locations;
		this->_locationTargets = other._locationTargets;
		this->_isListenSet = other._isListenSet;
		this->_isServerNameSet = other._isServerNameSet;
		this->_isRootPathSet = other._isRootPathSet;
		this->_isAllowedMethodsSet = other._isAllowedMethodsSet;
		this->_isClientMaxBodySizeSet = other._isClientMaxBodySizeSet;
		this->_isClientTimeoutSet = other._isClientTimeoutSet;
	}
	return (*this);
}

Server::~Server() {}

// -------------- PRIVATE METHODS ---------------
void Server::handleListenDir(Server& server, const Node& node) {
	DirChecker::checkDuplicateDirective(node, server.getIsListenSet());
	DirChecker::checkListenDirective(node);

	const std::vector<Node>& arg = node.getArguments();
	const std::string& argStr = arg[0].getName();
	std::string ipString;
	std::string portString;
	std::string::size_type pos = argStr.find(":");
	if (pos == std::string::npos) {
		ipString = "0.0.0.0";
		portString = argStr;
	}
	else {
		ipString = argStr.substr(0, pos);
		portString = argStr.substr(pos + 1);
	}
	server.setIp(ipString);
	server.setPort(portString);
	server.setIsListen();
}

void Server::handleServerNameDir(Server& server, const Node& node) {
	DirChecker::checkDuplicateDirective(node, server.getIsServerNameSet());
	DirChecker::checkServerNameDirective(node);

	server.setServerName(node.getArguments()[0].getName());
	server.setIsServerName();
}

// error_page <status code> <URI path>
void Server::handleErrorPageDir(Server& server, const Node& node) {
	DirChecker::checkErrorPageDirective(node);

	const std::vector<Node>& arg = node.getArguments();
	size_t argSize = arg.size();
	const std::string& uriPath = arg[argSize - 2].getName();

	for (size_t i = 0; i < argSize - 2; i++) {
		int statusCode = std::atoi(arg[i].getName().c_str());
		server.addErrorPage(statusCode, uriPath);
	}
}

void Server::handleAllowedMethodDir(Server& server, const Node& node) {
	DirChecker::checkDuplicateDirective(node, server.getIsAllowedMethodsSet());
	DirChecker::checkAllowedMethodDirective(node);

	const std::vector<Node>& arg = node.getArguments();
	std::vector<Node>::const_iterator it = arg.begin();
	std::set<HttpMethod> newAllowedMethods;
	for ( ; it != arg.end(); ++it) {
		if (it->getName() == "GET")
			newAllowedMethods.insert(GET);
		if (it->getName() == "POST")
			newAllowedMethods.insert(POST);
		if (it->getName() == "DELETE")
			newAllowedMethods.insert(DELETE);
		if (it->getName() == "HEAD")
			newAllowedMethods.insert(HEAD);
	}
	server.setAllowedMethods(newAllowedMethods);
	server.setIsAllowedMethods();
}

void Server::handleIndexDir(Server& server, const Node& node) {
	DirChecker::checkIndexDirective(node);
	const std::vector<Node>& arg = node.getArguments();
	std::vector<Node>::const_iterator it = arg.begin();

	std::vector<std::string> newIndexFiles;
	for ( ; it != arg.end() - 1; ++it) {
		newIndexFiles.push_back(it->getName());
	}
	server.setIndexFiles(newIndexFiles);
}

void Server::handleClientMaxBodySizeDir(Server& server, const Node& node) {
	DirChecker::checkDuplicateDirective(node, server.getIsClientMaxBodySizeSet());
	DirChecker::checkClientMaxBodySizeDirective(node);

	const std::vector<Node>& arg = node.getArguments();
	char *end;
	long clientMaxBodySize = std::strtol(arg[0].getName().c_str(), &end, 10);
	server.setClientMaxBodySize(clientMaxBodySize);
	server.setIsClientMaxBodySize();
}

void Server::handleClientTimeoutDir(Server& server, const Node& node) {
	DirChecker::checkDuplicateDirective(node, server.getIsClientTimeoutSet());
	DirChecker::checkClientTimeoutDirective(node);

	const std::vector<Node>& arg = node.getArguments();
	char *end;
	long clientTimeout = std::strtol(arg[0].getName().c_str(), &end, 10);
	server.setClientTimeout(clientTimeout);
	server.setIsClientTimeout();
}

void Server::handleUnsupportedDir(const Node& node) {
	std::string msg = node.getName() + " at line: " + intToString(node.getLine())
		+ " col: " + intToString(node.getColumn()) + " not supported in server block";
	throw (Parser::ParseErrorException(msg));
}

void Server::handleUnknownDir(const Node& node) {
	std::string msg = "unexpected directive " + node.getName() + " at line: " +
		intToString(node.getLine()) + " col: " + intToString(node.getColumn())
		+ " in server block";
	throw (Parser::ParseErrorException(msg));
}

void Server::handleLocationDir(Server& server, const Node& node) {
	DirChecker::checkLocationDirective(node, server.getLocationTargets());
	
	Location loc(server, node);
	std::string locationTarget = loc.getUriPath();
	server.addLocationTarget(locationTarget);
	server.addLocation(loc);
}

// -------------- PRIVATE UTILS ---------------
void Server::checkServerBlock(const Node& serverNode) {
	if (serverNode.getArguments().size() != 0) {
		const Node& arg = serverNode.getArguments()[0];
		std::string msg = "unexpected " + arg.getName() + " at line: " +
			intToString(arg.getLine()) + " col: " + intToString(arg.getColumn());
		throw (Parser::ParseErrorException(msg));
	}
	if (serverNode.getChildren().size() < 1) {
		std::string msg = "http block at line: " + intToString(serverNode.getLine()) +
			" col: " + intToString(serverNode.getColumn()) + " is empty";
		throw (Parser::ParseErrorException(msg));
	}
}

void Server::assignRootPath(Server& server, const Node& serverNode) {
	const std::vector<Node>& child = serverNode.getChildren();
	std::vector<Node>::const_iterator it_child = child.begin();

	for ( ; it_child != child.end(); ++it_child) {
		if (it_child->getName() == "root") {
			DirChecker::checkRootDirective(*it_child);
			if (!server.getIsRootPathSet()) {
				server.setRootPath(it_child->getArguments()[0].getName());
				server.setIsRootPath();
			}
			else {
				std::string msg = "extra root specified at line: " +
					intToString(it_child->getLine()) + 
					" col: " + intToString(it_child->getColumn());
				throw (Parser::ParseErrorException(msg));
			}
		}
	}
	if (!server.getIsRootPathSet()) {
		std::string msg = "root not specified for server block at line: " +
			intToString(serverNode.getLine()) + " col: " + intToString(serverNode.getColumn());
		throw (Parser::ParseErrorException(msg));
	}
}

// default error pages path = actual full filepath
void Server::assignDefaultErrorPages(Server& server) {
	int codes[] = {
		400, 401, 402, 403, 404, 405, 406, 407, 408, 409,
		410, 411, 412, 413, 414, 415, 416, 417, 421, 426,
		428, 429, 431, 500, 501, 502, 503, 504, 505
	};
	size_t size = sizeof(codes) / sizeof(int);
	std::string path = server.getRootPath() + "/www/error_pages/default_pages/";

	for (size_t i = 0; i < size; i++) {
		std::ostringstream oss;
		oss << path << codes[i] << "_default.html";
		server.addErrorPage(codes[i], oss.str());
	}
}

std::set<HttpMethod> Server::getDefaultHttpMethods() {
	std::set<HttpMethod> allowedMethods;
	allowedMethods.insert(GET);
	allowedMethods.insert(POST);
	allowedMethods.insert(DELETE);
	allowedMethods.insert(HEAD);

	return (allowedMethods);
}

// ---------- GETTERS ----------
const std::string& Server::getIp() const {
	return (_ip);
}

const std::string& Server::getPort() const {
	return (_port);
}

const std::string& Server::getServerName() const {
	return (_serverName);
}

const std::string& Server::getRootPath() const {
	return (_rootPath);
}

const std::set<HttpMethod>& Server::getAllowedMethods() const {
	return (_allowedMethods);
}

long Server::getClientMaxBodySize() const {
	return (_clientMaxBodySize);
}

long Server::getClientTimeout() const {
	return (_clientTimeout);
}

const std::vector<std::string>& Server::getIndexFiles() const {
	return (_indexFiles);
}

const std::map<int, std::string>& Server::getErrorPages() const {
	return (_errorPages);
}

const std::vector<Location>& Server::getLocations() const {
	return (_locations);
}

const std::set<std::string>& Server::getLocationTargets() const {
	return (_locationTargets);
}

bool Server::getIsListenSet() const {
	return (_isListenSet);
}

bool Server::getIsServerNameSet() const {
	return (_isServerNameSet);
}

bool Server::getIsRootPathSet() const {
	return (_isRootPathSet);
}

bool Server::getIsAllowedMethodsSet() const {
	return (_isAllowedMethodsSet);
}

bool Server::getIsClientMaxBodySizeSet() const {
	return (_isClientMaxBodySizeSet);
}

bool Server::getIsClientTimeoutSet() const {
	return (_isClientTimeoutSet);
}

// ---------- SETTERS ----------
void Server::setIp(const std::string& ip) {
	_ip = ip;
}

void Server::setPort(const std::string& port) {
	_port = port;
}

void Server::setServerName(const std::string& serverName) {
	_serverName = serverName;
}

void Server::setRootPath(const std::string& path) {
	_rootPath = path;
}

void Server::setAllowedMethods(const std::set<HttpMethod>& newAllowedMethods) {
	_allowedMethods = newAllowedMethods;
}

void Server::setClientMaxBodySize(long clientMaxBodySize) {
	_clientMaxBodySize = clientMaxBodySize;
}

void Server::setClientTimeout(long clientTimeout) {
	_clientTimeout = clientTimeout;
}

void Server::setIndexFiles(const std::vector<std::string>& newIndexFiles) {
	_indexFiles = newIndexFiles;
}

void Server::setErrorPages(const std::map<int, std::string>& newErrorPages) {
	_errorPages = newErrorPages;
}

void Server::setLocations(const std::vector<Location>& newLocations) {
	_locations = newLocations;
}

void Server::setIsListen() {
	if (!_isListenSet)
		_isListenSet = true;
}

void Server::setIsServerName() {
	if (!_isServerNameSet)
		_isServerNameSet = true;
}

void Server::setIsRootPath() {
	if (!_isRootPathSet)
		_isRootPathSet = true;
}

void Server::setIsAllowedMethods() {
	if (!_isAllowedMethodsSet)
		_isAllowedMethodsSet = true;
}

void Server::setIsClientMaxBodySize() {
	if (!_isClientMaxBodySizeSet)
		_isClientMaxBodySizeSet = true;
}

void Server::setIsClientTimeout() {
	if (!_isClientTimeoutSet)
		_isClientTimeoutSet = true;
}

// --------------- PUBLIC UTILS ---------------
const Location* Server::matchLocation(const std::string& target) const {
	const Location* match = NULL;
	size_t matchLength = 0;

	for (size_t i = 0; i < _locations.size(); i++) {
		const std::string& path = _locations[i].getUriPath();

		if (target.compare(0, path.size(), path) == 0
			&& (target.size() == path.size() || target[path.size()] == '/')) {
			// && (target.size() == path.size() || (target.size() > path.size() && target[path.size()] == '/'))) {
			if (path.size() > matchLength) {
				matchLength = path.size();
				match = &_locations[i];
			}
		}
	}
	return (match);
}

const std::string* Server::getErrorPagePath(int statusCode) const {
	std::map<int, std::string>::const_iterator it = _errorPages.find(statusCode);
	if (it != _errorPages.end())
		return (&it->second);
	return (NULL);
}

void Server::addErrorPage(int code, const std::string& errorPagePath) {
	_errorPages[code] = errorPagePath;
}

void Server::addLocation(const Location& location) {
	_locations.push_back(location);
}

void Server::addLocationTarget(const std::string& locationTarget) {
	_locationTargets.insert(locationTarget);
}

// --------------- SERVERKEY ---------------
ServerKey::ServerKey(const std::string& ip, const std::string& port, const std::string& serverName) :
	_ip(ip), _port(port), _serverName(serverName) {}

bool operator<(const ServerKey& lhs, const ServerKey& rhs) {
	if (lhs._port != rhs._port)
		return (lhs._port < rhs._port);
	if (lhs._serverName != rhs._serverName) // should be !=
		return (lhs._serverName < rhs._serverName);
	return (lhs._ip < rhs._ip);
}
