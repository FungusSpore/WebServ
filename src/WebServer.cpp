#include "../includes/WebServer.hpp"
#include "../includes/MiniHttpUtils.hpp"
#include <iostream>

// ========== Location Class Implementation ==========

Location::Location() 
    : _uriPath("/"), _hasCustomMethods(false), _locationMode(STATIC), 
      _path("/home/vee/42KL/Core/m5/tempweb/www/html"), _pathMode(ROOT), _hasRootPath(true),
      _clientMaxBodySize(1024*1024), _hasClientMaxBodySize(false),
      _hasErrorPages(false), _redirectionStatusCode(301) {
    
    // Set default allowed methods
    _allowedMethods.insert(GET);
    _allowedMethods.insert(HEAD);
    _allowedMethods.insert(POST);
}

Location::Location(const Location& other) 
    : _uriPath(other._uriPath), _allowedMethods(other._allowedMethods),
      _hasCustomMethods(other._hasCustomMethods), _locationMode(other._locationMode),
      _path(other._path), _address(other._address), _pathMode(other._pathMode),
      _hasRootPath(other._hasRootPath), _clientMaxBodySize(other._clientMaxBodySize),
      _hasClientMaxBodySize(other._hasClientMaxBodySize), _errorPages(other._errorPages),
      _hasErrorPages(other._hasErrorPages), _indexFiles(other._indexFiles),
      _proxyPassAddress(other._proxyPassAddress), _redirectionStatusCode(other._redirectionStatusCode),
      _redirectionAddress(other._redirectionAddress) {}

Location& Location::operator=(const Location& other) {
    if (this != &other) {
        _uriPath = other._uriPath;
        _allowedMethods = other._allowedMethods;
        _hasCustomMethods = other._hasCustomMethods;
        _locationMode = other._locationMode;
        _path = other._path;
        _address = other._address;
        _pathMode = other._pathMode;
        _hasRootPath = other._hasRootPath;
        _clientMaxBodySize = other._clientMaxBodySize;
        _hasClientMaxBodySize = other._hasClientMaxBodySize;
        _errorPages = other._errorPages;
        _hasErrorPages = other._hasErrorPages;
        _indexFiles = other._indexFiles;
        _proxyPassAddress = other._proxyPassAddress;
        _redirectionStatusCode = other._redirectionStatusCode;
        _redirectionAddress = other._redirectionAddress;
    }
    return *this;
}

Location::~Location() {}

// Setters for dummy implementation
void Location::setUriPath(const std::string& uriPath) { _uriPath = uriPath; }
void Location::setLocationMode(LocationMode mode) { _locationMode = mode; }
void Location::setPath(const std::string& path) { _path = path; }
void Location::setPathMode(PathMode mode) { _pathMode = mode; }
void Location::setAllowedMethods(const std::set<HttpMethod>& methods) { _allowedMethods = methods; }
void Location::setIndexFiles(const std::vector<std::string>& indexFiles) { _indexFiles = indexFiles; }
void Location::setAddress(const std::string& address) { _address = address; }
void Location::setRedirectionStatusCode(int statusCode) { _redirectionStatusCode = statusCode; }

// Getters
const std::string& Location::getUriPath() const { return _uriPath; }
const std::set<HttpMethod>& Location::getAllowedMethods() const { return _allowedMethods; }
LocationMode Location::getLocationMode() const { return _locationMode; }
const std::string& Location::getPath() const { return _path; }
PathMode Location::getPathMode() const { return _pathMode; }
long Location::getClientMaxBodySize() const { return _clientMaxBodySize; }
bool Location::hasClientMaxBodySize() const { return _hasClientMaxBodySize; }
const std::map<int, std::string>& Location::getErrorPages() const { return _errorPages; }
const std::vector<std::string>& Location::getIndexFiles() const { return _indexFiles; }
int Location::getRedirectionStatusCode() const { return _redirectionStatusCode; }
const std::string& Location::getAddress() const { return _address; }

// ========== Server Class Implementation ==========

Server::Server() 
    : _ip("0.0.0.0"), _port("80"), _serverName("localhost"),
      _rootPath("/home/vee/42KL/Core/m5/tempweb/www/html"), _clientMaxBodySize(1024*1024) {
    
    // Set default allowed methods
    _allowedMethods.insert(GET);
    _allowedMethods.insert(HEAD);
    _allowedMethods.insert(POST);
    
    // Add default index files
    _indexFiles.push_back("index.html");
    _indexFiles.push_back("index.htm");
    
    // Create default location for "/"
    Location defaultLocation;
    defaultLocation.setUriPath("/");
    defaultLocation.setLocationMode(STATIC);
    defaultLocation.setPath("/home/vee/42KL/Core/m5/tempweb/www/html");
    defaultLocation.setPathMode(ROOT);
    defaultLocation.setAllowedMethods(_allowedMethods);
    defaultLocation.setIndexFiles(_indexFiles);
    
    // Create autoindex location for "/directory/"
    Location autoindexLocation;
    autoindexLocation.setUriPath("/directory/");
    autoindexLocation.setLocationMode(AUTOINDEX);
    autoindexLocation.setPath("/home/vee/42KL/Core/m5/tempweb/www/html");
    autoindexLocation.setPathMode(ROOT);
    autoindexLocation.setAllowedMethods(_allowedMethods);
    
    // Create CGI location for "/cgi-bin/"
    Location cgiLocation;
    cgiLocation.setUriPath("/cgi-bin/");
    cgiLocation.setLocationMode(CGI);
    cgiLocation.setPath("/home/vee/42KL/Core/m5/tempweb/www");
    cgiLocation.setPathMode(ROOT);
    cgiLocation.setAllowedMethods(_allowedMethods);
    
    // Create images location for "/images/"
    Location imagesLocation;
    imagesLocation.setUriPath("/images/");
    imagesLocation.setLocationMode(AUTOINDEX);
    imagesLocation.setPath("/home/vee/42KL/Core/m5/tempweb/www/html");
    imagesLocation.setPathMode(ROOT);
    imagesLocation.setAllowedMethods(_allowedMethods);
    
    // Create redirection location for "/redirect"
    Location redirectLocation;
    redirectLocation.setUriPath("/redirect");
    redirectLocation.setLocationMode(REDIRECTION);
    redirectLocation.setAddress("https://www.example.com");
    redirectLocation.setRedirectionStatusCode(301);
    redirectLocation.setAllowedMethods(_allowedMethods);
    
    // Create redirection location for "/redirect"
    Location xredirectLocation;
    redirectLocation.setUriPath("/redirect/404");
    redirectLocation.setLocationMode(REDIRECTION);
    redirectLocation.setAddress("https://www.example.com");
    redirectLocation.setRedirectionStatusCode(404);
    redirectLocation.setAllowedMethods(_allowedMethods);

    _locations.push_back(cgiLocation);          // More specific paths first
    _locations.push_back(imagesLocation);
    _locations.push_back(autoindexLocation);
    _locations.push_back(redirectLocation);
    _locations.push_back(xredirectLocation);
    _locations.push_back(defaultLocation);      // Most general path last
}

Server::Server(const Server& other) 
    : _ip(other._ip), _port(other._port), _serverName(other._serverName),
      _rootPath(other._rootPath), _allowedMethods(other._allowedMethods),
      _clientMaxBodySize(other._clientMaxBodySize), _errorPages(other._errorPages),
      _locations(other._locations), _indexFiles(other._indexFiles) {}

Server& Server::operator=(const Server& other) {
    if (this != &other) {
        _ip = other._ip;
        _port = other._port;
        _serverName = other._serverName;
        _rootPath = other._rootPath;
        _allowedMethods = other._allowedMethods;
        _clientMaxBodySize = other._clientMaxBodySize;
        _errorPages = other._errorPages;
        _locations = other._locations;
        _indexFiles = other._indexFiles;
    }
    return *this;
}

Server::~Server() {}

const Location* Server::matchLocation(const std::string& target) const {
    // Find the most specific location that matches the target path
    const Location* bestMatch = NULL;
    size_t bestMatchLength = 0;
    
    for (std::vector<Location>::const_iterator it = _locations.begin(); 
         it != _locations.end(); ++it) {
        const std::string& uriPath = it->getUriPath();
        
        // Check if target starts with this location's URI path
        if (target.find(uriPath) == 0) {
            // For exact matches or when location ends with /
            if (target.length() == uriPath.length() || 
                uriPath[uriPath.length() - 1] == '/' ||
                target[uriPath.length()] == '/') {
                
                if (uriPath.length() > bestMatchLength) {
                    bestMatch = &(*it);
                    bestMatchLength = uriPath.length();
                }
            }
        }
    }
    
    return bestMatch;
}

const std::string* Server::getErrorPagePath(int statusCode) const {
    std::map<int, std::string>::const_iterator it = _errorPages.find(statusCode);
    if (it != _errorPages.end()) {
        return &(it->second);
    }
    return NULL;
}

// ========== WebServer Class Implementation ==========

WebServer::WebServer() {
    _ports.push_back("80");
    _ports.push_back("8080");
    
    // Create default server configurations
    ServerKey key1 = {"0.0.0.0", "80", "localhost"};
    ServerKey key2 = {"0.0.0.0", "8080", "localhost"};
    
    Server defaultServer;
    _serverMap[key1] = defaultServer;
    _serverMap[key2] = defaultServer;
}

WebServer::WebServer(const char* filename) {
    // For now, just create a default configuration
    // In the real implementation, this would parse the config file
    _ports.push_back("80");
    _ports.push_back("8080");
    
    ServerKey key = {"0.0.0.0", "80", "localhost"};
    Server defaultServer;
    _serverMap[key] = defaultServer;
    
    std::cout << "Dummy WebServer: Loaded configuration from " << filename << std::endl;
}

WebServer::WebServer(const WebServer& other) 
    : _ports(other._ports), _serverMap(other._serverMap) {}

WebServer& WebServer::operator=(const WebServer& other) {
    if (this != &other) {
        _ports = other._ports;
        _serverMap = other._serverMap;
    }
    return *this;
}

WebServer::~WebServer() {}

const Server* WebServer::matchServer(const ServerKey& key) const {
    // Try exact match first
    std::map<ServerKey, Server>::const_iterator it = _serverMap.find(key);
    if (it != _serverMap.end()) {
        return &(it->second);
    }
    
    // Try with default IP if not found
    ServerKey defaultKey = key;
    defaultKey.ip = "0.0.0.0";
    it = _serverMap.find(defaultKey);
    if (it != _serverMap.end()) {
        return &(it->second);
    }
    
    // Try with localhost as server name
    defaultKey.serverName = "localhost";
    it = _serverMap.find(defaultKey);
    if (it != _serverMap.end()) {
        return &(it->second);
    }
    
    // Return first available server as fallback
    if (!_serverMap.empty()) {
        return &(_serverMap.begin()->second);
    }
    
    return NULL;
}

const std::vector<std::string>& WebServer::getPorts() const {
    return _ports;
}

// ========== ServerKey Comparison Operator ==========

bool operator<(const ServerKey& lhs, const ServerKey& rhs) {
    if (lhs.serverName < rhs.serverName) return true;
    if (lhs.serverName > rhs.serverName) return false;
    return lhs.port < rhs.port;
}
