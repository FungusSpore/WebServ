#include "Location.hpp"
#include "Node.hpp"
#include "Server.hpp"

// --------------- CONSTRUCTORS/DESTRUCTORS ---------------
Location::Location(const Server& server, const Node& locationNode) : 
	_uriPath(locationNode.getArguments()[0].getName()),
	_allowedMethods(server.getAllowedMethods()),
	_pathMode(DEFAULT_PATHMODE),
	_locationMode(DEFAULT_LOCATIONMODE),
	_isAutoindexOn(false),
	_isCgiOn(false),
	_clientMaxBodySize(server.getClientMaxBodySize()),
	_clientTimeout(server.getClientTimeout()),
	_indexFiles(server.getIndexFiles()),
	_errorPages(server.getErrorPages()),
	_redirectionStatusCode(-1),
	_isAllowedMethodsSet(false),
	_isClientMaxBodySizeSet(false),
	_isClientTimeoutSet(false),
	_isRootSet(false),
	_isAliasSet(false),
	_isAutoindexSet(false),
	_isProxyPassSet(false),
	_isRedirectionSet(false),
	_isCgiSet(false)
{
	const std::vector<Node>& child = locationNode.getChildren();
	std::vector<Node>::const_iterator it = child.begin();
	for ( ; it != child.end(); ++it) {
		switch (it->findDirective(it->getName())) {
			case (DIR_LOCATION)				: handleUnsupportedDir(*it); break ;
			case (DIR_LISTEN)				: handleUnsupportedDir(*it); break ;
			case (DIR_SERVER_NAME)			: handleUnsupportedDir(*it); break ;
			case (DIR_ROOT)					: handleRootDir(*this, *it); break ;
			case (DIR_ALIAS)				: handleAliasDir(*this, *it); break ;
			case (DIR_ERROR_PAGE)			: handleErrorPageDir(*this, *it); break ;
			case (DIR_ALLOWED_METHOD)		: handleAllowedMethodDir(*this, *it); break ;
			case (DIR_INDEX)				: handleIndexDir(*this, *it); break ;
			case (DIR_CGI)					: handleCgiDir(*this, *it); break ;
			case (DIR_AUTOINDEX)			: handleAutoindexDir(*this, *it); break ;
			case (DIR_PROXY_PASS)			: handleProxyPassDir(*this, *it); break ;
			case (DIR_REDIRECTION)			: handleRedirectionDir(*this, *it); break ;
			case (DIR_CLIENT_MAX_BODY_SIZE)	: handleClientMaxBodySizeDir(*this, *it); break ;
			case (DIR_CLIENT_TIMEOUT)		: handleClientTimeoutDir(*this, *it); break;
			default							: handleUnknownDir(*it);
		}
	}
	if (_locationMode == DEFAULT_LOCATIONMODE)
		_locationMode = STATIC;
}

Location::Location(const Location& other) : 
	_uriPath(other._uriPath),
	_allowedMethods(other._allowedMethods),
	_pathMode(other._pathMode),
	_path(other._path),
	_locationMode(other._locationMode),
	_isAutoindexOn(other._isAutoindexOn),
	_isCgiOn(other._isCgiOn),
	_address(other._address),
	_clientMaxBodySize(other._clientMaxBodySize),
	_clientTimeout(other._clientTimeout),
	_indexFiles(other._indexFiles),
	_errorPages(other._errorPages),
	_redirectionStatusCode(other._redirectionStatusCode),
	_isAllowedMethodsSet(other._isAllowedMethodsSet),
	_isClientMaxBodySizeSet(other._isClientMaxBodySizeSet),
	_isClientTimeoutSet(other._isClientTimeoutSet),
	_isRootSet(other._isRootSet),
	_isAliasSet(other._isAliasSet),
	_isAutoindexSet(other._isAutoindexSet),
	_isProxyPassSet(other._isProxyPassSet),
	_isRedirectionSet(other._isRedirectionSet),
	_isCgiSet(other._isCgiSet) {}

Location& Location::operator=(const Location& other) {
	if (this != & other) {
		this->_uriPath = other._uriPath;
		this->_allowedMethods = other._allowedMethods;
		this->_pathMode = other._pathMode;
		this->_path = other._path;
		this->_locationMode = other._locationMode;
		this->_isAutoindexOn = other._isAutoindexOn;
		this->_isCgiOn = other._isCgiOn;
		this->_address = other._address;
		this->_clientMaxBodySize = other._clientMaxBodySize;
		this->_clientTimeout = other._clientTimeout;
		this->_indexFiles = other._indexFiles;
		this->_errorPages = other._errorPages;
		this->_redirectionStatusCode = other._redirectionStatusCode;
		this->_isAllowedMethodsSet = other._isAllowedMethodsSet;
		this->_isClientMaxBodySizeSet = other._isClientMaxBodySizeSet;
		this->_isClientTimeoutSet = other._isClientTimeoutSet;
		this->_isRootSet = other._isRootSet;
		this->_isAliasSet = other._isAliasSet;
		this->_isAutoindexSet = other._isAutoindexSet;
		this->_isProxyPassSet = other._isProxyPassSet;
		this->_isRedirectionSet = other._isRedirectionSet;
		this->_isCgiSet = other._isCgiSet;
	}
	return (*this);
}

Location::~Location() {}

// -------------- PRIVATE METHODS ---------------
void Location::handleRootDir(Location& location, const Node& node) {
	DirChecker::checkDuplicateDirective(node, location.getIsRootSet());
	DirChecker::checkPathModeDirective(node, location.getPathMode());
	DirChecker::checkRootDirective(node);

	const std::vector<Node>& arg = node.getArguments();
	const std::string& root = arg[0].getName();
	location.setPath(root);
	location.setPathMode(ROOT);
	location.setIsRoot();
}

void Location::handleAliasDir(Location& location, const Node& node) {
	DirChecker::checkDuplicateDirective(node, location.getIsAliasSet());
	DirChecker::checkPathModeDirective(node, location.getPathMode());
	DirChecker::checkAliasDirective(node);

	const std::vector<Node>& arg = node.getArguments();
	const std::string& alias = arg[0].getName();
	location.setPath(alias);
	location.setPathMode(ALIAS);
	location.setIsAlias();
}

void Location::handleErrorPageDir(Location& location, const Node& node) {
	DirChecker::checkErrorPageDirective(node);

	const std::vector<Node>& arg = node.getArguments();
	size_t argSize = arg.size();
	const std::string& uriPath = arg[argSize - 2].getName();

	for (size_t i = 0; i < argSize - 2; i++) {
		int statusCode = std::atoi(arg[i].getName().c_str());
		location.addErrorPage(statusCode, uriPath);
	}
}

void Location::handleAllowedMethodDir(Location& location, const Node& node) {
	DirChecker::checkDuplicateDirective(node, location.getIsAllowedMethodsSet());
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
	location.setAllowedMethods(newAllowedMethods);
	location.setIsAllowedMethods();
}

void Location::handleIndexDir(Location& location, const Node& node) {
	DirChecker::checkLocationModeDirective(node, location.getLocationMode());
	DirChecker::checkIndexDirective(node);
	const std::vector<Node>& arg = node.getArguments();
	std::vector<Node>::const_iterator it = arg.begin();

	std::vector<std::string> newIndexFiles;
	for ( ; it != arg.end() - 1; ++it) {
		newIndexFiles.push_back(it->getName());
	}
	location.setIndexFiles(newIndexFiles);
	location.setLocationMode(STATIC);
}

void Location::handleCgiDir(Location& location, const Node& node) {
	DirChecker::checkDuplicateDirective(node, location.getIsCgiSet());
	DirChecker::checkLocationModeDirective(node, location.getLocationMode());
	DirChecker::checkCgiDirective(node);

	const std::string& state = node.getArguments()[0].getName();
	if (state == "on") {
		location.setIsCgiOn();
	}
	location.setIsCgi();
	location.setLocationMode(CGI);
}

void Location::handleAutoindexDir(Location& location, const Node& node) {
	DirChecker::checkDuplicateDirective(node, location.getIsAutoindexSet());
	DirChecker::checkLocationModeDirective(node, location.getLocationMode());
	DirChecker::checkAutoindexDirective(node);

	const std::string& state = node.getArguments()[0].getName();
	if (state == "on") {
		location.setIsAutoindexOn();
	}
	location.setIsAutoindex();
	location.setLocationMode(AUTOINDEX);
}

void Location::handleProxyPassDir(Location& location, const Node& node) {
	DirChecker::checkDuplicateDirective(node, location.getIsProxyPassSet());
	DirChecker::checkLocationModeDirective(node, location.getLocationMode());
	DirChecker::checkProxyPassDirective(node);

	const std::string& address = node.getArguments()[0].getName();
	location.setAddress(address);
	location.setIsProxyPass();
	location.setLocationMode(PROXYPASS);
}

void Location::handleRedirectionDir(Location& location, const Node& node) {
	DirChecker::checkDuplicateDirective(node, location.getIsRedirectionSet());
	DirChecker::checkLocationModeDirective(node, location.getLocationMode());
	DirChecker::checkRedirectionDirective(node);


	const std::vector<Node>& arg = node.getArguments();
	int statusCode = std::atoi(arg[0].getName().c_str());
	const std::string& address = arg[1].getName();
	location.setRedirectionStatusCode(statusCode);
	location.setAddress(address);
	location.setIsRedirection();
	location.setLocationMode(REDIRECTION);
}

void Location::handleClientMaxBodySizeDir(Location& location, const Node& node) {
	DirChecker::checkDuplicateDirective(node, location.getIsClientMaxBodySizeSet());
	DirChecker::checkClientMaxBodySizeDirective(node);

	const std::vector<Node>& arg = node.getArguments();
	char *end;
	long clientMaxBodySize = std::strtol(arg[0].getName().c_str(), &end, 10);
	location.setClientMaxBodySize(clientMaxBodySize);
	location.setIsClientMaxBodySize();
}

void Location::handleClientTimeoutDir(Location& location, const Node& node) {
	DirChecker::checkDuplicateDirective(node, location.getIsClientTimeoutSet());
	DirChecker::checkClientTimeoutDirective(node);

	const std::vector<Node>& arg = node.getArguments();
	char *end;
	long clientTimeout = std::strtol(arg[0].getName().c_str(), &end, 10);
	location.setClientTimeout(clientTimeout);
	location.setIsClientTimeout();
}

void Location::handleUnsupportedDir(const Node& node) {
	std::string msg = node.getName() + " at line: " + intToString(node.getLine())
		+ " col: " + intToString(node.getColumn()) + " not supported in location block";
	throw (Parser::ParseErrorException(msg));
}

void Location::handleUnknownDir(const Node& node) {
	std::string msg = "unexpected directive " + node.getName() + " at line: " +
		intToString(node.getLine()) + " col: " + intToString(node.getColumn())
		+ " in server block";
	throw (Parser::ParseErrorException(msg));
}

// ---------- GETTERS ----------
const std::string& Location::getUriPath() const {
	return (_uriPath);
}

const std::set<HttpMethod>& Location::getAllowedMethods() const {
	return (_allowedMethods);
}

PathMode Location::getPathMode() const {
	return (_pathMode);
}

const std::string& Location::getPath() const {
	return (_path);
}

LocationMode Location::getLocationMode() const {
	return (_locationMode);
}

bool Location::getIsAutoindexOn() const {
	return (_isAutoindexOn);
}

bool Location::getIsCgiOn() const {
	return (_isCgiOn);
}

const std::string& Location::getAddress() const {
	return (_address);
}

long Location::getClientMaxBodySize() const {
	return (_clientMaxBodySize);
}

long Location::getClientTimeout() const {
	return (_clientTimeout);
}

const std::vector<std::string>& Location::getIndexFiles() const {
	return (_indexFiles);
}

const std::map<int, std::string>& Location::getErrorPages() const {
	return (_errorPages);
}

int Location::getRedirectionStatusCode() const {
	return (_redirectionStatusCode);
}

bool Location::getIsAllowedMethodsSet() const {
	return (_isAllowedMethodsSet);
}

bool Location::getIsClientMaxBodySizeSet() const {
	return (_isClientMaxBodySizeSet);
}

bool Location::getIsClientTimeoutSet() const {
	return (_isClientTimeoutSet);
}

bool Location::getIsRootSet() const {
	return (_isRootSet);
}

bool Location::getIsAliasSet() const {
	return (_isAliasSet);
}

bool Location::getIsAutoindexSet() const {
	return (_isAutoindexSet);
}

bool Location::getIsProxyPassSet() const {
	return (_isProxyPassSet);
}

bool Location::getIsRedirectionSet() const {
	return (_isRedirectionSet);
}

bool Location::getIsCgiSet() const {
	return (_isCgiSet);
}

// ---------- SETTERS ----------
void Location::setUriPath(const std::string& uriPath) {
	_uriPath = uriPath;
}

void Location::setAllowedMethods(const std::set<HttpMethod>& newAllowedMethods) {
	_allowedMethods = newAllowedMethods;
}

void Location::setPathMode(PathMode mode) {
	_pathMode = mode;
}

void Location::setPath(const std::string& path) {
	_path = path;
}

void Location::setLocationMode(LocationMode mode) {
	_locationMode = mode;
}

void Location::setIsAutoindexOn() {
	if (!_isAutoindexOn)
		_isAutoindexOn = true;
}

void Location::setIsCgiOn() {
	if (!_isCgiOn)
		_isCgiOn = true;
}

void Location::setAddress(const std::string& address) {
	_address = address;
}

void Location::setClientMaxBodySize(long clientMaxBodySize) {
	_clientMaxBodySize = clientMaxBodySize;
}

void Location::setClientTimeout(long clientTimeout) {
	_clientTimeout = clientTimeout;
}

void Location::setIndexFiles(const std::vector<std::string>& newIndexFiles) {
	_indexFiles = newIndexFiles;
}

void Location::setErrorPages(const std::map<int, std::string>& newErrorPages) {
	_errorPages = newErrorPages;
}

void Location::setRedirectionStatusCode(int redirectionStatusCode) {
	_redirectionStatusCode = redirectionStatusCode;
}

void Location::setIsAllowedMethods() {
	if (!_isAllowedMethodsSet)
		_isAllowedMethodsSet = true;
}

void Location::setIsClientMaxBodySize() {
	if (!_isClientMaxBodySizeSet)
		_isClientMaxBodySizeSet = true;
}

void Location::setIsClientTimeout() {
	if (!_isClientTimeoutSet)
		_isClientTimeoutSet = true;
}

void Location::setIsRoot() {
	if (!_isRootSet)
		_isRootSet = true;
}

void Location::setIsAlias() {
	if (!_isAliasSet)
		_isAliasSet = true;
}

void Location::setIsAutoindex() {
	if (!_isAutoindexSet)
		_isAutoindexSet = true;
}

void Location::setIsProxyPass() {
	if (!_isProxyPassSet)
		_isProxyPassSet = true;
}

void Location::setIsRedirection() {
	if (!_isRedirectionSet)
		_isRedirectionSet = true;
}

void Location::setIsCgi() {
	if (!_isCgiSet)
		_isCgiSet = true;
}

// --------------- PUBLIC UTILS ---------------
const std::string* Location::getErrorPagePath(int statusCode) const {
	std::map<int, std::string>::const_iterator it = _errorPages.find(statusCode);
	if (it != _errorPages.end())
		return (&it->second);
	return (NULL);
}

void Location::addErrorPage(int code, const std::string& errorPagePath) {
	_errorPages[code] = errorPagePath;
}
